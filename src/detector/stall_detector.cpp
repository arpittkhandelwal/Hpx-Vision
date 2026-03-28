#include <hpx/hpx.hpp>
#include <hpx/modules/threading_base.hpp>
#include <hpx/vision/observer.hpp>
#include <hpx/vision/causality_id.hpp>
#include <hpx/vision/metadata_manager.hpp>
#include <hpx/vision/wfg.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>

namespace hpx {
namespace vision {

/**
 * @brief Stall Detector that monitors threads for long-duration suspension
 */
class stall_detector {
public:
    stall_detector(std::chrono::milliseconds timeout) 
        : timeout_(timeout), running_(false) {}

    void start() {
        if (running_) return;
        running_ = true;
        
        hpx::threads::register_thread([this]() {
            while (running_) {
                hpx::this_thread::sleep_for(std::chrono::milliseconds(500));
                auto stalls = check_for_stalls();
                if (!stalls.empty()) {
                    report_stalls(stalls);
                }
            }
        }, "vision_stall_detector");
    }

    void stop() {
        running_ = false;
    }

private:
    struct stall_info {
        causality_id id;
        std::chrono::milliseconds duration;
    };

    std::vector<stall_info> check_for_stalls() {
        std::vector<stall_info> stalled_tasks;
        auto now = std::chrono::steady_clock::now();
        auto& mm = metadata_manager::instance();
        
        for (auto const& cid : mm.get_active_tasks()) {
            auto last_active = mm.get_last_active(cid);
            if (last_active == std::chrono::steady_clock::now()) continue; // Not set or error

            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_active);
            if (diff > timeout_) {
                stalled_tasks.push_back({cid, diff});
            }
        }
        return stalled_tasks;
    }

    void report_stalls(const std::vector<stall_info>& stalls) {
        std::cout << "\033[1;31m[HPX-VISION] ALERT: " << stalls.size() << " task stalls detected!\033[0m" << std::endl;
        for (const auto& stall : stalls) {
            std::cout << "  - Stalled Task: " << std::to_string(stall.id.data) 
                      << " (Idle for " << stall.duration.count() << "ms)" << std::endl;
        }
    }

    std::chrono::milliseconds timeout_;
    std::atomic<bool> running_;
};

} // namespace vision
} // namespace hpx
