#include <hpx/vision/dist_sync.hpp>
#include <hpx/vision/metadata_manager.hpp>
#include <hpx/vision/context.hpp>
#include <hpx/hpx.hpp>
#include <iostream>

namespace hpx::vision::messaging {

    void register_remote_cid(causality_id remote_cid, hpx::naming::gid_type hpx_thread_id) {
        std::cout << "[HPX-Vision] Received Remote CID: " << remote_cid.data 
                  << " for thread: " << std::hex << hpx_thread_id.msb << std::endl;
        metadata_manager::instance().register_task(hpx_thread_id, remote_cid);
    }

} // namespace hpx::vision::messaging

// HPX Plain Action Registration - Disabled temporarily to resolve macOS modular build errors
// HPX_PLAIN_ACTION(hpx::vision::messaging::register_remote_cid, register_remote_cid_action);
