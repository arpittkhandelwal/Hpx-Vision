#pragma once
#include <hpx/config.hpp>
#include <hpx/vision/causality_id.hpp>
#include <hpx/vision/metrics.hpp>
#include <hpx/modules/synchronization.hpp>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>
#include <shared_mutex>
#include <mutex>
#include <stack>
#include <set>

namespace hpx::vision {

/**
 * @brief Wait-For-Graph (WFG) for tracking asynchronous task dependencies.
 * 
 * The WFG is a globally distributed graph where nodes represent HPX tasks (CIDs)
 * and edges represent dependencies (futures, mutexes, parcels). It uses Tarjan's
 * SCC algorithm to identify deadlocks with $O(V+E)$ complexity.
 */
class wfg {
public:
    /** @brief Get the singleton instance of the WFG. */
    static wfg& instance() {
        static wfg inst;
        return inst;
    }

    /**
     * @brief Records a dependency from one task to another with a lease.
     * @param from The task that is waiting.
     * @param to The task being waited on.
     */
    void add_edge(causality_id from, causality_id to) {
        auto now = std::chrono::steady_clock::now();
        std::unique_lock lock(mutex_);
        
        auto& neighbors = adj_[from];
        auto it = std::find_if(neighbors.begin(), neighbors.end(), 
            [&](const edge& e) { return e.to == to; });
            
        if (it != neighbors.end()) {
            it->last_updated = now;
        } else {
            neighbors.push_back({to, now});
        }
        
        auto& metrics = metrics_collector::instance();
        metrics.record_edge_op(std::chrono::steady_clock::now() - now);
        metrics.update_graph_size(adj_.size());
    }

    /**
     * @brief Prunes edges that have not been refreshed within the timeout.
     * 
     * This handles distributed consistency by eventually removing dependencies
     * from nodes that have crashed or failed to report task completion.
     */
    void prune_stale_edges(std::chrono::milliseconds timeout = std::chrono::milliseconds(5000)) {
        auto now = std::chrono::steady_clock::now();
        std::unique_lock lock(mutex_);
        
        for (auto it = adj_.begin(); it != adj_.end(); ) {
            auto& neighbors = it->second;
            neighbors.erase(
                std::remove_if(neighbors.begin(), neighbors.end(),
                    [&](const edge& e) { return (now - e.last_updated) > timeout; }),
                neighbors.end()
            );
            
            if (neighbors.empty()) {
                it = adj_.erase(it);
            } else {
                ++it;
            }
        }
    }

    /**
     * @brief Removes a task and all its associated edges from the graph.
     * @param id The CID of the task to remove.
     */
    void remove_node(causality_id id) {
        auto start = std::chrono::steady_clock::now();
        std::unique_lock lock(mutex_);
        adj_.erase(id);
        for (auto& [node, neighbors] : adj_) {
            neighbors.erase(
                std::remove_if(neighbors.begin(), neighbors.end(),
                    [&](const edge& e) { return e.to == id; }), 
                neighbors.end()
            );
        }
        
        auto& metrics = metrics_collector::instance();
        metrics.record_edge_op(std::chrono::steady_clock::now() - start);
        metrics.update_graph_size(adj_.size());
    }

    /**
     * @brief Detects if any cycles (deadlocks) exist in the graph.
     * @return true if a cycle is found, false otherwise.
     */
    bool has_cycle() const {
        return !get_deadlocked_sets().empty();
    }

    /**
     * @brief Finds all sets of tasks involved in deadlocks.
     * @return A vector of sets, where each set is a Strongly Connected Component (SCC) 
     *         with size > 1 (representing a cycle).
     */
    std::vector<std::set<causality_id>> get_deadlocked_sets() const {
        auto start = std::chrono::steady_clock::now();
        std::shared_lock lock(mutex_);
        
        if (adj_.empty()) return {};

        std::unordered_map<causality_id, int> disc, low;
        std::unordered_map<causality_id, bool> stack_member;
        std::stack<causality_id> st;
        int time = 0;
        std::vector<std::set<causality_id>> deadlocked_sets;

        for (auto const& [node, _] : adj_) {
            if (disc.find(node) == disc.end()) {
                tarjan_scc(node, disc, low, st, stack_member, time, deadlocked_sets);
            }
        }

        auto& metrics = metrics_collector::instance();
        metrics.record_cycle_check(std::chrono::steady_clock::now() - start);
        return deadlocked_sets;
    }

    /** @brief Exports the current graph state to Mermaid.js format. */
    std::string to_mermaid() const {
        std::shared_lock lock(mutex_);
        std::string out = "graph TD\n";
        for (auto const& [from, neighbors] : adj_) {
            for (auto const& e : neighbors) {
                out += "  " + std::to_string(from.data) + " --> " + std::to_string(e.to.data) + "\n";
            }
        }
        return out;
    }

    /**
     * @brief High-performance edge visitor for custom data exporting.
     * @param visitor Lambda that receives (from, to, timestamp).
     */
    template <typename F>
    void visit_edges(F&& visitor) const {
        std::shared_lock lock(mutex_);
        for (auto const& [from, neighbors] : adj_) {
            for (auto const& e : neighbors) {
                visitor(from, e.to, e.last_updated);
            }
        }
    }

private:
    struct edge {
        causality_id to;
        std::chrono::steady_clock::time_point last_updated;
    };

    wfg() = default;

    /** @brief Internal Tarjan's logic to find SCCs. */
    void tarjan_scc(causality_id u, 
                   std::unordered_map<causality_id, int>& disc, 
                   std::unordered_map<causality_id, int>& low,
                   std::stack<causality_id>& st,
                   std::unordered_map<causality_id, bool>& stack_member,
                   int& time,
                   std::vector<std::set<causality_id>>& deadlocked_sets) const {
        disc[u] = low[u] = ++time;
        st.push(u);
        stack_member[u] = true;

        auto it = adj_.find(u);
        if (it != adj_.end()) {
            for (auto const& e : it->second) {
                causality_id v = e.to;
                if (disc.find(v) == disc.end()) {
                    tarjan_scc(v, disc, low, st, stack_member, time, deadlocked_sets);
                    low[u] = (std::min)(low[u], low[v]);
                } else if (stack_member[v]) {
                    low[u] = (std::min)(low[u], disc[v]);
                }
            }
        }

        if (low[u] == disc[u]) {
            std::set<causality_id> current_scc;
            while (st.top() != u) {
                causality_id node = st.top();
                st.pop();
                stack_member[node] = false;
                current_scc.insert(node);
            }
            st.pop();
            stack_member[u] = false;
            current_scc.insert(u);

            // If SCC size > 1 OR there's a self-loop (u -> u), it's a deadlock
            if (current_scc.size() > 1) {
                deadlocked_sets.push_back(current_scc);
            } else {
                // Check for self-loop
                auto it_u = adj_.find(u);
                if (it_u != adj_.end()) {
                    for (auto const& e : it_u->second) {
                        if (e.to == u) {
                            deadlocked_sets.push_back(current_scc);
                            break;
                        }
                    }
                }
            }
        }
    }


    std::unordered_map<causality_id, std::vector<edge>> adj_;
    mutable std::shared_mutex mutex_;
};

} // namespace hpx::vision
