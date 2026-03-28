#pragma once
#include <hpx/vision/causality_id.hpp>
#include <string>
#include <sstream>
#include <iomanip>
#include <random>
#include <functional>

namespace hpx::vision {

/**
 * @brief Provides security and isolation features for HPX-Vision.
 */
class security_manager {
public:
    static security_manager& instance() {
        static security_manager inst;
        return inst;
    }

    /**
     * @brief Anonymizes a CID by hashing its components with a session salt.
     * This prevents internal infrastructure details (locality/thread IDs) from 
     * leaking to the dashboard.
     */
    std::string anonymize_cid(causality_id cid) {
        // Robust mixing function (SplitMix64-style) to ensure unique bit distribution
        std::uint64_t x = cid.data ^ salt_;
        x += 0x9e3779b97f4a7c15;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
        x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
        std::uint64_t h = x ^ (x >> 31);
        
        std::stringstream ss;
        ss << std::hex << std::setw(12) << std::setfill('0') << (h & 0xFFFFFFFFFFFFULL);
        return ss.str();
    }

    /**
     * @brief Sets the application-level namespace/tenant ID.
     */
    void set_namespace(std::string const& ns) {
        namespace_ = ns;
    }

    std::string const& get_namespace() const {
        return namespace_;
    }

private:
    security_manager() {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<std::uint64_t> dis;
        salt_ = dis(gen);
    }

    std::uint64_t salt_;
    std::string namespace_{"default"};
};

} // namespace hpx::vision
