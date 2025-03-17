#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "core/SmartPtr.h"
#include "core/WeakPtr.h"
#include "core/CircularReferenceDetector.h"
#include "core/SmartResourceManager.h"

using namespace RebelCAD::Core;

// Test class with reference counting
class TestObject {
public:
    TestObject(int value = 0) : m_value(value) { ++s_instanceCount; }
    ~TestObject() { --s_instanceCount; }
    
    int getValue() const { return m_value; }
    void setValue(int value) { m_value = value; }
    
    static int getInstanceCount() { return s_instanceCount; }
    
private:
    int m_value;

public:
    static int s_instanceCount;
};

int TestObject::s_instanceCount = 0;

// Test fixture
class SmartPtrTest : public ::testing::Test {
protected:
    void SetUp() override {
        TestObject::s_instanceCount = 0;
    }
};

// Memory layout and alignment tests
TEST_F(SmartPtrTest, MemoryLayoutAndAlignment) {
    SmartPtr<TestObject> ptr = SmartPtr<TestObject>::make(42);
    
    // Verify control block alignment
    auto* ctrl = reinterpret_cast<const void*>(ptr.get());
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(ctrl) % 64, 0) 
        << "Control block should be 64-byte aligned";
    
    // Test basic functionality
    EXPECT_EQ(ptr->getValue(), 42);
    EXPECT_EQ(TestObject::getInstanceCount(), 1);
    
    // Verify single allocation
    EXPECT_EQ(ptr.use_count(), 1);
}

TEST_F(SmartPtrTest, CopyConstruction) {
    SmartPtr<TestObject> ptr1 = SmartPtr<TestObject>::make(42);
    {
        SmartPtr<TestObject> ptr2 = ptr1;
        EXPECT_EQ(ptr1.use_count(), 2);
        EXPECT_EQ(ptr2.use_count(), 2);
        EXPECT_EQ(TestObject::getInstanceCount(), 1);
    }
    EXPECT_EQ(ptr1.use_count(), 1);
    EXPECT_EQ(TestObject::getInstanceCount(), 1);
}

TEST_F(SmartPtrTest, MoveConstruction) {
    SmartPtr<TestObject> ptr1 = SmartPtr<TestObject>::make(42);
    SmartPtr<TestObject> ptr2 = std::move(ptr1);
    
    EXPECT_EQ(ptr2->getValue(), 42);
    EXPECT_EQ(ptr2.use_count(), 1);
    EXPECT_EQ(ptr1.get(), nullptr);
}

// Weak pointer tests
TEST_F(SmartPtrTest, WeakPointerBasics) {
    WeakPtr<TestObject> weak;
    {
        SmartPtr<TestObject> strong = SmartPtr<TestObject>::make(42);
        weak = strong;
        EXPECT_FALSE(weak.expired());
        
        auto locked = weak.lock();
        ASSERT_TRUE(locked);
        EXPECT_EQ(locked->getValue(), 42);
    }
    EXPECT_TRUE(weak.expired());
    EXPECT_FALSE(weak.lock());
}

// Enhanced circular reference tests
class Node {
public:
    Node(int value) : m_value(value) {}
    void setNext(SmartPtr<Node> next) { m_next = next; }
    SmartPtr<Node> getNext() const { return m_next; }
    WeakPtr<Node> getWeakNext() const { return m_next; }
    
private:
    int m_value;
    SmartPtr<Node> m_next;
};

TEST_F(SmartPtrTest, CircularReferenceDetection) {
    auto node1 = SmartPtr<Node>::make(1);
    auto node2 = SmartPtr<Node>::make(2);
    
    // Create circular reference
    node1->setNext(node2);
    node2->setNext(node1);
    
    // Verify automatic cycle detection
    auto& detector = CircularReferenceDetector::getInstance();
    EXPECT_TRUE(detector.hasCircularReferences(node1.get()));
    EXPECT_TRUE(detector.hasCircularReferences(node2.get()));
    
    // Verify cycle is broken when one node is reset
    node1->setNext(SmartPtr<Node>(nullptr));
    EXPECT_FALSE(detector.hasCircularReferences(node1.get()));
    EXPECT_FALSE(detector.hasCircularReferences(node2.get()));
}

TEST_F(SmartPtrTest, CircularReferenceMemoryLeak) {
    int initialCount = TestObject::getInstanceCount();
    
    {
        auto obj1 = SmartPtr<TestObject>::make(1);
        auto obj2 = SmartPtr<TestObject>::make(2);
        
        // Create circular reference through resource manager
        SmartResourceManager::getInstance().registerResourceRelationship(
            obj1.get(), obj2.get());
        SmartResourceManager::getInstance().registerResourceRelationship(
            obj2.get(), obj1.get());
    }
    
    // Verify no memory leaks despite circular references
    EXPECT_EQ(TestObject::getInstanceCount(), initialCount);
}

