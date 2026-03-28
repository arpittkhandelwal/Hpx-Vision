#pragma once
#include <mutex>
#include <shared_mutex>
namespace hpx {
    using shared_mutex = std::shared_mutex;
    template <typename T> using lock_guard = std::lock_guard<T>;
}
