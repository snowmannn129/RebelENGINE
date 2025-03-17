#pragma once

#include <string>
#include <stdexcept>

namespace RebelCAD {
namespace Core {

enum class ErrorCode {
    None = 0,
    Unknown,
    InvalidArgument,
    InvalidOperation,
    OutOfMemory,
    FileNotFound,
    FileIOError,
    ShaderCompilationFailed,
    GraphicsError,
    // Add more error codes as needed
};

class Error : public std::runtime_error {
public:
    Error(ErrorCode code, const std::string& message)
        : std::runtime_error(message)
        , code_(code) {}

    ErrorCode getCode() const { return code_; }

    static void throwError(ErrorCode code, const std::string& message) {
        throw Error(code, message);
    }

private:
    ErrorCode code_;
};

// Convenience macro for throwing errors
#define REBEL_THROW_ERROR(code, message) \
    RebelCAD::Core::Error::throwError(RebelCAD::Core::ErrorCode::code, message)

} // namespace Core
} // namespace RebelCAD
