#include "modeling/SubdivisionPreviewCache.hpp"
#include <algorithm>
#include <execution>
#include <immintrin.h> // For AVX2 intrinsics

namespace RebelCAD {
namespace Modeling {

SubdivisionPreviewCache::SubdivisionPreviewCache()
    : isValid_(false) {
    // Initialize default LOD thresholds (in world units)
    lodThresholds_ = {
        10.0f,  // Level 0: Base mesh for distances > 10
        5.0f,   // Level 1: First subdivision for distances > 5
        2.0f,   // Level 2: Second subdivision for distances > 2
        0.0f    // Level 3: Maximum subdivision for close-up views
    };
}

SubdivisionPreviewCache::~SubdivisionPreviewCache() {
    clear();
}

void SubdivisionPreviewCache::update(
    std::shared_ptr<SubdivisionSurface> surface,
    int maxLevel) {
    if (!surface) {
        throw Error::InvalidArgument("Surface cannot be null");
    }

    std::lock_guard<std::mutex> lock(mutex_);
    
    // Clear existing cache
    clear();
    
    currentSurface_ = surface;
    
    // Pre-compute subdivision levels using parallel execution
    std::vector<int> levels(maxLevel + 1);
    std::iota(levels.begin(), levels.end(), 0);
    
    // Create temporary storage for results
    struct ComputeResult {
        int level;
        std::shared_ptr<HalfEdgeMesh> mesh;
        size_t memoryUsage;
    };
    std::vector<ComputeResult> results(maxLevel + 1);
    
    // Compute meshes in parallel
    std::for_each(std::execution::par, levels.begin(), levels.end(),
        [this, &results, surface](int level) {
            ComputeResult& result = results[level];
            result.level = level;
            
            if (level == 0) {
                result.mesh = surface->getBaseMesh();
            } else {
                result.mesh = surface->subdivide(level);
            }
            
            result.memoryUsage = computeMeshMemory(result.mesh);
        });
    
    // Store results in cache
    auto now = std::chrono::steady_clock::now();
    for (const auto& result : results) {
        CacheEntry entry;
        entry.mesh = result.mesh;
        entry.lastAccess = now;
        entry.memoryUsage = result.memoryUsage;
        meshCache_[result.level] = std::move(entry);
    }
    
    isValid_ = true;
}

std::shared_ptr<HalfEdgeMesh> SubdivisionPreviewCache::getLODMesh(
    const Graphics::ViewportCamera& camera,
    const glm::mat4& worldTransform) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!isValid_) {
        throw Error::InvalidOperation("Cache is not valid");
    }
    
    // Get object's center in world space
    glm::vec3 center = glm::vec3(worldTransform[3]);
    
    // Calculate distance to camera
    float distance = camera.distanceTo(center);
    
    // Compute appropriate LOD level
    int level = computeLODLevel(distance);
    
    // Get cached mesh for this level
    auto it = meshCache_.find(level);
    if (it == meshCache_.end()) {
        // Fall back to nearest available level
        auto nearest = std::min_element(meshCache_.begin(), meshCache_.end(),
            [level](const auto& a, const auto& b) {
                return std::abs(a.first - level) < std::abs(b.first - level);
            });
        it = nearest;
    }
    
    // Update last access time
    it->second.lastAccess = std::chrono::steady_clock::now();
    
    // Update transforms for efficient rendering
    updateMeshTransforms(it->second.mesh, worldTransform);
    
    return it->second.mesh;
}

void SubdivisionPreviewCache::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    meshCache_.clear();
    vertexPool_.clear();
    edgePool_.clear();
    facePool_.clear();
    currentSurface_.reset();
    isValid_ = false;
}

bool SubdivisionPreviewCache::isValid() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return isValid_;
}

void SubdivisionPreviewCache::setLODThresholds(
    const std::vector<float>& thresholds) {
    std::lock_guard<std::mutex> lock(mutex_);
    lodThresholds_ = thresholds;
}

std::pair<size_t, std::vector<size_t>> SubdivisionPreviewCache::getMemoryStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t totalMemory = 0;
    std::vector<size_t> memoryPerLevel;
    
    for (const auto& [level, entry] : meshCache_) {
        totalMemory += entry.memoryUsage;
        memoryPerLevel.push_back(entry.memoryUsage);
    }
    
    return {totalMemory, memoryPerLevel};
}

// Memory Pool Implementation
void* SubdivisionPreviewCache::MeshElementPool::allocate(size_t size) {
    // Round size up to multiple of 16 for SIMD alignment
    size = (size + 15) & ~15;
    
    // Try to find space in existing blocks
    for (auto& block : blocks_) {
        if (block.elementSize != size) continue;
        
        // Find first free slot
        for (size_t i = 0; i < block.used.size(); ++i) {
            if (!block.used[i]) {
                block.used[i] = true;
                totalAllocated_ += size;
                return block.data.data() + (i * size);
            }
        }
    }
    
    // Create new block
    PoolBlock newBlock;
    newBlock.elementSize = size;
    newBlock.blockSize = BLOCK_SIZE;
    size_t numElements = BLOCK_SIZE / size;
    newBlock.data.resize(numElements * size);
    newBlock.used.resize(numElements, false);
    newBlock.used[0] = true;
    blocks_.push_back(std::move(newBlock));
    totalAllocated_ += size;
    
    return blocks_.back().data.data();
}

