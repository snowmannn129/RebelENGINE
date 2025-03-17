#pragma once

#include <string>

namespace RebelCAD {
namespace UI {

// Event types
struct PreviewUpdateEvent {
    static constexpr const char* type = "PreviewUpdate";
};

struct ValidationErrorEvent {
    static constexpr const char* type = "ValidationError";
    std::string message;
    ValidationErrorEvent(const std::string& msg) : message(msg) {}
};

struct OperationCompleteEvent {
    static constexpr const char* type = "OperationComplete";
};

} // namespace UI
} // namespace RebelCAD
