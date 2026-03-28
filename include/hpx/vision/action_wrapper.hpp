#pragma once

#include <hpx/hpx.hpp>
#include <hpx/vision/causality_id.hpp>
#include <hpx/vision/observer.hpp>

namespace hpx {
namespace vision {

/**
 * @brief Wrapped HPX action that carries a Causality ID
 */
template <typename Action>
struct async_action_wrapper {
    template <typename... Args>
    auto operator()(hpx::id_type const& id, Args&&... args) const {
        causality_id cid = context::get();
        return hpx::async<Action>(id, std::forward<Args>(args)..., cid);
    }
};

/**
 * @brief Helper to spawn a distributed action with causality tracking
 */
template <typename Action, typename... Args>
auto async(hpx::id_type const& id, Args&&... args) {
    causality_id cid = context::get();
    // Note: This assumes the Action has been modified to accept causality_id as last arg
    // for a truly non-intrusive version, we would need a shadow parcel system.
    return hpx::async<Action>(id, std::forward<Args>(args)..., cid);
}

} // namespace vision
} // namespace hpx
