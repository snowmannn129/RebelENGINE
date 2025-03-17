#include "core/EventSystem.hpp"
#include "core/Log.h"
#include <mutex>

namespace RebelCAD {
namespace Core {

namespace {
    std::mutex g_eventMutex;
}

EventSystem& EventSystem::getInstance() {
    static EventSystem instance;
    return instance;
}

EventHandle EventSystem::registerHandler(const std::string& eventType, EventHandler handler) {
    std::lock_guard<std::mutex> lock(g_eventMutex);
    
    EventHandle handle = m_nextHandle++;
    m_handlers[eventType].push_back({handle, handler});
    
    REBEL_LOG_INFO("Registered event handler for type '{}' with handle {}", eventType, handle);
    return handle;
}

void EventSystem::unregisterHandler(EventHandle handle) {
    std::lock_guard<std::mutex> lock(g_eventMutex);
    
    for (auto& [type, handlers] : m_handlers) {
        auto originalSize = handlers.size();
        handlers.erase(
            std::remove_if(
                handlers.begin(),
                handlers.end(),
                [handle](const auto& h) { return h.first == handle; }
            ),
            handlers.end()
        );
        
        if (handlers.size() < originalSize) {
            REBEL_LOG_INFO("Unregistered event handler {} for type '{}'", handle, type);
        }
    }
}

void EventSystem::dispatchEvent(const Event& event) {
    std::lock_guard<std::mutex> lock(g_eventMutex);
    
    auto it = m_handlers.find(event.getType());
    if (it != m_handlers.end()) {
        REBEL_LOG_TRACE("Dispatching event of type '{}' to {} handlers", 
                  event.getType(), it->second.size());
        
        for (const auto& [handle, handler] : it->second) {
            try {
                handler(event);
            } catch (const std::exception& e) {
                REBEL_LOG_ERROR("Exception in event handler {}: {}", handle, e.what());
            } catch (...) {
                REBEL_LOG_ERROR("Unknown exception in event handler {}", handle);
            }
        }
    } else {
        REBEL_LOG_TRACE("No handlers registered for event type '{}'", event.getType());
    }
}

} // namespace Core
} // namespace RebelCAD
