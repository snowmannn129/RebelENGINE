#include "core/GarbageCollector.h"
#include <algorithm>
#include <stack>

namespace RebelCAD::Core {

GarbageCollector& GarbageCollector::getInstance() {
    static GarbageCollector instance;
    return instance;
}

GarbageCollector::GarbageCollector() : config() {
    start();
}

GarbageCollector::~GarbageCollector() {
    stop();
}

void GarbageCollector::start() {
    if (!running) {
        running = true;
        collectorThread = std::thread(&GarbageCollector::backgroundCollectionThread, this);
    }
}

void GarbageCollector::stop() {
    if (running) {
        running = false;
        collectionTrigger.notify_one();
        if (collectorThread.joinable()) {
            collectorThread.join();
        }
    }
}

void GarbageCollector::registerObject(void* ptr, size_t size, std::function<void(void*)> destructor) {
    if (!ptr) return;

    std::lock_guard<std::mutex> lock(mutex);
    managedObjects.emplace(ptr, ObjectInfo(size, std::move(destructor)));
    totalMemory += size;

    // Notify MemoryLeakDetector for comprehensive tracking
    MemoryLeakDetector::getInstance().recordAllocation(ptr, size, __FILE__, __LINE__);
}

void GarbageCollector::addReference(void* from, void* to) {
    if (!from || !to) return;

    std::lock_guard<std::mutex> lock(mutex);
    auto fromIt = managedObjects.find(from);
    auto toIt = managedObjects.find(to);
    
    if (fromIt != managedObjects.end() && toIt != managedObjects.end()) {
        fromIt->second.references.insert(to);
        toIt->second.refCount++;
    }
}

void GarbageCollector::removeReference(void* from, void* to) {
    if (!from || !to) return;

    std::lock_guard<std::mutex> lock(mutex);
    auto fromIt = managedObjects.find(from);
    auto toIt = managedObjects.find(to);
    
    if (fromIt != managedObjects.end() && toIt != managedObjects.end()) {
        fromIt->second.references.erase(to);
        if (toIt->second.refCount > 0) {
            toIt->second.refCount--;
        }
    }
}

void GarbageCollector::collect(bool fullCollection) {
    if (fullCollection) {
        std::lock_guard<std::mutex> lock(mutex);
        markPhase();
        sweepPhase();
    } else {
        incrementalCollection();
    }
}

void GarbageCollector::configure(const GCConfig& newConfig) {
    std::lock_guard<std::mutex> lock(mutex);
    config = newConfig;
}

std::pair<size_t, size_t> GarbageCollector::getStats() const {
    std::lock_guard<std::mutex> lock(mutex);
    return {totalMemory, managedObjects.size()};
}

void GarbageCollector::backgroundCollectionThread() {
    while (running) {
        std::unique_lock<std::mutex> lock(mutex);
        
        auto waitResult = collectionTrigger.wait_for(lock, 
            config.collectionInterval,
            [this]() { 
                return !running || 
                       collectionRequested || 
                       totalMemory >= config.memoryThreshold; 
            });

        if (!running) break;

        if (waitResult || collectionRequested) {
            collectionRequested = false;
            if (config.isIncremental) {
                incrementalCollection();
            } else {
                markPhase();
                sweepPhase();
            }
        }
    }
}

void GarbageCollector::markPhase() {
    // Reset all mark flags
    for (auto& [ptr, info] : managedObjects) {
        info.marked = false;
    }

    // Mark all objects reachable from root set (refCount > 0)
    std::stack<void*> workList;
    for (const auto& [ptr, info] : managedObjects) {
        if (info.refCount > 0) {
            workList.push(ptr);
        }
    }

    while (!workList.empty()) {
        void* current = workList.top();
        workList.pop();

        auto it = managedObjects.find(current);
        if (it != managedObjects.end() && !it->second.marked) {
            it->second.marked = true;
            for (void* ref : it->second.references) {
                workList.push(ref);
            }
        }
    }
}

void GarbageCollector::sweepPhase() {
    auto it = managedObjects.begin();
    while (it != managedObjects.end()) {
        if (!it->second.marked) {
            // Object is unreachable, clean it up
            void* ptr = it->first;
            size_t size = it->second.size;
            
            // Call destructor
            if (it->second.destructor) {
                it->second.destructor(ptr);
            }

            // Update memory tracking
            totalMemory -= size;
            MemoryLeakDetector::getInstance().recordDeallocation(ptr);

            // Remove from managed objects
            it = managedObjects.erase(it);
        } else {
            ++it;
        }
    }
}

bool GarbageCollector::detectCycles(void* ptr, std::unordered_set<void*>& visited) {
    if (!ptr) return false;

    if (visited.find(ptr) != visited.end()) {
        return true; // Cycle detected
    }

    visited.insert(ptr);
    auto it = managedObjects.find(ptr);
    if (it != managedObjects.end()) {
        for (void* ref : it->second.references) {
            if (detectCycles(ref, visited)) {
                return true;
            }
        }
    }
    visited.erase(ptr);
    return false;
}

void GarbageCollector::incrementalCollection() {
    static size_t lastPosition = 0;
    const size_t batchSize = 100; // Process 100 objects per increment
    
    std::lock_guard<std::mutex> lock(mutex);
    
    auto start = std::chrono::steady_clock::now();
    size_t processed = 0;
    
    // Convert map to vector for indexed access
    std::vector<std::pair<void*, ObjectInfo*>> objects;
    for (auto& [ptr, info] : managedObjects) {
        objects.emplace_back(ptr, &info);
    }
    
    while (processed < batchSize && 
           lastPosition < objects.size() && 
           std::chrono::steady_clock::now() - start < 
               std::chrono::milliseconds(config.maxPauseTime)) {
        
        auto& [ptr, info] = objects[lastPosition];
        
        // Check if object is unreachable
        if (info->refCount == 0) {
            std::unordered_set<void*> visited;
            if (!detectCycles(ptr, visited)) {
                // No cycles, safe to collect
                if (info->destructor) {
                    info->destructor(ptr);
                }
                totalMemory -= info->size;
                MemoryLeakDetector::getInstance().recordDeallocation(ptr);
                managedObjects.erase(ptr);
            }
        }
        
        lastPosition++;
        processed++;
    }
    
    // Reset position if we've processed all objects
    if (lastPosition >= objects.size()) {
        lastPosition = 0;
    }
}

} // namespace RebelCAD::Core
