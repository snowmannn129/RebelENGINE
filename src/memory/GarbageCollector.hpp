#pragma once

#include <memory>
#include <vector>
#include <unordered_set>
#include <functional>
#include <chrono>

namespace RebelCAD {

class GarbageCollector {
public:
    static GarbageCollector& getInstance() {
        static GarbageCollector instance;
        return instance;
    }

    // Register an object for garbage collection
    template<typename T>
    void registerObject(std::shared_ptr<T> object) {
        if (!object) return;
        
        m_objects.insert(std::static_pointer_cast<void>(object));
        m_lastUsedTime[object.get()] = std::chrono::steady_clock::now();
    }

    // Unregister an object from garbage collection
    template<typename T>
    void unregisterObject(std::shared_ptr<T> object) {
        if (!object) return;
        
        m_objects.erase(std::static_pointer_cast<void>(object));
        m_lastUsedTime.erase(object.get());
    }

    // Mark object as recently used
    template<typename T>
    void markUsed(T* object) {
        if (!object) return;
        m_lastUsedTime[object] = std::chrono::steady_clock::now();
    }

    // Collect garbage based on timeout
    void collect(std::chrono::seconds timeout = std::chrono::seconds(300)) {
        auto now = std::chrono::steady_clock::now();
        
        for (auto it = m_lastUsedTime.begin(); it != m_lastUsedTime.end();) {
            if (now - it->second > timeout) {
                // Find and remove corresponding shared_ptr
                for (auto objIt = m_objects.begin(); objIt != m_objects.end(); ++objIt) {
                    if (objIt->get() == it->first) {
                        m_objects.erase(objIt);
                        break;
                    }
                }
                it = m_lastUsedTime.erase(it);
            } else {
                ++it;
            }
        }
    }

    // Clear all tracked objects
    void clear() {
        m_objects.clear();
        m_lastUsedTime.clear();
    }

private:
    GarbageCollector() = default;
    ~GarbageCollector() = default;
    GarbageCollector(const GarbageCollector&) = delete;
    GarbageCollector& operator=(const GarbageCollector&) = delete;

    std::unordered_set<std::shared_ptr<void>> m_objects;
    std::unordered_map<void*, std::chrono::steady_clock::time_point> m_lastUsedTime;
};

} // namespace RebelCAD
