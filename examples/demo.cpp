#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/iostream.hpp>
#include <hpx/modules/threading_base.hpp>

// Include Vision headers (external)
#include <hpx/vision/observer.hpp>
#include <hpx/vision/causality_id.hpp>

void child_task(int id) {
    auto my_cid = hpx::vision::context::get();
    hpx::cout << "Child Task " << id << " running with CID: " << my_cid.to_string() << std::endl;
}

int hpx_main() {
    // 1. Initialize HPX-Vision hooks (usually done via a plugin, here manual for demo)
    hpx::vision::init_hooks();

    auto root_cid = hpx::vision::context::get();
    hpx::cout << "Root Task running with CID: " << root_cid.to_string() << std::endl;

    // 2. Spawn some child tasks
    std::vector<hpx::future<void>> futures;
    for (int i = 0; i < 3; ++i) {
        futures.push_back(hpx::async(&child_task, i));
    }

    hpx::wait_all(futures);

    return hpx::finalize();
}

int main(int argc, char** argv) {
    return hpx::init(argc, argv);
}
