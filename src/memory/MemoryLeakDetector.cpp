#include "core/MemoryLeakDetector.h"
#include "core/Log.h"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace RebelCAD {
namespace Core {

AllocationRecord::AllocationRecord(size_t s, const char* f, int l)
    : size(s), file(f), line(l) {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    timestamp = ss.str();
}

MemoryLeakDetector& MemoryLeakDetector::getInstance() {
    static MemoryLeakDetector instance;
    return instance;
}

MemoryLeakDetector::MemoryLeakDetector()
    : totalMemory_(0), trackingEnabled_(true) {}

MemoryLeakDetector::~MemoryLeakDetector() {
    if (hasLeaks()) {
        dumpLeaks("memory_leaks.txt");
    }
}

void* MemoryLeakDetector::trackAllocation(void* ptr, size_t size, const char* file, int line) {
    if (!trackingEnabled_) {
        return ptr;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    allocations_[ptr] = std::make_unique<AllocationRecord>(size, file, line);
    totalMemory_ += size;
    return ptr;
}

void MemoryLeakDetector::trackDeallocation(void* ptr) {
    if (!trackingEnabled_ || !ptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = allocations_.find(ptr);
    if (it != allocations_.end()) {
        totalMemory_ -= it->second->size;
        allocations_.erase(it);
    }
}

size_t MemoryLeakDetector::getCurrentMemoryUsage() const {
    return totalMemory_;
}

size_t MemoryLeakDetector::getActiveAllocations() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return allocations_.size();
}

void MemoryLeakDetector::dumpLeaks(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (allocations_.empty()) {
        return;
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        // Log error using RebelCAD logging system
        return;
    }

    file << "Memory Leak Report - " << getCurrentTimestamp() << "\n";
    file << "----------------------------------------\n\n";
    file << "Total Memory Leaks: " << allocations_.size() << "\n";
    file << "Total Memory Leaked: " << totalMemory_ << " bytes\n\n";

    for (const auto& [ptr, record] : allocations_) {
        file << "Leak at address: " << ptr << "\n"
             << "Size: " << record->size << " bytes\n"
             << "File: " << record->file << ":" << record->line << "\n"
             << "Allocation Time: " << record->timestamp << "\n"
             << "----------------------------------------\n";
    }

    file.close();
}

bool MemoryLeakDetector::hasLeaks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return !allocations_.empty();
}

void MemoryLeakDetector::enableTracking(bool enable) {
    trackingEnabled_ = enable;
}

void MemoryLeakDetector::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    allocations_.clear();
    totalMemory_ = 0;
}

std::string MemoryLeakDetector::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Global operator overloads
void* operator new(size_t size, const char* file, int line) {
    void* ptr = std::malloc(size);
    if (!ptr) {
        throw std::bad_alloc();
    }
    return MemoryLeakDetector::getInstance().trackAllocation(ptr, size, file, line);
}

void* operator new[](size_t size, const char* file, int line) {
    void* ptr = std::malloc(size);
    if (!ptr) {
        throw std::bad_alloc();
    }
    return MemoryLeakDetector::getInstance().trackAllocation(ptr, size, file, line);
}

void operator delete(void* ptr) noexcept {
    if (ptr) {
        MemoryLeakDetector::getInstance().trackDeallocation(ptr);
        std::free(ptr);
    }
}

void operator delete[](void* ptr) noexcept {
    if (ptr) {
        MemoryLeakDetector::getInstance().trackDeallocation(ptr);
        std::free(ptr);
    }
}

} // namespace Core
} // namespace RebelCAD
