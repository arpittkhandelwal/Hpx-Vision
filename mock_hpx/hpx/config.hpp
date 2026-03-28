#pragma once
#include <cstdint>
#include <functional>

#define HPX_CXX_EXPORT
#define HPX_CORE_EXPORT
#define HPX_CXX_CORE_EXPORT

namespace hpx { 
    namespace threads { 
        struct thread_id_type { 
            void* data; 
            bool operator==(const thread_id_type& o) const { return data == o.data; }
            bool operator!=(const thread_id_type& o) const { return data != o.data; }
        }; 
    }
    namespace naming { 
        struct gid_type { 
            uint64_t msb; 
            uint64_t lsb; 
            bool operator==(const gid_type& o) const { return msb == o.msb && lsb == o.lsb; }
            bool operator!=(const gid_type& o) const { return msb != o.msb || lsb != o.lsb; }
        }; 
    }
}

namespace std {
    template <> 
    struct hash<hpx::threads::thread_id_type> { 
        size_t operator()(const hpx::threads::thread_id_type& t) const noexcept { 
            return reinterpret_cast<size_t>(t.data); 
        } 
    };
    
    template <> 
    struct hash<hpx::naming::gid_type> { 
        size_t operator()(const hpx::naming::gid_type& g) const noexcept { 
            return g.msb ^ g.lsb; 
        } 
    };
}
