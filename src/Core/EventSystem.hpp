#pragma once

#include <functional>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

namespace RebelCAD {
namespace Core {

// Forward declaration
class Event;

// Event handler type definition
using EventHandler = std::function<void(const Event&)>;
using EventHandle = size_t;

class Event {
public:
    Event(const std::string& type) : m_type(type) {}
    virtual ~Event() = default;

    const std::string& getType() const { return m_type; }

private:
    std::string m_type;
};

class EventSystem {
public:
    static EventSystem& getInstance() {
        static EventSystem instance;
        return instance;
    }

    // Register an event handler
    EventHandle registerHandler(const std::string& eventType, EventHandler handler) {
        EventHandle handle = m_nextHandle++;
        m_handlers[eventType].push_back({handle, handler});
        return handle;
    }

    // Unregister an event handler
    void unregisterHandler(EventHandle handle) {
        for (auto& [type, handlers] : m_handlers) {
            handlers.erase(
                std::remove_if(
                    handlers.begin(),
                    handlers.end(),
                    [handle](const auto& h) { return h.first == handle; }
                ),
                handlers.end()
            );
        }
    }

    // Dispatch an event
    void dispatchEvent(const Event& event) {
        auto it = m_handlers.find(event.getType());
        if (it != m_handlers.end()) {
            for (const auto& [handle, handler] : it->second) {
                handler(event);
            }
        }
    }

private:
    EventSystem() : m_nextHandle(1) {}
    ~EventSystem() = default;
    EventSystem(const EventSystem&) = delete;
    EventSystem& operator=(const EventSystem&) = delete;

    std::unordered_map<std::string, std::vector<std::pair<EventHandle, EventHandler>>> m_handlers;
    EventHandle m_nextHandle;
};

// Specialized event types
class PreviewUpdateEvent : public Event {
public:
    PreviewUpdateEvent() : Event("PreviewUpdate") {}
};

class OperationCompleteEvent : public Event {
public:
    OperationCompleteEvent() : Event("OperationComplete") {}
};

class ErrorEvent : public Event {
public:
    ErrorEvent(const std::string& message) 
        : Event("Error"), m_message(message) {}
    
    const std::string& getMessage() const { return m_message; }

private:
    std::string m_message;
};

// UI-specific events
class ThemeChangedEvent : public Event {
public:
    ThemeChangedEvent(const std::string& oldTheme, const std::string& newTheme)
        : Event("ThemeChanged"), oldTheme(oldTheme), newTheme(newTheme) {}

    const std::string& getOldTheme() const { return oldTheme; }
    const std::string& getNewTheme() const { return newTheme; }

private:
    std::string oldTheme;
    std::string newTheme;
};

} // namespace Core
} // namespace RebelCAD
