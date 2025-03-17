#pragma once

#include "BlockAllocator.h"
#include <memory>
#include <type_traits>

namespace RebelCAD {
namespace Core {

/**
 * @brief Custom STL-compatible allocator that uses BlockAllocator
 * 
 * This allocator wrapper allows STL containers to use our optimized BlockAllocator
 * for memory management. It handles proper sizing and alignment requirements while
 * maintaining the STL allocator requirements.
 */
template<typename T>
class CustomAllocator {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal = std::false_type;

    template<typename U>
    struct rebind {
        using other = CustomAllocator<U>;
    };

    CustomAllocator() noexcept : m_allocator(nullptr) {
        // Default constructor - allocator must be set before use
    }

    explicit CustomAllocator(BlockAllocator& allocator) noexcept 
        : m_allocator(&allocator) {}

    template<typename U>
    CustomAllocator(const CustomAllocator<U>& other) noexcept 
        : m_allocator(other.getAllocator()) {}

    T* allocate(size_type n) {
        if (n > std::numeric_limits<size_type>::max() / sizeof(T)) {
            throw std::bad_alloc();
        }
        
        if (m_allocator == nullptr) {
            throw std::runtime_error("Allocator not initialized");
        }

        void* ptr = m_allocator->allocate(n * sizeof(T));
        if (!ptr) {
            throw std::bad_alloc();
        }
        
        return static_cast<T*>(ptr);
    }

    void deallocate(T* p, size_type) noexcept {
        if (m_allocator && p) {
            m_allocator->deallocate(p);
        }
    }

    template<typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        ::new(static_cast<void*>(p)) U(std::forward<Args>(args)...);
    }

    template<typename U>
    void destroy(U* p) {
        p->~U();
    }

    BlockAllocator* getAllocator() const noexcept {
        return m_allocator;
    }

    bool operator==(const CustomAllocator& other) const noexcept {
        return m_allocator == other.m_allocator;
    }

    bool operator!=(const CustomAllocator& other) const noexcept {
        return m_allocator != other.m_allocator;
    }

private:
    BlockAllocator* m_allocator;
};

} // namespace Core
} // namespace RebelCAD
