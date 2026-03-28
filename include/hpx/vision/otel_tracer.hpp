#pragma once
#include <hpx/config.hpp>
#include <hpx/vision/causality_id.hpp>
#include <string>
#include <chrono>
#include <fstream>
#include <mutex>

namespace hpx::vision {

/**
 * @brief Simple OpenTelemetry-ready tracer for HPX-Vision
 * Logs spans to a file in structured JSON format.
 */
class otel_tracer {
public:
    static otel_tracer& instance() {
        static otel_tracer inst;
        return inst;
    }

    struct span {
        std::string name;
        causality_id cid;
        std::chrono::steady_clock::time_point start_time;
        
        span(std::string const& name, causality_id cid) 
            : name(name), cid(cid), start_time(std::chrono::steady_clock::now()) {}

        ~span() {
            auto end_time = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
            otel_tracer::instance().log_span(name, cid, duration.count());
        }
    };

    void log_span(std::string const& name, causality_id cid, std::uint64_t duration_us) {
        std::lock_guard lock(mutex_);
        if (!out_.is_open()) {
            out_.open("vision_traces.json", std::ios::app);
        }
        
        out_ << "{\"span\": \"" << name << "\", "
             << "\"cid\": \"" << std::to_string(cid.data) << "\", "
             << "\"duration_us\": " << duration_us << ", "
             << "\"timestamp\": " << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) 
             << "}\n";
        out_.flush();
    }

private:
    otel_tracer() = default;
    std::ofstream out_;
    std::mutex mutex_;
};

#define VISION_SPAN(name, cid) hpx::vision::otel_tracer::span _vspan(name, cid)

} // namespace hpx::vision
