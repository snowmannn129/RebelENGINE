#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>
#include <memory>
#include <typeindex>
#include <functional>
#include <chrono>
#include "SmartPtr.h"
#include "WeakPtr.h"

namespace RebelCAD {
namespace Core {

/**
 * @brief Detects and helps prevent circular references in smart pointers
 * 
 * This class maintains a graph of object relationships to detect potential
 * circular references. It uses weak references to track relationships without
 * creating strong reference cycles.
 */
class CircularReferenceDetector {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static CircularReferenceDetector& getInstance() {
        static CircularReferenceDetector instance;
        return instance;
    }

    /**
     * @brief Register a relationship between two objects
     * 
     * @param source Pointer to the source object
     * @param target Pointer to the target object
     * @param sourceType Type information for the source object
     * @param targetType Type information for the target object
     */
    void registerRelationship(const void* source, const void* target,
                            const std::type_index& sourceType,
                            const std::type_index& targetType) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Create or update relationship entry
        auto& relationships = m_objectRelationships[source];
        relationships.emplace_back(RelationshipInfo{
            target,
            targetType,
            std::chrono::steady_clock::now()
        });

        // Check for cycles
        if (detectCycle(source)) {
            // Log warning about potential circular reference
            logCircularReference(source, sourceType, target, targetType);
        }
    }

    /**
     * @brief Remove all relationships for an object
     * @param ptr Pointer to the object being destroyed
     */
    void removeRelationships(const void* ptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_objectRelationships.erase(ptr);
        
        // Remove references to this object from other relationships
        for (auto& [_, relationships] : m_objectRelationships) {
            relationships.erase(
                std::remove_if(relationships.begin(), relationships.end(),
                    [ptr](const RelationshipInfo& info) {
                        return info.target == ptr;
                    }),
                relationships.end()
            );
        }
    }

    /**
     * @brief Check if an object is part of any circular references
     * @param ptr Pointer to the object to check
     * @return true if the object is part of a circular reference
     */
    bool hasCircularReferences(const void* ptr) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return detectCycle(ptr);
    }

    /**
     * @brief Get all objects involved in circular references
     * @return Vector of pointers to objects in circular references
     */
    std::vector<const void*> getCircularReferenceObjects() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<const void*> result;
        
        for (const auto& [ptr, _] : m_objectRelationships) {
            if (detectCycle(ptr)) {
                result.push_back(ptr);
            }
        }
        
        return result;
    }

private:
    struct RelationshipInfo {
        const void* target;
        std::type_index targetType;
        std::chrono::steady_clock::time_point creationTime;
    };

    mutable std::mutex m_mutex;
    std::unordered_map<const void*, std::vector<RelationshipInfo>> m_objectRelationships;

    // Private constructor for singleton
    CircularReferenceDetector() = default;

    /**
     * @brief Detect if there's a cycle starting from the given object
     * 
     * Uses depth-first search to detect cycles in the relationship graph
     */
    bool detectCycle(const void* start) const {
        std::unordered_set<const void*> visited;
        std::unordered_set<const void*> recursionStack;
        return dfsDetectCycle(start, visited, recursionStack);
    }

    bool dfsDetectCycle(const void* current,
                       std::unordered_set<const void*>& visited,
                       std::unordered_set<const void*>& recursionStack) const {
        visited.insert(current);
        recursionStack.insert(current);
        
        auto it = m_objectRelationships.find(current);
        if (it != m_objectRelationships.end()) {
            for (const auto& rel : it->second) {
                if (visited.find(rel.target) == visited.end()) {
                    if (dfsDetectCycle(rel.target, visited, recursionStack)) {
                        return true;
                    }
                } else if (recursionStack.find(rel.target) != recursionStack.end()) {
                    return true;
                }
            }
        }
        
        recursionStack.erase(current);
        return false;
    }

    /**
     * @brief Log information about detected circular references
     */
    void logCircularReference(const void* source, const std::type_index& sourceType,
                            const void* target, const std::type_index& targetType) const {
        // TODO: Integrate with logging system
        // For now, we'll just store the information for debugging
        m_circularReferenceLog.push_back({
            source,
            sourceType,
            target,
            targetType,
            std::chrono::steady_clock::now()
        });
    }

    // Structure for logging circular reference information
    struct CircularReferenceLogEntry {
        const void* source;
        std::type_index sourceType;
        const void* target;
        std::type_index targetType;
        std::chrono::steady_clock::time_point detectionTime;
    };

    mutable std::vector<CircularReferenceLogEntry> m_circularReferenceLog;
};

} // namespace Core
} // namespace RebelCAD
