#pragma once

#include "modeling/PreviewCache.hpp"
#include "modeling/HalfEdgeMesh.hpp"
#include "modeling/SubdivisionSurface.hpp"
#include "graphics/ViewportCamera.hpp"
#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <glm/glm.hpp>

namespace RebelCAD {
namespace Modeling {

/**
 * @brief Specialized cache for subdivision surface previews with LOD support
 * 
 * This class provides efficient caching of subdivision levels with:
 * - Multi-level caching for different subdivision depths
 * - View-dependent LOD selection
 * - Memory pooling for mesh elements
 * - SIMD-optimized vertex computations
 */
class SubdivisionPreviewCache {
public:
    SubdivisionPreviewCache();
    ~SubdivisionPreviewCache();

    /**
     * @brief Updates the cache with a new subdivision surface
     * @param surface The subdivision surface to cache
     * @param maxLevel Maximum subdivision level to pre-compute
     */
    void update(std::shared_ptr<SubdivisionSurface> surface, int maxLevel = 3);

    /**
     * @brief Gets the appropriate LOD mesh based on view parameters
     * @param camera The viewport camera for distance calculations
     * @param worldTransform The world transform of the object
     * @return Mesh at appropriate subdivision level
     */
    std::shared_ptr<HalfEdgeMesh> getLODMesh(
        const Graphics::ViewportCamera& camera,
        const glm::mat4& worldTransform) const;

    /**
     * @brief Clears all cached data
     */
    void clear();

    /**
     * @brief Checks if cache contains valid data
     */
    bool isValid() const;

    /**
     * @brief Sets the LOD distance thresholds
     * @param thresholds Vector of distances for each LOD level
     */
    void setLODThresholds(const std::vector<float>& thresholds);

    /**
     * @brief Gets memory usage statistics
     * @return Pair of (total bytes, bytes per level)
     */
    std::pair<size_t, std::vector<size_t>> getMemoryStats() const;

private:
    // Cache data structure
    struct CacheEntry {
        std::shared_ptr<HalfEdgeMesh> mesh;
        std::chrono::steady_clock::time_point lastAccess;
        size_t memoryUsage;

        CacheEntry() = default;
        CacheEntry(const CacheEntry&) = default;
        CacheEntry(CacheEntry&&) = default;
        CacheEntry& operator=(const CacheEntry&) = default;
        CacheEntry& operator=(CacheEntry&&) = default;
    };

    // Memory pool for mesh elements
    class MeshElementPool {
    public:
        void* allocate(size_t size);
        void deallocate(void* ptr);
        size_t getAllocatedSize() const;
        void clear();

    private:
        struct PoolBlock {
            std::vector<uint8_t> data;
            std::vector<bool> used;
            size_t blockSize;
            size_t elementSize;
        };
        std::vector<PoolBlock> blocks_;
        size_t totalAllocated_ = 0;
        static constexpr size_t BLOCK_SIZE = 1024 * 1024; // 1MB blocks
    };

    // Cache data
    std::unordered_map<int, CacheEntry> meshCache_;
    std::vector<float> lodThresholds_;
    std::shared_ptr<SubdivisionSurface> currentSurface_;
    bool isValid_ = false;
    
    // Memory management
    MeshElementPool vertexPool_;
    MeshElementPool edgePool_;
    MeshElementPool facePool_;
    
    // Thread safety
    mutable std::mutex mutex_;

    // Cache settings
    static constexpr auto CACHE_TIMEOUT = std::chrono::minutes(5);
    static constexpr size_t MAX_CACHE_MEMORY = 1024 * 1024 * 1024; // 1GB

    /**
     * @brief Computes appropriate LOD level based on view distance
     * @param distance View distance to object
     * @return Optimal subdivision level
     */
    int computeLODLevel(float distance) const;

    /**
     * @brief Removes least recently used cache entries to free memory
     * @param requiredBytes Number of bytes to free
     */
    void evictCache(size_t requiredBytes);

    /**
     * @brief Validates cache entries and removes expired ones
     */
    void validateCache();

    /**
     * @brief Computes memory usage for a mesh
     * @param mesh The mesh to analyze
     * @return Memory usage in bytes
     */
    size_t computeMeshMemory(const std::shared_ptr<HalfEdgeMesh>& mesh) const;

    /**
     * @brief Updates mesh transforms for efficient rendering
     * @param mesh The mesh to update
     * @param worldTransform The world transform to apply
     */
    void updateMeshTransforms(
        const std::shared_ptr<HalfEdgeMesh>& mesh,
        const glm::mat4& worldTransform) const;
};

} // namespace Modeling
} // namespace RebelCAD
