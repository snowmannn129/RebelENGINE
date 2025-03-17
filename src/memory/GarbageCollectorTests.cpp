#include <gtest/gtest.h>
#include "core/GarbageCollector.h"
#include <thread>
#include <chrono>

using namespace RebelCAD::Core;

class TestObject {
public:
    static int instanceCount;
    TestObject() { instanceCount++; }
    ~TestObject() { instanceCount--; }
};

int TestObject::instanceCount = 0;

class GarbageCollectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        TestObject::instanceCount = 0;
        GarbageCollector::getInstance().stop();
        GarbageCollector::getInstance().start();
    }

    void TearDown() override {
        GarbageCollector::getInstance().stop();
    }
};

TEST_F(GarbageCollectorTest, BasicAllocationAndCollection) {
    // Create an object and verify it's tracked
    auto* obj = REBEL_GC_NEW(TestObject);
    EXPECT_EQ(TestObject::instanceCount, 1);

    // Trigger collection
    GarbageCollector::getInstance().collect(true);
    
    // Object should still exist due to reference count
    EXPECT_EQ(TestObject::instanceCount, 1);

    // Remove reference and collect
    REBEL_GC_UNREF(obj);
    GarbageCollector::getInstance().collect(true);
    
    // Object should be collected
    EXPECT_EQ(TestObject::instanceCount, 0);
}

TEST_F(GarbageCollectorTest, CircularReferences) {
    class Node {
    public:
        Node* next = nullptr;
        ~Node() { REBEL_GC_UNREF(next); }
    };

    // Create circular reference
    auto* node1 = REBEL_GC_NEW(Node);
    auto* node2 = REBEL_GC_NEW(Node);
    
    node1->next = REBEL_GC_REF(node2);
    node2->next = REBEL_GC_REF(node1);

    // Remove external references
    REBEL_GC_UNREF(node1);
    REBEL_GC_UNREF(node2);

    // Trigger collection
    GarbageCollector::getInstance().collect(true);
    
    // Check memory stats
    auto [totalMem, objCount] = GarbageCollector::getInstance().getStats();
    EXPECT_EQ(objCount, 0) << "Circular reference not collected";
}

TEST_F(GarbageCollectorTest, IncrementalCollection) {
    const int NUM_OBJECTS = 1000;
    std::vector<TestObject*> objects;

    // Create many objects
    for (int i = 0; i < NUM_OBJECTS; i++) {
        objects.push_back(REBEL_GC_NEW(TestObject));
    }
    EXPECT_EQ(TestObject::instanceCount, NUM_OBJECTS);

    // Configure for incremental collection
    GCConfig config;
    config.isIncremental = true;
    config.maxPauseTime = 1; // 1ms max pause
    GarbageCollector::getInstance().configure(config);

    // Remove references
    for (auto* obj : objects) {
        REBEL_GC_UNREF(obj);
    }
    objects.clear();

    // Wait for incremental collection to complete
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    EXPECT_EQ(TestObject::instanceCount, 0) 
        << "Not all objects collected incrementally";
}

TEST_F(GarbageCollectorTest, MemoryThreshold) {
    GCConfig config;
    config.memoryThreshold = 1024; // 1KB threshold
    GarbageCollector::getInstance().configure(config);

    // Create objects until threshold is exceeded
    std::vector<TestObject*> objects;
    for (int i = 0; i < 1000; i++) {
        objects.push_back(REBEL_GC_NEW(TestObject));
    }

    // Wait for automatic collection
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    auto [totalMem, objCount] = GarbageCollector::getInstance().getStats();
    EXPECT_LT(totalMem, config.memoryThreshold) 
        << "Memory threshold not enforced";

    // Cleanup
    for (auto* obj : objects) {
        REBEL_GC_UNREF(obj);
    }
}

TEST_F(GarbageCollectorTest, ThreadSafety) {
    const int NUM_THREADS = 4;
    const int OBJECTS_PER_THREAD = 1000;
    std::vector<std::thread> threads;

    auto threadFunc = []() {
        std::vector<TestObject*> objects;
        for (int i = 0; i < OBJECTS_PER_THREAD; i++) {
            auto* obj = REBEL_GC_NEW(TestObject);
            objects.push_back(obj);
            if (i % 2 == 0) {
                REBEL_GC_UNREF(obj);
            }
        }
    };

    // Start threads
    for (int i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back(threadFunc);
    }

    // Wait for threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Trigger final collection
    GarbageCollector::getInstance().collect(true);
    
    // Verify no memory leaks
    auto [totalMem, objCount] = GarbageCollector::getInstance().getStats();
    EXPECT_EQ(objCount, OBJECTS_PER_THREAD * NUM_THREADS / 2)
        << "Unexpected number of surviving objects";
}
