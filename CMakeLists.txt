cmake_minimum_required(VERSION 3.22)
project(HPX-Vision LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find HPX
find_package(HPX CONFIG REQUIRED)

add_definitions(-DHPX_HAVE_NETWORKING)

# Local project headers
include_directories(BEFORE include)

# Dependencies: HPX should be found via find_package
# On macOS, users can set -DHPX_DIR=/path/to/hpx if not in standard path


# Consolidated executable build to avoid ODR/Double-Init issues on macOS
add_executable(vision_demo 
    examples/vision_demo.cpp
    src/context.cpp
    src/hooks/hook_manager.cpp
    src/hooks/dist_sync.cpp
    src/graph/wfg.cpp
)

# Use standard HPX target
target_link_libraries(vision_demo PRIVATE HPX::hpx)
