#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include <stdexcept>

namespace RebelCAD {

class SmartResourceManager {
public:
    static SmartResourceManager& getInstance() {
        static SmartResourceManager instance;
        return instance;
    }

    // Resource registration
    template<typename T>
    void registerResource(const std::string& id, std::shared_ptr<T> resource) {
        if (m_resources.find(id) != m_resources.end()) {
            throw std::runtime_error("Resource with ID " + id + " already exists");
        }
        m_resources[id] = resource;
    }

    // Resource retrieval
    template<typename T>
    std::shared_ptr<T> getResource(const std::string& id) {
        auto it = m_resources.find(id);
        if (it == m_resources.end()) {
            throw std::runtime_error("Resource with ID " + id + " not found");
        }
        return std::static_pointer_cast<T>(it->second);
    }

    // Resource removal
    void removeResource(const std::string& id) {
        auto it = m_resources.find(id);
        if (it != m_resources.end()) {
            m_resources.erase(it);
        }
    }

    // Resource existence check
    bool hasResource(const std::string& id) const {
        return m_resources.find(id) != m_resources.end();
    }

    // Clear all resources
    void clear() {
        m_resources.clear();
    }

private:
    SmartResourceManager() = default;
    ~SmartResourceManager() = default;
    SmartResourceManager(const SmartResourceManager&) = delete;
    SmartResourceManager& operator=(const SmartResourceManager&) = delete;

    std::unordered_map<std::string, std::shared_ptr<void>> m_resources;
};

} // namespace RebelCAD
