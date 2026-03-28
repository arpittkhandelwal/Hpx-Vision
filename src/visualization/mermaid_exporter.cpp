#include <hpx/hpx.hpp>
#include <hpx/vision/wait_for_graph.hpp>
#include <hpx/vision/causality_id.hpp>
#include <iostream>
#include <string>
#include <sstream>

namespace hpx {
namespace vision {

/**
 * @brief Exports the Wait-For Graph (WFG) into a Mermaid.js string
 */
class mermaid_exporter {
public:
    static std::string to_string() {
        std::stringstream ss;
        ss << "graph TD\n";
        
        auto& graph = wait_for_graph::instance();
        // Since we don't have a way to iterate over all nodes in wait_for_graph yet,
        // we'll need a better iterator for the WFG.
        // For this demo, we'll assume a simplified traversal.
        
        ss << "  %% Wait-For Graph (WFG)\n";
        return ss.str();
    }
};

} // namespace vision
} // namespace hpx
