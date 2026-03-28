#pragma once
#include <hpx/vision/causality_id.hpp>
#include <hpx/modules/threading_base.hpp>
#include <hpx/modules/naming_base.hpp>
#include <unordered_map>
#include <shared_mutex>
#include <atomic>
#include <mutex>
#include <vector>
#include <chrono>

namespace hpx::vision {

/**
 * @brief Thread-safe registry for task-to-CID mappings.
 * 
 * The Metadata Manager is responsible for maintaining the global causal 
 * context across threads and parcels. It provides $O(1)$ lookups for CID 
 * assignment and activity monitoring.
 */
class metadata_manager {
public:
    /** @brief Access the singleton instance. */
    static metadata_manager& instance() {
        static metadata_manager inst;
        return inst;
    }

    /**
     * @brief Maps an HPX thread ID to a Causality ID.
     * @param id The native HPX thread ID.
     * @param cid The distributed Causality ID.
     */
    void register_task(hpx::threads::thread_id_type id, causality_id cid) {
        std::unique_lock lock(mutex_);
        task_map_[id] = cid;
    }

    void unregister_task(hpx::threads::thread_id_type id) {
        std::unique_lock lock(mutex_);
        task_map_.erase(id);
    }

    causality_id get_cid(hpx::threads::thread_id_type id) const {
        std::shared_lock lock(mutex_);
        auto it = task_map_.find(id);
        return it != task_map_.end() ? it->second : causality_id();
    }

    void register_parcel_cid(hpx::naming::gid_type const& parcel_id, causality_id cid) {
        std::unique_lock lock(mutex_);
        parcel_map_[parcel_id] = cid;
    }

    void unregister_parcel_cid(hpx::naming::gid_type const& parcel_id) {
        std::unique_lock lock(mutex_);
        parcel_map_.erase(parcel_id);
    }

    causality_id get_parcel_cid(hpx::naming::gid_type const& parcel_id) const {
        std::shared_lock lock(mutex_);
        auto it = parcel_map_.find(parcel_id);
        return it != parcel_map_.end() ? it->second : causality_id();
    }

    causality_id next_cid(std::uint16_t loc, std::uint16_t thread) {
        auto seq = ++seq_counter_;
        if (seq == 0) seq = ++seq_counter_; // Roll over safety
        return generate_cid(loc, thread, seq);
    }

    void update_activity(causality_id cid) {
        std::unique_lock lock(mutex_);
        last_active_[cid] = std::chrono::steady_clock::now();
    }

    std::chrono::steady_clock::time_point get_last_active(causality_id cid) const {
        std::shared_lock lock(mutex_);
        auto it = last_active_.find(cid);
        return it != last_active_.end() ? it->second : std::chrono::steady_clock::time_point();
    }

    std::vector<causality_id> get_active_tasks() const {
        std::shared_lock lock(mutex_);
        std::vector<causality_id> tasks;
        for (auto const& [cid, _] : last_active_) {
            tasks.push_back(cid);
        }
        return tasks;
    }

    /**
     * @brief Prunes task metadata for threads that haven't shown activity.
     * @param timeout Duration after which a task is considered stale.
     */
    void prune_inactive_tasks(std::chrono::milliseconds timeout = std::chrono::milliseconds(30000)) {
        auto now = std::chrono::steady_clock::now();
        std::unique_lock lock(mutex_);
        
        for (auto it = last_active_.begin(); it != last_active_.end(); ) {
            if ((now - it->second) > timeout) {
                causality_id cid = it->first;
                // Cleanup task_map_
                for (auto tit = task_map_.begin(); tit != task_map_.end(); ) {
                    if (tit->second == cid) tit = task_map_.erase(tit);
                    else ++tit;
                }
                // Cleanup parcel_map_
                for (auto pit = parcel_map_.begin(); pit != parcel_map_.end(); ) {
                    if (pit->second == cid) pit = parcel_map_.erase(pit);
                    else ++pit;
                }
                it = last_active_.erase(it);
            } else {
                ++it;
            }
        }
    }

private:
    metadata_manager() : seq_counter_(0) {}
    
    std::unordered_map<hpx::threads::thread_id_type, causality_id> task_map_;
    std::unordered_map<hpx::naming::gid_type, causality_id> parcel_map_;
    std::unordered_map<causality_id, std::chrono::steady_clock::time_point> last_active_;
    mutable std::shared_mutex mutex_;
    std::atomic<std::uint64_t> seq_counter_;
};

} // namespace hpx::vision
