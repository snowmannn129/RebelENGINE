#pragma once

#include "SmartPtr.h"

namespace RebelCAD {
namespace Core {

/**
 * @brief Enhanced weak pointer implementation with optimized memory layout
 * 
 * WeakPtr provides a non-owning reference to an object managed by SmartPtr.
 * Uses the same ControlBlock structure for efficient memory management and
 * cache-friendly access patterns.
 * 
 * @tparam T Type of the referenced object
 */
template<typename T>
class WeakPtr {
public:
    using element_type = T;
    using ControlBlockType = typename SmartPtr<T>::ControlBlockType;

    // Default constructor
    constexpr WeakPtr() noexcept : m_ctrl(nullptr) {}

    // Constructor from SmartPtr
    WeakPtr(const SmartPtr<T>& smart) noexcept : m_ctrl(smart.m_ctrl) {
        if (m_ctrl) {
            m_ctrl->addWeakRef();
        }
    }

    // Copy constructor
    WeakPtr(const WeakPtr& other) noexcept : m_ctrl(other.m_ctrl) {
        if (m_ctrl) {
            m_ctrl->addWeakRef();
        }
    }

    // Move constructor
    WeakPtr(WeakPtr&& other) noexcept : m_ctrl(other.m_ctrl) {
        other.m_ctrl = nullptr;
    }

    // Destructor
    ~WeakPtr() {
        if (m_ctrl && m_ctrl->releaseWeakRef()) {
            delete m_ctrl;
        }
    }

    // Copy assignment
    WeakPtr& operator=(const WeakPtr& other) noexcept {
        WeakPtr(other).swap(*this);
        return *this;
    }

    // Move assignment
    WeakPtr& operator=(WeakPtr&& other) noexcept {
        WeakPtr(std::move(other)).swap(*this);
        return *this;
    }

    // Assignment from SmartPtr
    WeakPtr& operator=(const SmartPtr<T>& smart) noexcept {
        WeakPtr(smart).swap(*this);
        return *this;
    }

    // Swap implementation
    void swap(WeakPtr& other) noexcept {
        std::swap(m_ctrl, other.m_ctrl);
    }

    // Lock the weak pointer to get a SmartPtr
    SmartPtr<T> lock() const noexcept {
        if (!m_ctrl || m_ctrl->expired()) {
            return SmartPtr<T>();
        }

        // Create new SmartPtr and increment reference count
        SmartPtr<T> smart;
        smart.m_ctrl = m_ctrl;
        smart.m_ctrl->addStrongRef();
        return smart;
    }

    // Utility methods
    size_t use_count() const noexcept {
        return m_ctrl ? m_ctrl->useCount() : 0;
    }

    bool expired() const noexcept {
        return !m_ctrl || m_ctrl->expired();
    }

    void reset() noexcept {
        WeakPtr().swap(*this);
    }

private:
    ControlBlockType* m_ctrl;

    template<typename U> friend class SmartPtr;
};

} // namespace Core
} // namespace RebelCAD
