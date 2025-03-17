#include "core/Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace RebelCAD {
namespace Core {

std::shared_ptr<spdlog::logger> Log::s_Logger;

void Log::init() {
    spdlog::set_pattern("%^[%T] %n: %v%$");
    s_Logger = spdlog::stdout_color_mt("REBEL");
    s_Logger->set_level(spdlog::level::trace);
}

} // namespace Core
} // namespace RebelCAD
