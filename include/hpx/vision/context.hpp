#pragma once
#include <hpx/vision/causality_id.hpp>

namespace hpx::vision {
    struct context {
        static void set(causality_id cid) noexcept;
        static causality_id get() noexcept;
        static void reset() noexcept;
    };
}
