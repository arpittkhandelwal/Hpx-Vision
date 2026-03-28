#include <hpx/vision/causality_id.hpp>
#include <hpx/vision/context.hpp>

namespace hpx::vision {

static thread_local causality_id current_cid_tls;

void context::set(causality_id cid) noexcept {
    // std::cout << "[Context] Setting CID: " << cid.data << std::endl;
    current_cid_tls = cid;
}

causality_id context::get() noexcept {
    return current_cid_tls;
}

void context::reset() noexcept {
    current_cid_tls = causality_id();
}

} // namespace hpx::vision
