#pragma once

#include <hpx/hpx.hpp>
#include <hpx/vision/causality_id.hpp>
#include <hpx/vision/observer.hpp>

namespace hpx {
namespace vision {

/**
 * @brief Wrapped HPX future that tracks the provider's causality
 */
template <typename T>
class future {
public:
    future(hpx::future<T>&& f) : f_(std::move(f)) {
        // Record the causality of the task that created this future
        provider_cid_ = context::get();
    }

    auto get() {
        return f_.get();
    }

    causality_id provider_cid() const { return provider_cid_; }

private:
    hpx::future<T> f_;
    causality_id provider_cid_;
};

/**
 * @brief Helper to wrap a standard HPX future into a vision-tracked future
 */
template <typename T>
future<T> track(hpx::future<T>&& f) {
    return future<T>(std::move(f));
}

} // namespace vision
} // namespace hpx