void SubdivisionPreviewCache::MeshElementPool::deallocate(void* ptr) {
    // Find block containing this pointer
    for (auto& block : blocks_) {
        uint8_t* start = block.data.data();
        uint8_t* end = start + block.data.size();
        if (ptr >= start && ptr < end) {
            size_t index = ((uint8_t*)ptr - start) / block.elementSize;
            if (index < block.used.size()) {
                block.used[index] = false;
                totalAllocated_ -= block.elementSize;
            }
            return;
        }
    }
}

size_t SubdivisionPreviewCache::MeshElementPool::getAllocatedSize() const {
    return totalAllocated_;
}

void SubdivisionPreviewCache::MeshElementPool::clear() {
    blocks_.clear();
    totalAllocated_ = 0;
}

// Private Methods
int SubdivisionPreviewCache::computeLODLevel(float distance) const {
    // Find appropriate level based on distance thresholds
    for (size_t i = 0; i < lodThresholds_.size(); ++i) {
        if (distance > lodThresholds_[i]) {
            return static_cast<int>(i);
        }
    }
    return static_cast<int>(lodThresholds_.size() - 1);
}

void SubdivisionPreviewCache::evictCache(size_t requiredBytes) {
    // Sort entries by last access time
    std::vector<std::pair<int, CacheEntry*>> entries;
    for (auto& [level, entry] : meshCache_) {
        entries.emplace_back(level, &entry);
    }
    
    std::sort(entries.begin(), entries.end(),
        [](const auto& a, const auto& b) {
            return a.second->lastAccess < b.second->lastAccess;
        });
    
    // Remove entries until we have enough space
    size_t freedBytes = 0;
    for (const auto& [level, entry] : entries) {
        if (freedBytes >= requiredBytes) break;
        
        // Don't evict base mesh
        if (level == 0) continue;
        
        freedBytes += entry->memoryUsage;
        meshCache_.erase(level);
    }
}

void SubdivisionPreviewCache::validateCache() {
    auto now = std::chrono::steady_clock::now();
    
    // Remove expired entries
    for (auto it = meshCache_.begin(); it != meshCache_.end();) {
        if (now - it->second.lastAccess > CACHE_TIMEOUT && it->first != 0) {
            it = meshCache_.erase(it);
        } else {
            ++it;
        }
    }
}

size_t SubdivisionPreviewCache::computeMeshMemory(
    const std::shared_ptr<HalfEdgeMesh>& mesh) const {
    if (!mesh) return 0;
    
    // Calculate memory usage for vertices, edges, faces
    size_t vertexMem = mesh->getVertices().size() * sizeof(HalfEdgeMesh::Vertex);
    size_t edgeMem = mesh->getEdges().size() * sizeof(HalfEdgeMesh::Edge);
    size_t faceMem = mesh->getFaces().size() * sizeof(HalfEdgeMesh::Face);
    
    return vertexMem + edgeMem + faceMem;
}

void SubdivisionPreviewCache::updateMeshTransforms(
    const std::shared_ptr<HalfEdgeMesh>& mesh,
    const glm::mat4& worldTransform) const {
    if (!mesh) return;
    
    // Use SIMD to transform vertices in parallel
    const auto& vertices = mesh->getVertices();
    const size_t numVertices = vertices.size();
    
    // Process 8 vertices at a time using AVX2
    for (size_t i = 0; i < numVertices; i += 8) {
        size_t remaining = std::min(8ULL, numVertices - i);
        
        // Load vertex positions
        __m256 positions[3];
        for (size_t j = 0; j < remaining; ++j) {
            const auto& pos = vertices[i + j]->position;
            reinterpret_cast<float*>(&positions[0])[j] = pos.x;
            reinterpret_cast<float*>(&positions[1])[j] = pos.y;
            reinterpret_cast<float*>(&positions[2])[j] = pos.z;
        }
        
        // Transform positions
        __m256 transformed[3];
        for (int row = 0; row < 3; ++row) {
            transformed[row] = _mm256_mul_ps(positions[0], _mm256_set1_ps(worldTransform[row][0]));
            transformed[row] = _mm256_add_ps(transformed[row],
                _mm256_mul_ps(positions[1], _mm256_set1_ps(worldTransform[row][1])));
            transformed[row] = _mm256_add_ps(transformed[row],
                _mm256_mul_ps(positions[2], _mm256_set1_ps(worldTransform[row][2])));
            transformed[row] = _mm256_add_ps(transformed[row], _mm256_set1_ps(worldTransform[row][3]));
        }
        
        // Store transformed positions
        for (size_t j = 0; j < remaining; ++j) {
            vertices[i + j]->position = glm::vec3(
                reinterpret_cast<float*>(&transformed[0])[j],
                reinterpret_cast<float*>(&transformed[1])[j],
                reinterpret_cast<float*>(&transformed[2])[j]
            );
        }
    }
    
    // Update face and edge transforms
    mesh->computeFaceNormals();
}

} // namespace Modeling
} // namespace RebelCAD
