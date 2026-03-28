#pragma once
#include <atomic>
#include <chrono>
#include <string>
#include <vector>
#include <mutex>
#include <random>
#include <algorithm>

namespace hpx::vision {

/**
 * @brief Thread-safe performance telemetry collector.
 * 
 * Metrics Collector provides low-latency monitoring of graph operations, 
 * cycle detection overhead, and memory pressure. It is designed to expose 
 * data to both HPX performance counters and external dashboards.
 */
class metrics_collector {
public:
    /** @brief Access the singleton instance. */
    static metrics_collector& instance() {
        static metrics_collector inst;
        return inst;
    }

    struct snapshot {
        std::uint64_t edge_ops_count;
        double avg_edge_op_latency_ns;
        std::uint64_t cycle_checks_count;
        double avg_cycle_check_latency_ns;
        std::size_t max_graph_nodes;
        std::size_t current_graph_nodes;
    };

    void record_edge_op(std::chrono::nanoseconds duration) {
        edge_ops_count_++;
        total_edge_op_time_ns_ += duration.count();
    }

    void record_cycle_check(std::chrono::nanoseconds duration) {
        cycle_checks_count_++;
        total_cycle_check_time_ns_ += duration.count();
    }

    void update_graph_size(std::size_t nodes) {
        current_graph_nodes_ = nodes;
        if (nodes > max_graph_nodes_) {
            max_graph_nodes_ = nodes;
        }
        
        // Dynamic Throttling: Adjust sampling based on overhead
        auto snapshot = get_snapshot();
        adjust_sampling(snapshot.avg_edge_op_latency_ns);
    }

    bool should_sample() const {
        // Simple probabilistic check
        static thread_local std::uint32_t seed = std::random_device{}();
        static thread_local std::mt19937 gen(seed);
        std::uniform_real_distribution<> dis(0.0, 1.0);
        return dis(gen) < sampling_ratio_.load();
    }

    void adjust_sampling(double current_latency_ns) {
        float current_ratio = sampling_ratio_.load();
        if (current_latency_ns > 500.0 && current_ratio > 0.01f) {
            sampling_ratio_.store(current_ratio * 0.5f); // High overhead, throttle down
        } else if (current_latency_ns < 50.0 && current_ratio < 1.0f) {
            sampling_ratio_.store((std::min)(1.0f, current_ratio * 1.1f)); // Low overhead, throttle up
        }
    }

    float get_sampling_ratio() const { return sampling_ratio_.load(); }

    snapshot get_snapshot() const {
        snapshot s;
        s.edge_ops_count = edge_ops_count_.load();
        s.avg_edge_op_latency_ns = s.edge_ops_count > 0 ? 
            static_cast<double>(total_edge_op_time_ns_.load()) / s.edge_ops_count : 0.0;
        
        s.cycle_checks_count = cycle_checks_count_.load();
        s.avg_cycle_check_latency_ns = s.cycle_checks_count > 0 ? 
            static_cast<double>(total_cycle_check_time_ns_.load()) / s.cycle_checks_count : 0.0;
        
        s.max_graph_nodes = max_graph_nodes_.load();
        s.current_graph_nodes = current_graph_nodes_.load();
        return s;
    }

private:
    metrics_collector() = default;

    std::atomic<std::uint64_t> edge_ops_count_{0};
    std::atomic<std::uint64_t> total_edge_op_time_ns_{0};
    std::atomic<std::uint64_t> cycle_checks_count_{0};
    std::atomic<std::uint64_t> total_cycle_check_time_ns_{0};
    std::atomic<std::size_t> max_graph_nodes_{0};
    std::atomic<std::size_t> current_graph_nodes_{0};
    std::atomic<float> sampling_ratio_{0.1f};
};

} // namespace hpx::vision
