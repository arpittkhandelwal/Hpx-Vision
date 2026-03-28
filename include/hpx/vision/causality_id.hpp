#pragma once

#pragma once
#include <cstdint>
#include <string>
#include <sstream>

namespace hpx::vision {

/**
 * @brief Global 64-bit Causality Identifier (CID)
 * Layout: [48-63] : Locality (16) | [36-47] : Thread (12) | [00-35] : Sequence (36)
 */
struct causality_id {
    std::uint64_t data;

    causality_id() noexcept : data(0) {}
    explicit causality_id(std::uint64_t val) noexcept : data(val) {}

    bool is_valid() const noexcept { return data != 0; }
    
    std::uint16_t locality() const noexcept { return static_cast<std::uint16_t>(data >> 48); }
    std::uint16_t thread() const noexcept { return static_cast<std::uint16_t>((data >> 36) & 0xFFF); }
    std::uint64_t sequence() const noexcept { return data & 0xFFFFFFFFF; }

    bool operator==(causality_id const& other) const noexcept { return data == other.data; }
    bool operator!=(causality_id const& other) const noexcept { return data != other.data; }
    bool operator<(causality_id const& other) const noexcept { return data < other.data; }

    template <typename Archive>
    void serialize(Archive& ar, unsigned int const /* version */)
    {
        ar & data;
    }
};

/**
 * @brief Helper to generate a new CID
 */
inline causality_id generate_cid(std::uint16_t loc, std::uint16_t thread, std::uint64_t seq) noexcept {
    return causality_id(
        (static_cast<std::uint64_t>(loc) << 48) |
        (static_cast<std::uint64_t>(thread & 0xFFF) << 36) |
        (seq & 0xFFFFFFFFF)
    );
}

} // namespace hpx::vision

// Custom hash for causality_id
namespace std {
template <>
struct hash<hpx::vision::causality_id> {
    std::size_t operator()(hpx::vision::causality_id const& cid) const noexcept {
        return std::hash<std::uint64_t>{}(cid.data);
    }
};
} // namespace std
