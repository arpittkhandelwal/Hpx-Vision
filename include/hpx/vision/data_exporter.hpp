#pragma once
#include <hpx/vision/wfg.hpp>
#include <hpx/vision/metrics.hpp>
#include <hpx/vision/metadata_manager.hpp>
#include <hpx/vision/security.hpp>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <chrono>
#include <sstream>

namespace hpx::vision {

/**
 * @brief Exports WFG and Metrics to JSON for the web dashboard.
 * 
 * This service bridges the C++ core with the React frontend by writing 
 * structured state directly to a shared directory.
 */
class data_exporter {
public:
    static void export_to_json(std::string const& filepath) {
        std::ofstream out(filepath);
        if (!out.is_open()) return;

        auto& graph = wfg::instance();
        auto& metrics = metrics_collector::instance();
        auto& mm = metadata_manager::instance();
        
        auto deadlocked_sets = graph.get_deadlocked_sets();
        auto snapshot = metrics.get_snapshot();
        
        auto& sm = security_manager::instance();
        
        std::stringstream ss;
        ss << "{\n";
        ss << "  \"namespace\": \"" << sm.get_namespace() << "\",\n";
        ss << "  \"samplingRatio\": " << metrics.get_sampling_ratio() << ",\n";
        
        // 1. Metrics
        ss << "  \"metrics\": {\n";
        ss << "    \"edgeOps\": " << snapshot.edge_ops_count << ",\n";
        ss << "    \"avgLatency\": " << snapshot.avg_edge_op_latency_ns << ",\n";
        ss << "    \"cycleChecks\": " << snapshot.cycle_checks_count << ",\n";
        ss << "    \"maxNodes\": " << snapshot.max_graph_nodes << ",\n";
        ss << "    \"currentNodes\": " << snapshot.current_graph_nodes << "\n";
        ss << "  },\n";

        // 2. Nodes
        ss << "  \"nodes\": [\n";
        auto tasks = mm.get_active_tasks();
        for (size_t i = 0; i < tasks.size(); ++i) {
            bool is_deadlocked = false;
            for (auto const& s : deadlocked_sets) {
                if (s.count(tasks[i])) { is_deadlocked = true; break; }
            }
            
            ss << "    {\"id\": \"" << sm.anonymize_cid(tasks[i]) << "\", "
               << "\"stalled\": " << (is_deadlocked ? "true" : "false") << "}";
            if (i < tasks.size() - 1) ss << ",";
            ss << "\n";
        }
        ss << "  ],\n";

        // 3. Links
        ss << "  \"links\": [\n";
        bool first_link = true;
        graph.visit_edges([&](causality_id from, causality_id to, auto /*ts*/) {
            if (!first_link) ss << ",\n";
            ss << "    {\"source\": \"" << sm.anonymize_cid(from) << "\", "
               << "\"target\": \"" << sm.anonymize_cid(to) << "\"}";
            first_link = false;
        });
        ss << "\n  ],\n";

        // 4. Clusters
        ss << "  \"clusters\": [\n";
        for (size_t i = 0; i < deadlocked_sets.size(); ++i) {
            ss << "    [";
            size_t j = 0;
            for (auto const& cid : deadlocked_sets[i]) {
                ss << "\"" << sm.anonymize_cid(cid) << "\"";
                if (++j < deadlocked_sets[i].size()) ss << ", ";
            }
            ss << "]";
            if (i < deadlocked_sets.size() - 1) ss << ",";
            ss << "\n";
        }
        ss << "  ]\n";
        
        ss << "}";
        out << ss.str();
    }
};

} // namespace hpx::vision
