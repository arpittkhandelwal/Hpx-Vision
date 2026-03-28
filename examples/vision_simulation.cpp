#include <hpx/hpx.hpp>
#include <hpx/vision/wfg.hpp>
#include <hpx/vision/metrics.hpp>
#include <hpx/vision/metadata_manager.hpp>
#include <hpx/vision/data_exporter.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

using namespace hpx::vision;

void simulate_activity() {
    auto& mm = metadata_manager::instance();
    auto& graph = wfg::instance();
    
    // Create some fake tasks
    causality_id c1(1001), c2(1002), c3(1003), c4(1004);
    
    mm.update_activity(c1);
    mm.update_activity(c2);
    mm.update_activity(c3);
    mm.update_activity(c4);

    std::cout << "[SIM] Starting Dependency Simulation..." << std::endl;

    // Add normal edges
    graph.add_edge(c1, c2);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Security: Set application namespace
    security_manager::instance().set_namespace("production-hpx-cluster-01");

    // Add a cycle (Deadlock Cluster A)
    std::cout << "[SIM] Injecting Deadlock A: C2 -> C3 -> C4 -> C2" << std::endl;
    graph.add_edge(c2, c3);
    graph.add_edge(c3, c4);
    graph.add_edge(c4, c2);

    // Add a second independent cycle (Deadlock Cluster B)
    causality_id c5(1005), c6(1006);
    mm.update_activity(c5);
    mm.update_activity(c6);
    std::cout << "[SIM] Injecting Deadlock B: C5 -> C6 -> C5" << std::endl;
    graph.add_edge(c5, c6);
    graph.add_edge(c6, c5);

    // Continuous export loop
    for (int i = 0; i < 60; ++i) {
        // Heartbeat activity for all tasks to prevent premature pruning
        mm.update_activity(c1);
        mm.update_activity(c2);
        mm.update_activity(c3);
        mm.update_activity(c4);
        mm.update_activity(c5);
        mm.update_activity(c6);

        // Fault Tolerance: Periodic pruning
        graph.prune_stale_edges(std::chrono::seconds(10));
        mm.prune_inactive_tasks(std::chrono::seconds(30));
        
        data_exporter::export_to_json("dashboard/public/vision_data.json");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    simulate_activity();
    return 0;
}
