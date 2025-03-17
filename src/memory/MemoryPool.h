#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>
#include "BlockAllocator.h"

namespace RebelCAD {
namespace Core {

/**
 * @brief A fixed-size block memory pool implementation that efficiently manages memory allocations.
 * 
 * The MemoryPool class provides a thread-safe memory allocation system that:
 * - Reduces memory fragmentation by managing fixed-size blocks
 * - Improves performance by minimizing system allocation calls
 * - Supports concurrent access through thread-safe operations
 * - Tracks allocations to help detect memory leaks
 * 
 * @tparam T The type of objects to be allocated
 * @tparam BlockSize The number of objects per memory block (default: 1024)
 */
/**
 * @brief A type-safe memory pool that uses BlockAllocator for efficient memory management.
 * 
 * This class provides a typed interface over BlockAllocator, ensuring type safety
 * and proper alignment for the allocated objects. It supports:
 * - Efficient allocation and deallocation of objects of type T
 * - Automatic memory alignment based on type requirements
 * - Memory leak detection
 * - Thread-safe operations
 * 
 * @tparam T The type of objects to be allocated
 */
template<typename T>
class MemoryPool {
public:
    /**
     * @brief Constructs a new MemoryPool instance.
     * @param initialCapacity Initial number of objects the pool can hold (default: 1024)
     */
    explicit MemoryPool(size_t initialCapacity = 1024)
        : m_allocator(initialCapacity * sizeof(T), std::max(alignof(T), size_t(16))) {}

    /**
     * @brief Allocates memory for a single object of type T.
     * @return Pointer to the allocated memory
     * @throws std::bad_alloc if allocation fails
     */
    T* allocate() {
        void* memory = m_allocator.allocate(sizeof(T));
        if (!memory) {
            throw std::bad_alloc();
        }
        return static_cast<T*>(memory);
    }

    /**
     * @brief Deallocates previously allocated memory.
     * @param ptr Pointer to the memory to deallocate
     */
    void deallocate(T* ptr) {
        if (ptr) {
            m_allocator.deallocate(ptr);
        }
    }

    /**
     * @brief Returns the total amount of allocated memory in bytes.
     * @return Total allocated bytes
     */
    size_t getAllocatedSize() const {
        return m_allocator.getAllocatedSize();
    }

    /**
     * @brief Returns the total capacity of the pool in bytes.
     * @return Total capacity in bytes
     */
    size_t getCapacity() const {
        return m_allocator.getCapacity();
    }

private:
    BlockAllocator m_allocator;  ///< Underlying block allocator
};

} // namespace Core
} // namespace RebelCAD
