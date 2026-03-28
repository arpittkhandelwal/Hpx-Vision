#pragma once

#include <hpx/config.hpp>
#include <hpx/hpx.hpp>
#include <hpx/vision/causality_id.hpp>
#include <hpx/modules/actions_base.hpp>
#include <hpx/actions_base/plain_action.hpp>
#include <hpx/modules/naming.hpp>

namespace hpx {
    namespace vision {
        namespace messaging {
            /**
             * @brief Side-channel action to register a CID for a received parcel.
             */
            HPX_CXX_EXPORT void register_remote_cid(hpx::naming::gid_type const& parcel_id, hpx::vision::causality_id cid);
        }
    }
}

// Definitive HPX plain action declaration
HPX_DECLARE_PLAIN_ACTION(hpx::vision::messaging::register_remote_cid, register_remote_cid_action);
