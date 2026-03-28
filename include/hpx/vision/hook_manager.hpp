#pragma once
#include <hpx/vision/causality_id.hpp>
#include <hpx/vision/metadata_manager.hpp>
#include <hpx/threading_base/external_timer.hpp>
#include <hpx/runtime_local/get_locality_id.hpp>
#include <hpx/runtime_local/get_worker_thread_num.hpp>
#include <memory>
#include <string>

namespace hpx::vision::hooks {

/**
 * @brief Custom APEX task wrapper to carry CID metadata
 */
struct vision_task_wrapper : hpx::util::external_timer::task_wrapper {
    causality_id cid;
    causality_id parent_cid;
};

/**
 * @brief Hook called when a new task is created
 */
std::shared_ptr<hpx::util::external_timer::task_wrapper> on_new_task(
    std::string const& name, std::uint64_t const task_id,
    std::shared_ptr<hpx::util::external_timer::task_wrapper> const parent);

/**
 * @brief Hook called when a task starts executing
 */
void on_start(std::shared_ptr<hpx::util::external_timer::task_wrapper> wrapper);

/**
 * @brief Hook called when a task stops or yields
 */
void on_stop_yield(std::shared_ptr<hpx::util::external_timer::task_wrapper> wrapper);

/**
 * @brief Register HPX-Vision hooks with the HPX runtime
 */
void register_hooks();

} // namespace hpx::vision::hooks
