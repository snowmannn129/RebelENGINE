#pragma once

#include <atomic>
#include <type_traits>
#include <memory>
#include <new>
#include "MemoryPool.h"
#include "CircularReferenceDetector.h"

namespace RebelCAD {
namespace Core {

/**
 * @brief Control block for smart pointer management
 * 
 * Combines reference counting and object storage in a single aligned allocation
 * for better cache performance and reduced memory fragmentation.
 */
template<typename T>
struct alignas(64) ControlBlock {
    struct RefCount {
        std::atomic<size_t> strong{1};
        std::atomic<size_t> weak{0};
        std::atomic<bool> expired{false};
    } refs;

    union {
        T object;
        uint8_t storage[sizeof(T)];
    };

    ControlBlock() noexcept {}
    ~ControlBlock() noexcept {}

    // Placement new operator for aligned allocation
    static void* operator new(size_t size) {
        void* ptr = MemoryPool<ControlBlock>().allocate();
        if (!ptr) throw std::bad_alloc();
        return ptr;
    }

    static void operator delete(void* ptr) noexcept {
        MemoryPool<ControlBlock>().deallocate(static_cast<ControlBlock*>(ptr));
    }

    // Reference counting operations
    void addStrongRef() noexcept {
        refs.strong.fetch_add(1, std::memory_order_relaxed);
    }

    bool releaseStrongRef() noexcept {
        if (refs.strong.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            refs.expired.store(true, std::memory_order_release);
            return true;
        }
        return false;
    }

    void addWeakRef() noexcept {
        refs.weak.fetch_add(1, std::memory_order_relaxed);
    }

    bool releaseWeakRef() noexcept {
        return refs.weak.fetch_sub(1, std::memory_order_acq_rel) == 1;
    }

    bool expired() const noexcept {
        return refs.expired.load(std::memory_order_acquire);
    }

    size_t useCount() const noexcept {
        return refs.strong.load(std::memory_order_relaxed);
    }
};

/**
 * @brief Enhanced smart pointer with optimized memory layout and cycle detection
 * 
 * Features:
 * - Single allocation for control block and object
 * - Cache-friendly memory alignment
 * - Automatic circular reference detection
 * - Thread-safe operations with minimal contention
 * 
 * @tparam T Type of the managed object
 */
template<typename T>
class SmartPtr {
public:
    using element_type = T;
    using ControlBlockType = ControlBlock<T>;

    // Constructors for nullptr
    constexpr SmartPtr() noexcept : m_ctrl(nullptr) {}
    constexpr SmartPtr(std::nullptr_t) noexcept : m_ctrl(nullptr) {}

    // Constructor for raw pointer (deprecated - use make() instead)
    explicit SmartPtr(T* ptr) = delete;

    // Copy constructor
    SmartPtr(const SmartPtr& other) noexcept : m_ctrl(other.m_ctrl) {
        if (m_ctrl) {
            m_ctrl->addStrongRef();
        }
    }

    // Move constructor
    SmartPtr(SmartPtr&& other) noexcept : m_ctrl(other.m_ctrl) {
        other.m_ctrl = nullptr;
    }

    // Destructor
    ~SmartPtr() {
        if (m_ctrl && m_ctrl->releaseStrongRef()) {
            // Remove from cycle detector before destruction
            CircularReferenceDetector::getInstance().removeRelationships(&m_ctrl->object);
            
            // Destroy object
            m_ctrl->object.~T();
            
            // Clean up control block if no weak references
            if (m_ctrl->releaseWeakRef()) {
                delete m_ctrl;
            }
        }
    }

    // Copy assignment
    SmartPtr& operator=(const SmartPtr& other) noexcept {
        if (this != &other) {
            SmartPtr(other).swap(*this);
        }
        return *this;
    }

    // Move assignment
    SmartPtr& operator=(SmartPtr&& other) noexcept {
        SmartPtr(std::move(other)).swap(*this);
        return *this;
    }

    // Dereference operators
    T& operator*() const noexcept { return m_ctrl->object; }
    T* operator->() const noexcept { return &m_ctrl->object; }

    // Utility methods
    T* get() const noexcept { return m_ctrl ? &m_ctrl->object : nullptr; }
    size_t use_count() const noexcept { return m_ctrl ? m_ctrl->useCount() : 0; }
    bool unique() const noexcept { return use_count() == 1; }
    explicit operator bool() const noexcept { return m_ctrl != nullptr; }
    
    // Reset pointer to null
    void reset() noexcept {
        SmartPtr().swap(*this);
    }

    // Comparison operators
    friend bool operator==(const SmartPtr& lhs, std::nullptr_t) noexcept {
        return !lhs;
    }
    
    friend bool operator==(std::nullptr_t, const SmartPtr& rhs) noexcept {
        return !rhs;
    }
    
    friend bool operator!=(const SmartPtr& lhs, std::nullptr_t) noexcept {
        return static_cast<bool>(lhs);
    }
    
    friend bool operator!=(std::nullptr_t, const SmartPtr& rhs) noexcept {
        return static_cast<bool>(rhs);
    }

    // Swap implementation
    void swap(SmartPtr& other) noexcept {
        std::swap(m_ctrl, other.m_ctrl);
    }

    // Factory method using MemoryPool with single allocation
    template<typename... Args>
    static SmartPtr<T> make(Args&&... args) {
        // Allocate control block
        auto* ctrl = new ControlBlock<T>();
        
        try {
            // Construct object in-place
            new (&ctrl->object) T(std::forward<Args>(args)...);
            
            // Create smart pointer
            SmartPtr<T> ptr;
            ptr.m_ctrl = ctrl;
            
            // Register with cycle detector
            CircularReferenceDetector::getInstance()
                .registerRelationship(&ctrl->object, nullptr,
                    std::type_index(typeid(T)), std::type_index(typeid(void)));
            
            return ptr;
        } catch (...) {
            delete ctrl;
            throw;
        }
    }

private:
    ControlBlockType* m_ctrl;

    // Friend declarations
    template<typename U> friend class WeakPtr;
    template<typename U> friend class SmartPtr;
};

} // namespace Core
} // namespace RebelCAD
