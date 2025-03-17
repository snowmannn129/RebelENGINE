#pragma once

#include <unordered_map>
#include <typeindex>
#include <functional>
#include <mutex>
#include "MemoryPool.h"
#include "SmartPtr.h"
#include "WeakPtr.h"
#include "CircularReferenceDetector.h"

namespace RebelCAD {
namespace Core {

/**
 * @brief Smart Resource Manager for efficient resource lifecycle management
 * 
 * The SmartResourceManager provides RAII-style resource management with:
 * - Automatic resource cleanup
 * - Reference counting
 * - Type-safe resource handling
 * - Integration with MemoryPool for efficient allocation
 * - Custom deletion strategies
 */
class SmartResourceManager {
public:
    /**
     * @brief Get the singleton instance of SmartResourceManager
     * @return Reference to the singleton instance
     */
    static SmartResourceManager& getInstance();

    /**
     * @brief Create a managed resource of type T
     * @tparam T Resource type
     * @tparam Args Constructor argument types
     * @param args Constructor arguments
     * @return std::shared_ptr to the managed resource
     */
    template<typename T, typename... Args>
    SmartPtr<T> createResource(Args&&... args) {
        // Create resource using MemoryPool
        auto ptr = SmartPtr<T>::make(std::forward<Args>(args)...);
        
        // Track resource creation
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_resourceCount[std::type_index(typeid(T))]++;
        }

        return ptr;
    }

    /**
     * @brief Get current count of active resources of type T
     * @tparam T Resource type to count
     * @return Number of active resources
     */
    template<typename T>
    size_t getResourceCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_resourceCount.find(std::type_index(typeid(T)));
        return (it != m_resourceCount.end()) ? it->second : 0;
    }

    /**
     * @brief Register a custom cleanup handler for a resource type
     * @tparam T Resource type
     * @param handler Custom cleanup function
     */
    template<typename T>
    void registerCleanupHandler(std::function<void(T*)> handler) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cleanupHandlers[std::type_index(typeid(T))] = 
            [handler](void* ptr) { 
                handler(static_cast<T*>(ptr));
                
                // Remove from circular reference detector
                CircularReferenceDetector::getInstance().removeRelationships(ptr);
            };
    }

    /**
     * @brief Register a relationship between two resources for circular reference detection
     * 
     * @tparam T Type of source resource
     * @tparam U Type of target resource
     * @param source Source resource pointer
     * @param target Target resource pointer
     */
    template<typename T, typename U>
    void registerResourceRelationship(const T* source, const U* target) {
        CircularReferenceDetector::getInstance().registerRelationship(
            static_cast<const void*>(source),
            static_cast<const void*>(target),
            std::type_index(typeid(T)),
            std::type_index(typeid(U))
        );
    }

    /**
     * @brief Check if a resource has any circular references
     * 
     * @tparam T Type of resource to check
     * @param resource Pointer to the resource
     * @return true if the resource is part of any circular references
     */
    template<typename T>
    bool hasCircularReferences(const T* resource) const {
        return CircularReferenceDetector::getInstance().hasCircularReferences(
            static_cast<const void*>(resource)
        );
    }

private:
    SmartResourceManager() = default;
    ~SmartResourceManager() = default;
    
    // Prevent copying
    SmartResourceManager(const SmartResourceManager&) = delete;
    SmartResourceManager& operator=(const SmartResourceManager&) = delete;

    mutable std::mutex m_mutex;
    std::unordered_map<std::type_index, size_t> m_resourceCount;
    std::unordered_map<std::type_index, std::function<void(void*)>> m_cleanupHandlers;
};

} // namespace Core
} // namespace RebelCAD
