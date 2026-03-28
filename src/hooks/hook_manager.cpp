#include <hpx/vision/hook_manager.hpp>
#include <hpx/vision/metadata_manager.hpp>
#include <hpx/vision/context.hpp>
#include <hpx/vision/causality_id.hpp>
#include <hpx/hpx.hpp>
#include <hpx/modules/threading_base.hpp>
#include <hpx/modules/runtime_local.hpp>
#include <iostream>

namespace hpx::vision::hooks {

    // HPX thread start hook: propagates CID from parent to child
    void on_thread_start(std::size_t pool_num, std::size_t thread_num, char const* pool_name, char const* thread_name) {
        try {
            auto parent_id = hpx::threads::get_parent_id();
            if (parent_id) {
                auto cid = metadata_manager::instance().get_cid(parent_id);
                if (cid.is_valid()) {
                    context::set(cid);
                }
            }
        } catch (...) {
            // Silently ignore to avoid crashing during bootstrap
        }
    }

    void register_hooks() {
        std::cout << "[HPX-Vision] Registering native HPX thread hooks..." << std::endl;
        hpx::register_thread_on_start_func(&on_thread_start);
        std::cout << "[HPX-Vision] SUCCESS: Thread hooks registered." << std::endl;
    }

} // namespace hpx::vision::hooks
