#pragma once

#include <string>
#include <memory>
#include "core/EventBus.h"

namespace RebelCAD {
namespace Core {

/**
 * @brief Base class for all events in the system
 */
struct Event {
    virtual ~Event() = default;
};

/**
 * @brief Error codes for system events
 */
enum class ErrorCode {
    None,
    FileNotFound,
    InvalidOperation,
    ShaderCompilationFailed,
    GraphicsError,
    ValidationError
};

/**
 * @brief Base class for error events
 */
struct ErrorEvent : public Event {
    ErrorCode code;
    std::string message;

    ErrorEvent(ErrorCode code, const std::string& message)
        : code(code), message(message) {}
};

namespace UI {

/**
 * @brief Event emitted when the UI theme changes
 */
struct ThemeChangedEvent : public Event {
    std::string themeName;
    bool isDarkMode;

    ThemeChangedEvent(const std::string& name, bool dark)
        : themeName(name), isDarkMode(dark) {}
};

/**
 * @brief Event emitted when a UI validation error occurs
 */
struct ValidationErrorEvent : public ErrorEvent {
    ValidationErrorEvent(const std::string& message)
        : ErrorEvent(ErrorCode::ValidationError, message) {}
};

} // namespace UI

} // namespace Core
} // namespace RebelCAD
