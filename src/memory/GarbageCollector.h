#pragma once

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <functional>
#include "MemoryLeakDetector.h"

namespace RebelCAD::Core {

/**
 * @brief Configuration options for the Garbage Collector
 */
struct GCConfig {
    size_t memoryThreshold{1024 * 1024 * 50};  // 50MB default threshold before GC triggers
    std::chrono::milliseconds collectionInterval{1000};  // Time between collection cycles
    size_t maxPauseTime{5};  // Maximum pause time in milliseconds
    bool isIncremental{true};  // Use incremental collection to reduce pauses
};

/**
 * @brief Garbage Collection System
 * 
 * Provides automatic memory management through reference counting and cycle detection.
 * Integrates with MemoryLeakDetector for comprehensive memory tracking.
 * Features:
 * - Incremental collection to minimize pause times
 * - Configurable collection triggers and intervals
 * - Thread-safe reference counting
 * - Cycle detection for handling circular references
 * - Integration with existing memory management systems
 */
class GarbageCollector {
public:
    static GarbageCollector& getInstance();

    /**
     * @brief Registers an object for garbage collection
     * @param ptr Pointer to the object
     * @param size Object size in bytes
     * @param destructor Function to call when destroying the object
     */
    void registerObject(void* ptr, size_t size, std::function<void(void*)> destructor);

    /**
     * @brief Adds a reference from one object to another
     * @param from Source object pointer
     * @param to Target object pointer
     */
    void addReference(void* from, void* to);

    /**
     * @brief Removes a reference between objects
     * @param from Source object pointer
     * @param to Target object pointer
     */
    void removeReference(void* from, void* to);

    /**
     * @brief Manually triggers garbage collection
     * @param fullCollection If true, performs a full collection cycle
     */
    void collect(bool fullCollection = false);

    /**
     * @brief Updates configuration settings
     * @param config New configuration options
     */
    void configure(const GCConfig& config);

    /**
     * @brief Gets current memory usage statistics
     * @return Pair of (total allocated bytes, number of managed objects)
     */
    std::pair<size_t, size_t> getStats() const;

    /**
     * @brief Starts the background collection thread
     */
    void start();

    /**
     * @brief Stops the background collection thread
     */
    void stop();

private:
    GarbageCollector();
    ~GarbageCollector();
    
    GarbageCollector(const GarbageCollector&) = delete;
    GarbageCollector& operator=(const GarbageCollector&) = delete;

    struct ObjectInfo {
        size_t size;
        std::atomic<size_t> refCount;
        std::function<void(void*)> destructor;
        std::unordered_set<void*> references;
        bool marked;

        ObjectInfo(size_t sz, std::function<void(void*)> dtor)
            : size(sz), refCount(1), destructor(std::move(dtor)), marked(false) {}
    };

    void backgroundCollectionThread();
    void markPhase();
    void sweepPhase();
    bool detectCycles(void* ptr, std::unordered_set<void*>& visited);
    void incrementalCollection();

    mutable std::mutex mutex;
    std::unordered_map<void*, ObjectInfo> managedObjects;
    std::atomic<size_t> totalMemory{0};
    GCConfig config;

    // Background collection thread control
    std::thread collectorThread;
    std::condition_variable collectionTrigger;
    std::atomic<bool> running{false};
    std::atomic<bool> collectionRequested{false};
};

// Helper macros for automatic registration
#define REBEL_GC_NEW(Type, ...) \
    [&]() { \
        Type* ptr = new Type(__VA_ARGS__); \
        GarbageCollector::getInstance().registerObject( \
            ptr, sizeof(Type), \
            [](void* p) { delete static_cast<Type*>(p); } \
        ); \
        return ptr; \
    }()

#define REBEL_GC_REF(ptr) \
    [&]() { \
        if (ptr) GarbageCollector::getInstance().addReference(this, ptr); \
        return ptr; \
    }()

#define REBEL_GC_UNREF(ptr) \
    [&]() { \
        if (ptr) GarbageCollector::getInstance().removeReference(this, ptr); \
        ptr = nullptr; \
    }()

} // namespace RebelCAD::Core
