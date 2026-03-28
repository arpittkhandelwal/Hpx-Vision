#pragma once

#include <hpx/config.hpp>
#include <hpx/parcelset/parcelset_fwd.hpp>

namespace hpx::vision::hooks {

    /**
     * @brief Initialize parcel interception for distributed tracking.
     */
    void register_parcel_hooks();

    /**
     * @brief Internal handler to intercept outbound parcels.
     */
    void intercept_parcel(std::error_code const& ec, hpx::parcelset::parcel const& p);

} // namespace hpx::vision::hooks
