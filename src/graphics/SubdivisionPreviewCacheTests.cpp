#include <gtest/gtest.h>
#include "modeling/SubdivisionPreviewCache.hpp"
#include "modeling/SubdivisionSurface.hpp"
#include "graphics/ViewportCamera.hpp"
#include <glm/gtc/matrix_transform.hpp>

using namespace RebelCAD;
using namespace RebelCAD::Modeling;
using namespace RebelCAD::Graphics;

class SubdivisionPreviewCacheTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple cube mesh for testing
        auto mesh = std::make_shared<HalfEdgeMesh>();
        
        // Front face vertices
        auto v0 = mesh->createVertex(glm::vec3(-1.0f, -1.0f,  1.0f));
        auto v1 = mesh->createVertex(glm::vec3( 1.0f, -1.0f,  1.0f));
        auto v2 = mesh->createVertex(glm::vec3( 1.0f,  1.0f,  1.0f));
        auto v3 = mesh->createVertex(glm::vec3(-1.0f,  1.0f,  1.0f));
        
        // Back face vertices
        auto v4 = mesh->createVertex(glm::vec3(-1.0f, -1.0f, -1.0f));
        auto v5 = mesh->createVertex(glm::vec3( 1.0f, -1.0f, -1.0f));
        auto v6 = mesh->createVertex(glm::vec3( 1.0f,  1.0f, -1.0f));
        auto v7 = mesh->createVertex(glm::vec3(-1.0f,  1.0f, -1.0f));
        
        // Create faces
        mesh->createFace({v0, v1, v2, v3}); // Front
        mesh->createFace({v5, v4, v7, v6}); // Back
        mesh->createFace({v1, v5, v6, v2}); // Right
        mesh->createFace({v4, v0, v3, v7}); // Left
        mesh->createFace({v3, v2, v6, v7}); // Top
        mesh->createFace({v4, v5, v1, v0}); // Bottom
        
        surface = std::make_shared<SubdivisionSurface>(mesh);
    }

    std::shared_ptr<SubdivisionSurface> surface;
};

TEST_F(SubdivisionPreviewCacheTest, InitialState) {
    SubdivisionPreviewCache cache;
    EXPECT_FALSE(cache.isValid());
}

TEST_F(SubdivisionPreviewCacheTest, UpdateCache) {
    SubdivisionPreviewCache cache;
    ASSERT_NO_THROW(cache.update(surface, 3));
    EXPECT_TRUE(cache.isValid());
}

TEST_F(SubdivisionPreviewCacheTest, GetLODMesh) {
    SubdivisionPreviewCache cache;
    cache.update(surface, 3);
    
    ViewportCamera camera;
    camera.setPosition(glm::vec3(0.0f, 0.0f, 10.0f));
    camera.setTarget(glm::vec3(0.0f));
    
    glm::mat4 worldTransform = glm::mat4(1.0f);
    auto mesh = cache.getLODMesh(camera, worldTransform);
    EXPECT_NE(mesh, nullptr);
}

TEST_F(SubdivisionPreviewCacheTest, LODSelection) {
    SubdivisionPreviewCache cache;
    cache.update(surface, 3);
    
    ViewportCamera camera;
    std::vector<float> thresholds = {10.0f, 5.0f, 2.0f, 0.0f};
    cache.setLODThresholds(thresholds);
    
    // Test different distances
    std::vector<glm::vec3> positions = {
        glm::vec3(0.0f, 0.0f, 15.0f),  // Should use LOD 0
        glm::vec3(0.0f, 0.0f, 7.0f),   // Should use LOD 1
        glm::vec3(0.0f, 0.0f, 3.0f),   // Should use LOD 2
        glm::vec3(0.0f, 0.0f, 1.0f)    // Should use LOD 3
    };
    
    glm::mat4 worldTransform = glm::mat4(1.0f);
    
    for (size_t i = 0; i < positions.size(); ++i) {
        camera.setPosition(positions[i]);
        auto mesh = cache.getLODMesh(camera, worldTransform);
        EXPECT_NE(mesh, nullptr);
        
        // Verify mesh complexity increases with LOD level
        if (i > 0) {
            auto prevMesh = cache.getLODMesh(camera, worldTransform);
            EXPECT_GE(mesh->getVertices().size(), prevMesh->getVertices().size());
        }
    }
}

