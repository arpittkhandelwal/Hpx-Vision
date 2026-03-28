#include <hpx/vision/hook_manager.hpp>
#include <hpx/vision/context.hpp>
#include <hpx/vision/wfg.hpp>
#include <hpx/hpx.hpp>
#include <hpx/init.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/modules/program_options.hpp>
#include <iostream>

// Tactical stub for HPX assertion handler on macOS
namespace hpx::assertion::detail {
    void handle_assert(std::source_location const&, char const*, std::string const&) noexcept {}
}

void async_task(int depth) {
    auto my_cid = hpx::vision::context::get();
    std::cout << "[Demo] Task at depth " << depth << " has CID: " << my_cid.data << std::endl;
    
    if (depth > 0) {
        hpx::async(&async_task, depth - 1).get();
    }
}

void deadlock_sim() {
    auto cid_a = hpx::vision::causality_id(0xAAAA0001);
    auto cid_b = hpx::vision::causality_id(0xBBBB0002);

    std::cout << "[Demo] Simulating Wait-For-Graph cycle: A -> B, B -> A" << std::endl;
    hpx::vision::wfg::instance().add_edge(cid_a, cid_b);
    hpx::vision::wfg::instance().add_edge(cid_b, cid_a);

    if (hpx::vision::wfg::instance().has_cycle()) {
        std::cout << "\n[HPX-Vision] ALERT: DEADLOCK DETECTED!" << std::endl;
        std::cout << hpx::vision::wfg::instance().to_mermaid() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // Phase 4 FIX: Defer hook registration until HPX main loop
    std::cout << "HPX-Vision Demo: Native Runtime Integration" << std::endl;
    
    return hpx::local::init([](hpx::program_options::variables_map&) {
        std::cout << "[Demo] HPX Runtime active. Registering hooks..." << std::endl;
        
        // Registering hooks safely INSIDE the HPX runtime
        hpx::vision::hooks::register_hooks();
        
        hpx::async([]() {
            // Set root CID
            hpx::vision::context::set(hpx::vision::causality_id(0xABCDEF12340001));
            
            async_task(2);
            deadlock_sim();
        }).get();
        
        std::cout << "[Demo] Tasks completed. Shutdown..." << std::endl;
        return hpx::local::finalize();
    }, argc, argv);
}
