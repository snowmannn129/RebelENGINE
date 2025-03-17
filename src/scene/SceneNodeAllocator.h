#pragma once

#include "core/MemoryPool.h"
#include <memory>

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Custom allocator for SceneNode that uses MemoryPool for efficient memory management
 */
template<typename T>
class SceneNodeAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    
    template<typename U>
    struct rebind {
        using other = SceneNodeAllocator<U>;
    };

    SceneNodeAllocator() noexcept {
        if (!s_pool) {
            s_pool = std::make_unique<Core::MemoryPool<T>>();
        }
    }
    
    template<typename U>
    SceneNodeAllocator(const SceneNodeAllocator<U>&) noexcept {}

    pointer allocate(size_type n) {
        if (n != 1) {
            throw std::bad_alloc(); // Only support single object allocation
        }
        return s_pool->allocate();
    }

    void deallocate(pointer p, size_type) noexcept {
        s_pool->deallocate(p);
    }

    void construct(pointer p, const std::string& name) {
        new(p) value_type(name);
    }

    template<typename U>
    void destroy(U* p) {
        p->~U();
    }

    bool operator==(const SceneNodeAllocator&) const noexcept {
        return true;
    }
    
    bool operator!=(const SceneNodeAllocator&) const noexcept {
        return false;
    }

private:
    static std::unique_ptr<Core::MemoryPool<T>> s_pool;
};

template<typename T>
std::unique_ptr<Core::MemoryPool<T>> SceneNodeAllocator<T>::s_pool;

} // namespace Graphics
} // namespace RebelCAD
