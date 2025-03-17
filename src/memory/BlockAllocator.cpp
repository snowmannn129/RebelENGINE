#include "core/BlockAllocator.hpp"
#include <cassert>
#include <cstring>

namespace rebel_cad {
namespace core {

BlockAllocator::BlockAllocator(size_t blockSize, size_t initialBlocks)
    : m_blockSize(blockSize)
    , m_numBlocks(0)
    , m_numFreeBlocks(0)
    , m_firstFreeBlock(nullptr) {
    assert(blockSize >= sizeof(void*));  // Ensure block can hold at least a pointer
    addBlocks(initialBlocks);
}

BlockAllocator::~BlockAllocator() {
    for (auto& block : m_blocks) {
        delete[] block;
    }
}

void* BlockAllocator::allocate() {
    if (!m_firstFreeBlock) {
        addBlocks(m_blocks.empty() ? 8 : m_blocks.size());
    }

    void* block = m_firstFreeBlock;
    m_firstFreeBlock = *reinterpret_cast<void**>(block);
    m_numFreeBlocks--;

    return block;
}

void BlockAllocator::deallocate(void* ptr) {
    if (!ptr) return;

    // Add block to free list
    *reinterpret_cast<void**>(ptr) = m_firstFreeBlock;
    m_firstFreeBlock = ptr;
    m_numFreeBlocks++;
}

void BlockAllocator::addBlocks(size_t count) {
    // Allocate new block array
    char* newBlocks = new char[m_blockSize * count];
    m_blocks.push_back(newBlocks);

    // Initialize free list
    for (size_t i = 0; i < count - 1; i++) {
        void* block = newBlocks + (i * m_blockSize);
        void* nextBlock = newBlocks + ((i + 1) * m_blockSize);
        *reinterpret_cast<void**>(block) = nextBlock;
    }

    // Set last block's next pointer to current free list
    void* lastBlock = newBlocks + ((count - 1) * m_blockSize);
    *reinterpret_cast<void**>(lastBlock) = m_firstFreeBlock;

    // Update free list head
    m_firstFreeBlock = newBlocks;

    // Update counts
    m_numBlocks += count;
    m_numFreeBlocks += count;
}

size_t BlockAllocator::getBlockSize() const {
    return m_blockSize;
}

size_t BlockAllocator::getNumBlocks() const {
    return m_numBlocks;
}

size_t BlockAllocator::getNumFreeBlocks() const {
    return m_numFreeBlocks;
}

} // namespace core
} // namespace rebel_cad
