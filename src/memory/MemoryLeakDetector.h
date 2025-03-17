#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <atomic>

namespace RebelCAD {
namespace Core {

struct AllocationRecord {
    size_t size;
    std::string file;
    int line;
    std::string timestamp;
    
    AllocationRecord(size_t s, const char* f, int l);
};

class MemoryLeakDetector {
public:
    static MemoryLeakDetector& getInstance();
    
    // Delete copy constructor and assignment operator
    MemoryLeakDetector(const MemoryLeakDetector&) = delete;
    MemoryLeakDetector& operator=(const MemoryLeakDetector&) = delete;
    
    // Track allocation
    void* trackAllocation(void* ptr, size_t size, const char* file, int line);
    
    // Track deallocation
    void trackDeallocation(void* ptr);
    
    // Get current memory usage
    size_t getCurrentMemoryUsage() const;
    
    // Get number of active allocations
    size_t getActiveAllocations() const;
    
    // Dump memory leaks to file
    void dumpLeaks(const std::string& filename);
    
    // Check for leaks
    bool hasLeaks() const;
    
    // Enable/disable tracking
    void enableTracking(bool enable);
    
    // Clear all tracking data
    void reset();

private:
    MemoryLeakDetector();
    ~MemoryLeakDetector();
    
    std::unordered_map<void*, std::unique_ptr<AllocationRecord>> allocations_;
    mutable std::mutex mutex_;
    std::atomic<size_t> totalMemory_;
    std::atomic<bool> trackingEnabled_;
    
    std::string getCurrentTimestamp() const;
};

// Global overloads for new and delete
void* operator new(size_t size, const char* file, int line);
void* operator new[](size_t size, const char* file, int line);
void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;

// Macros for tracking allocations
#ifdef MEMORY_LEAK_DETECTION
    #define new new(__FILE__, __LINE__)
#endif

} // namespace Core
} // namespace RebelCAD
