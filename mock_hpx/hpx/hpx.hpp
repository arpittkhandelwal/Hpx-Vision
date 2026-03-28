#pragma once
#include <hpx/config.hpp>
#include <hpx/modules/threading_base.hpp>
#include <hpx/modules/naming_base.hpp>
#include <hpx/modules/synchronization.hpp>
#include <thread>
#include <chrono>

namespace hpx {
    namespace this_thread {
        template <typename Rep, typename Period>
        void sleep_for(const std::chrono::duration<Rep, Period>& sleep_duration) {
            std::this_thread::sleep_for(sleep_duration);
        }
    }
}
