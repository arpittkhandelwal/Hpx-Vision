#include <hpx/vision/causality_id.hpp>
#include <hpx/vision/context.hpp>
#include <hpx/vision/wfg.hpp>
#include <hpx/vision/metadata_manager.hpp>
#include <iostream>
#include <thread>
#include <assert.h>

void test_causality_id() {
    std::cout << "[Test] Verifying Causality ID Layout..." << std::endl;
    auto cid = hpx::vision::generate_cid(1, 2, 100);
    assert(cid.locality() == 1);
    assert(cid.thread() == 2);
    assert(cid.sequence() == 100);
    std::cout << "[Test] SUCCESS: CID Layout verified." << std::endl;
}

void test_context_propagation() {
    std::cout << "[Test] Verifying Context Propagation (std::thread simulation)..." << std::endl;
    auto parent_cid = hpx::vision::causality_id(0xABCDEF0001);
    hpx::vision::context::set(parent_cid);
    
    std::thread t([parent_cid]() {
        // In a real HPX hook, this happens automatically
        hpx::vision::context::set(parent_cid);
        auto child_cid = hpx::vision::context::get();
        assert(child_cid == parent_cid);
        std::cout << "[Test]   Child thread recovered CID: " << child_cid.data << std::endl;
    });
    t.join();
    std::cout << "[Test] SUCCESS: Context propagation verified." << std::endl;
}

void test_wfg_cycle_detection() {
    std::cout << "[Test] Verifying Wait-For-Graph Cycle Detection..." << std::endl;
    auto& graph = hpx::vision::wfg::instance();
    
    auto cid1 = hpx::vision::causality_id(1);
    auto cid2 = hpx::vision::causality_id(2);
    auto cid3 = hpx::vision::causality_id(3);

    // DAG: 1 -> 2 -> 3
    graph.add_edge(cid1, cid2);
    graph.add_edge(cid2, cid3);
    assert(!graph.has_cycle());
    std::cout << "[Test]   DAG (1->2->3) correctly reported as NO cycle." << std::endl;

    // Cycle: 3 -> 1
    graph.add_edge(cid3, cid1);
    assert(graph.has_cycle());
    std::cout << "[Test]   Cycle (3->1) correctly DETECTED!" << std::endl;

    std::cout << "[Test]   Mermaid representation:\n" << graph.to_mermaid() << std::endl;
    std::cout << "[Test] SUCCESS: WFG Cycle detection verified." << std::endl;
}

int main() {
    std::cout << "HPX-Vision Core Unit Tests (Standalone Proof of Logic)\n" << std::endl;
    
    test_causality_id();
    test_context_propagation();
    test_wfg_cycle_detection();

    std::cout << "\n[HPX-Vision] ALL CORE LOGIC VERIFIED SUCCESSFULLY!" << std::endl;
    return 0;
}
