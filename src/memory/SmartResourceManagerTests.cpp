#include <gtest/gtest.h>
#include "core/SmartResourceManager.h"

using namespace RebelCAD::Core;

// Test resource class
class TestResource {
public:
    TestResource(int value = 0) : m_value(value) {}
    int getValue() const { return m_value; }
private:
    int m_value;
};

class SmartResourceManagerTest : public ::testing::Test {
protected:
    SmartResourceManager& manager = SmartResourceManager::getInstance();
};

TEST_F(SmartResourceManagerTest, CreateResource) {
    auto resource = manager.createResource<TestResource>(42);
    ASSERT_NE(resource, nullptr);
    EXPECT_EQ(resource->getValue(), 42);
    EXPECT_EQ(manager.getResourceCount<TestResource>(), 1);
}

TEST_F(SmartResourceManagerTest, ReferenceCountingAndCleanup) {
    {
        auto resource1 = manager.createResource<TestResource>(1);
        EXPECT_EQ(manager.getResourceCount<TestResource>(), 1);
        
        {
            auto resource2 = resource1; // Create second reference
            EXPECT_EQ(manager.getResourceCount<TestResource>(), 1);
        } // resource2 goes out of scope, but object should still exist
        
        EXPECT_EQ(manager.getResourceCount<TestResource>(), 1);
    } // resource1 goes out of scope, object should be destroyed
    
    EXPECT_EQ(manager.getResourceCount<TestResource>(), 0);
}

TEST_F(SmartResourceManagerTest, CustomCleanupHandler) {
    bool cleanupCalled = false;
    
    manager.registerCleanupHandler<TestResource>(
        [&cleanupCalled](TestResource* resource) {
            cleanupCalled = true;
        }
    );

    {
        auto resource = manager.createResource<TestResource>();
    } // Resource goes out of scope here

    EXPECT_TRUE(cleanupCalled);
}

TEST_F(SmartResourceManagerTest, MultipleResourceTypes) {
    struct AnotherResource {
        AnotherResource(double x) : value(x) {}
        double value;
    };

    auto resource1 = manager.createResource<TestResource>(1);
    auto resource2 = manager.createResource<AnotherResource>(3.14);

    EXPECT_EQ(manager.getResourceCount<TestResource>(), 1);
    EXPECT_EQ(manager.getResourceCount<AnotherResource>(), 1);

    resource1.reset();
    EXPECT_EQ(manager.getResourceCount<TestResource>(), 0);
    EXPECT_EQ(manager.getResourceCount<AnotherResource>(), 1);
}

TEST_F(SmartResourceManagerTest, MemoryPoolIntegration) {
    // Create multiple resources to test memory pool allocation
    std::vector<std::shared_ptr<TestResource>> resources;
    const size_t resourceCount = 100;

    for (size_t i = 0; i < resourceCount; ++i) {
        resources.push_back(manager.createResource<TestResource>(i));
        EXPECT_EQ(manager.getResourceCount<TestResource>(), i + 1);
    }

    // Verify all resources are valid
    for (size_t i = 0; i < resourceCount; ++i) {
        EXPECT_EQ(resources[i]->getValue(), i);
    }

    // Clear resources and verify cleanup
    resources.clear();
    EXPECT_EQ(manager.getResourceCount<TestResource>(), 0);
}
