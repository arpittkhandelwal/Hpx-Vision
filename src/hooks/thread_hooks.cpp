#include <hpx/vision/observer.hpp>
#include <hpx/modules/threading_base.hpp>
#include <hpx/modules/runtime_local.hpp>
#include <hpx/hpx.hpp>
#include <hpx/vision/otel_tracer.hpp>
#include <hpx/vision/metadata_manager.hpp>

namespace hpx {
namespace vision {

// Implementation of thread-local context
thread_local causality_id context::current_cid = causality_id();

void thread_hook(hpx::threads::thread_id_type id) {
    auto state = hpx::threads::get_thread_state(id);
    
    // When a thread is created or resumed
    if (state.state() == hpx::threads::thread_schedule_state::pending) {
        // Adaptive Sampling: Only track a subset of tasks to reduce overhead
        auto& metrics = metrics_collector::instance();
        if (!metrics.should_sample()) {
            return; // Skip tracking for this task
        }

        auto* data = hpx::threads::get_thread_id_data(id);
        auto parent_id = data->get_parent_thread_id();
        auto parent_cid = metadata_manager::instance().get_cid(parent_id);
        
        std::uint16_t loc = static_cast<std::uint16_t>(hpx::get_locality_id());
        std::uint16_t thread = static_cast<std::uint16_t>(hpx::get_worker_thread_num());
        
        auto my_cid = generate_cid(loc, thread);
        metadata_manager::instance().register_thread(id, my_cid);
        metadata_manager::instance().update_activity(my_cid);
        
        VISION_SPAN("thread_create", my_cid);

        // Update context
        context::set(my_cid);
    }
    // When a thread terminates
    else if (state.state() == hpx::threads::thread_schedule_state::deleted) {
        metadata_manager::instance().unregister_thread(id);
        context::set(causality_id());
    }
}

// Function to register the hook with HPX
void init_hooks() {
    hpx::threads::add_thread_hook(&thread_hook);
}

} // namespace vision
} // namespace hpx
