#pragma once

#include <hpx/vision/causality_id.hpp>
#include <hpx/modules/threading_base.hpp>
#include <unordered_map>
#include <hpx/modules/synchronization.hpp>
#include <mutex>
#include <shared_mutex>

namespace hpx {
namespace vision {

/**
 * @brief Manages the sidecar metadata (hpx::thread_id -> causality_id)
 * Optimized with a more granular locking strategy for Phase 2.
 */
class metadata_manager {
public:
    static metadata_manager& instance() {
        static metadata_manager inst;
        return inst;
    }

    void register_thread(hpx::threads::thread_id_type id, causality_id cid) {
        std::unique_lock<hpx::shared_mutex> lock(mutex_);
        thread_to_cid_[id] = cid;
    }

    causality_id get_cid(hpx::threads::thread_id_type id) {
        if (id == hpx::threads::invalid_thread_id) return causality_id();
        
        std::shared_lock<hpx::shared_mutex> lock(mutex_);
        auto it = thread_to_cid_.find(id);
        if (it != thread_to_cid_.end()) {
            return it->second;
        }
        return causality_id();
    }

    void unregister_thread(hpx::threads::thread_id_type id) {
        std::unique_lock<hpx::shared_mutex> lock(mutex_);
        thread_to_cid_.erase(id);
    }

private:
    std::unordered_map<hpx::threads::thread_id_type, causality_id> thread_to_cid_;
    hpx::shared_mutex mutex_;
};

/**
 * @brief Thread-local context for the current running task
 */
struct context {
    static thread_local causality_id current_cid;

    static void set(causality_id cid) { current_cid = cid; }
    static causality_id get() { return current_cid; }
};

} // namespace vision
} // namespace hpx
