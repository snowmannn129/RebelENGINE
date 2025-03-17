#pragma once

#include <memory>
#include <vector>
#include <cstddef>
#include <stdexcept>
#include <mutex>

namespace RebelCAD {

class MemoryPool {
public:
    static MemoryPool& getInstance() {
        static MemoryPool instance;
        return instance;
    }

    // Allocate memory from pool
    void* allocate(size_t size) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Find best-fit block
        for (auto& block : m_blocks) {
            if (!block.used && block.size >= size) {
                // Split block if significantly larger
                if (block.size > size + sizeof(Block)) {
                    Block newBlock;
                    newBlock.data = block.data + size;
                    newBlock.size = block.size - size;
                    newBlock.used = false;
                    
                    block.size = size;
                    m_blocks.push_back(newBlock);
                }
                
                block.used = true;
                return block.data;
            }
        }
        
        // Allocate new block if no suitable block found
        size_t blockSize = std::max(size, m_minBlockSize);
        uint8_t* data = new uint8_t[blockSize];
        
        Block block;
        block.data = data;
        block.size = blockSize;
        block.used = true;
        m_blocks.push_back(block);
        
        return data;
    }

    // Deallocate memory
    void deallocate(void* ptr) {
        if (!ptr) return;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Find and mark block as unused
        for (auto& block : m_blocks) {
            if (block.data == ptr) {
                block.used = false;
                
                // Merge with adjacent free blocks
                mergeAdjacentBlocks();
                return;
            }
        }
        
        throw std::runtime_error("Invalid pointer deallocation attempt");
    }

    // Allocate and construct object
    template<typename T, typename... Args>
    T* create(Args&&... args) {
        void* ptr = allocate(sizeof(T));
        return new(ptr) T(std::forward<Args>(args)...);
    }

    // Destroy and deallocate object
    template<typename T>
    void destroy(T* ptr) {
        if (!ptr) return;
        ptr->~T();
        deallocate(ptr);
    }

    // Clear all memory
    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        for (auto& block : m_blocks) {
            delete[] block.data;
        }
        m_blocks.clear();
    }

private:
    struct Block {
        uint8_t* data = nullptr;
        size_t size = 0;
        bool used = false;
    };

    MemoryPool(size_t minBlockSize = 1024) : m_minBlockSize(minBlockSize) {}
    ~MemoryPool() { clear(); }
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    void mergeAdjacentBlocks() {
        bool merged;
        do {
            merged = false;
            for (auto it = m_blocks.begin(); it != m_blocks.end(); ++it) {
                if (!it->used) {
                    auto nextIt = std::next(it);
                    if (nextIt != m_blocks.end() && !nextIt->used) {
                        it->size += nextIt->size;
                        delete[] nextIt->data;
                        m_blocks.erase(nextIt);
                        merged = true;
                        break;
                    }
                }
            }
        } while (merged);
    }

    std::vector<Block> m_blocks;
    size_t m_minBlockSize;
    std::mutex m_mutex;
};

// Smart pointer for memory pool allocations
template<typename T>
class PoolPtr {
public:
    template<typename... Args>
    static PoolPtr<T> create(Args&&... args) {
        return PoolPtr<T>(MemoryPool::getInstance().create<T>(std::forward<Args>(args)...));
    }

    PoolPtr() : m_ptr(nullptr) {}
    explicit PoolPtr(T* ptr) : m_ptr(ptr) {}
    ~PoolPtr() { reset(); }

    PoolPtr(const PoolPtr&) = delete;
    PoolPtr& operator=(const PoolPtr&) = delete;

    PoolPtr(PoolPtr&& other) noexcept : m_ptr(other.m_ptr) {
        other.m_ptr = nullptr;
    }

    PoolPtr& operator=(PoolPtr&& other) noexcept {
        if (this != &other) {
            reset();
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
        }
        return *this;
    }

    T* get() const { return m_ptr; }
    T& operator*() const { return *m_ptr; }
    T* operator->() const { return m_ptr; }
    explicit operator bool() const { return m_ptr != nullptr; }

    void reset() {
        if (m_ptr) {
            MemoryPool::getInstance().destroy(m_ptr);
            m_ptr = nullptr;
        }
    }

private:
    T* m_ptr;
};

} // namespace RebelCAD