// Thread safety tests
TEST_F(SmartPtrTest, ThreadSafety) {
    const int numThreads = 10;
    const int numOperations = 1000;
    
    SmartPtr<TestObject> shared = SmartPtr<TestObject>::make(0);
    std::vector<std::thread> threads;
    
    // Create multiple threads that read and write to the shared object
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([shared, numOperations]() {
            for (int j = 0; j < numOperations; ++j) {
                SmartPtr<TestObject> local = shared;
                local->setValue(local->getValue() + 1);
            }
        });
    }
    
    // Join all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify the final value
    EXPECT_EQ(shared->getValue(), numThreads * numOperations);
}

// Enhanced memory pool integration tests
TEST_F(SmartPtrTest, MemoryPoolIntegrationAndCachePerformance) {
    const int numAllocations = 1000;
    const int iterationsPerObject = 100;
    std::vector<SmartPtr<TestObject>> objects;
    
    // Measure allocation time
    auto startAlloc = std::chrono::high_resolution_clock::now();
    
    // Allocate many objects
    for (int i = 0; i < numAllocations; ++i) {
        objects.push_back(SmartPtr<TestObject>::make(i));
    }
    
    auto endAlloc = std::chrono::high_resolution_clock::now();
    auto allocTime = std::chrono::duration_cast<std::chrono::microseconds>(
        endAlloc - startAlloc).count();
    
    // Verify all objects are created
    EXPECT_EQ(TestObject::getInstanceCount(), numAllocations);
    
    // Test cache-friendly access pattern
    auto startAccess = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterationsPerObject; ++i) {
        for (auto& obj : objects) {
            obj->setValue(obj->getValue() + 1);
        }
    }
    
    auto endAccess = std::chrono::high_resolution_clock::now();
    auto accessTime = std::chrono::duration_cast<std::chrono::microseconds>(
        endAccess - startAccess).count();
    
    // Clear objects and verify cleanup
    objects.clear();
    EXPECT_EQ(TestObject::getInstanceCount(), 0);
    
    // Log performance metrics
    std::cout << "Allocation time per object: " 
              << (allocTime / numAllocations) << "us\n";
    std::cout << "Access time per operation: " 
              << (accessTime / (numAllocations * iterationsPerObject)) << "us\n";
}

// Null pointer and reset tests
TEST_F(SmartPtrTest, NullPointerAndReset) {
    // Test nullptr constructor
    SmartPtr<TestObject> ptr1(nullptr);
    EXPECT_FALSE(ptr1);
    EXPECT_EQ(ptr1.get(), nullptr);
    EXPECT_EQ(ptr1.use_count(), 0);

    // Test reset functionality
    SmartPtr<TestObject> ptr2 = SmartPtr<TestObject>::make(42);
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr2->getValue(), 42);
    EXPECT_EQ(TestObject::getInstanceCount(), 1);

    ptr2.reset();
    EXPECT_FALSE(ptr2);
    EXPECT_EQ(ptr2.get(), nullptr);
    EXPECT_EQ(ptr2.use_count(), 0);
    EXPECT_EQ(TestObject::getInstanceCount(), 0);

    // Test nullptr assignment
    ptr2 = SmartPtr<TestObject>::make(24);
    EXPECT_TRUE(ptr2);
    ptr2 = nullptr;
    EXPECT_FALSE(ptr2);
    EXPECT_EQ(TestObject::getInstanceCount(), 0);
}

// Test nullptr comparison operators
TEST_F(SmartPtrTest, NullptrComparison) {
    SmartPtr<TestObject> null_ptr;
    SmartPtr<TestObject> valid_ptr = SmartPtr<TestObject>::make(42);

    EXPECT_TRUE(null_ptr == nullptr);
    EXPECT_TRUE(nullptr == null_ptr);
    EXPECT_FALSE(valid_ptr == nullptr);
    EXPECT_FALSE(nullptr == valid_ptr);

    EXPECT_FALSE(null_ptr != nullptr);
    EXPECT_FALSE(nullptr != null_ptr);
    EXPECT_TRUE(valid_ptr != nullptr);
    EXPECT_TRUE(nullptr != valid_ptr);
}

// Resource manager integration tests
TEST_F(SmartPtrTest, ResourceManagerIntegration) {
    auto& manager = SmartResourceManager::getInstance();
    
    {
        auto resource = manager.createResource<TestObject>(42);
        EXPECT_EQ(manager.getResourceCount<TestObject>(), 1);
        EXPECT_EQ(resource->getValue(), 42);
    }
    
    EXPECT_EQ(manager.getResourceCount<TestObject>(), 0);
}

// Custom cleanup handler tests
TEST_F(SmartPtrTest, CustomCleanupHandler) {
    auto& manager = SmartResourceManager::getInstance();
    bool cleanupCalled = false;
    
    manager.registerCleanupHandler<TestObject>([&cleanupCalled](TestObject* obj) {
        cleanupCalled = true;
    });
    
    {
        auto resource = manager.createResource<TestObject>(42);
    }
    
    EXPECT_TRUE(cleanupCalled);
}