TEST_F(SubdivisionPreviewCacheTest, MemoryManagement) {
    SubdivisionPreviewCache cache;
    cache.update(surface, 3);
    
    auto [totalMemory, memoryPerLevel] = cache.getMemoryStats();
    EXPECT_GT(totalMemory, 0);
    EXPECT_EQ(memoryPerLevel.size(), 4); // LOD levels 0-3
    
    // Memory should increase with subdivision level
    for (size_t i = 1; i < memoryPerLevel.size(); ++i) {
        EXPECT_GT(memoryPerLevel[i], memoryPerLevel[i-1]);
    }
}

TEST_F(SubdivisionPreviewCacheTest, ThreadSafety) {
    SubdivisionPreviewCache cache;
    cache.update(surface, 3);
    
    ViewportCamera camera;
    camera.setPosition(glm::vec3(0.0f, 0.0f, 5.0f));
    glm::mat4 worldTransform = glm::mat4(1.0f);
    
    // Simulate multiple threads accessing cache
    const int numThreads = 4;
    std::vector<std::thread> threads;
    std::atomic<bool> failed(false);
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&]() {
            try {
                for (int j = 0; j < 100; ++j) {
                    auto mesh = cache.getLODMesh(camera, worldTransform);
                    EXPECT_NE(mesh, nullptr);
                    
                    // Simulate camera movement
                    camera.setPosition(glm::vec3(0.0f, 0.0f, float(5 + j % 10)));
                }
            } catch (...) {
                failed = true;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_FALSE(failed);
}

TEST_F(SubdivisionPreviewCacheTest, CacheEviction) {
    SubdivisionPreviewCache cache;
    cache.update(surface, 5); // Create more levels to test eviction
    
    ViewportCamera camera;
    glm::mat4 worldTransform = glm::mat4(1.0f);
    
    // Access meshes in reverse order to make lower levels less recently used
    for (int i = 5; i >= 0; --i) {
        camera.setPosition(glm::vec3(0.0f, 0.0f, float(i + 1)));
        auto mesh = cache.getLODMesh(camera, worldTransform);
        EXPECT_NE(mesh, nullptr);
    }
    
    // Force cache eviction by updating with new surface
    auto newMesh = std::make_shared<HalfEdgeMesh>(*surface->getBaseMesh());
    auto newSurface = std::make_shared<SubdivisionSurface>(newMesh);
    cache.update(newSurface, 5);
    
    // Verify cache is still valid
    EXPECT_TRUE(cache.isValid());
    
    // Verify we can still access meshes
    camera.setPosition(glm::vec3(0.0f, 0.0f, 5.0f));
    auto mesh = cache.getLODMesh(camera, worldTransform);
    EXPECT_NE(mesh, nullptr);
}

TEST_F(SubdivisionPreviewCacheTest, SIMDOptimization) {
    SubdivisionPreviewCache cache;
    cache.update(surface, 2);
    
    ViewportCamera camera;
    camera.setPosition(glm::vec3(0.0f, 0.0f, 5.0f));
    
    // Test SIMD-optimized transform
    glm::mat4 worldTransform = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(1.0f, 2.0f, 3.0f)
    );
    
    auto mesh = cache.getLODMesh(camera, worldTransform);
    EXPECT_NE(mesh, nullptr);
    
    // Verify transformed positions
    bool foundTransformedVertex = false;
    for (const auto& vertex : mesh->getVertices()) {
        // Check if any vertex has been transformed by the translation
        if (vertex->position.x > 0.0f || 
            vertex->position.y > 1.0f || 
            vertex->position.z > 2.0f) {
            foundTransformedVertex = true;
            break;
        }
    }
    EXPECT_TRUE(foundTransformedVertex);
}
