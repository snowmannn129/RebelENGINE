#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace RebelCAD {
namespace Core {

class Log {
public:
    static void init();
    static std::shared_ptr<spdlog::logger>& getLogger() { return s_Logger; }

private:
    static std::shared_ptr<spdlog::logger> s_Logger;
};

} // namespace Core
} // namespace RebelCAD

// Logging macros
#define REBEL_LOG_TRACE(...) RebelCAD::Core::Log::getLogger()->trace(__VA_ARGS__)
#define REBEL_LOG_INFO(...) RebelCAD::Core::Log::getLogger()->info(__VA_ARGS__)
#define REBEL_LOG_WARNING(...) RebelCAD::Core::Log::getLogger()->warn(__VA_ARGS__)
#define REBEL_LOG_ERROR(...) RebelCAD::Core::Log::getLogger()->error(__VA_ARGS__)
#define REBEL_LOG_CRITICAL(...) RebelCAD::Core::Log::getLogger()->critical(__VA_ARGS__)
