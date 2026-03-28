#pragma once

// HPX-Vision: External Causality Debugger & Deadlock Detector
// Top-level header for easy integration.

#include <hpx/vision/causality_id.hpp>
#include <hpx/vision/observer.hpp>
#include <hpx/vision/action_wrapper.hpp>
#include <hpx/vision/future_wrapper.hpp>

namespace hpx {
namespace vision {

/**
 * @brief Initialize HPX-Vision runtime components.
 * 
 * Call this in hpx_main() or as a pre-init hook.
 */
inline void init() {
    init_hooks();
}

} // namespace vision
} // namespace hpx
