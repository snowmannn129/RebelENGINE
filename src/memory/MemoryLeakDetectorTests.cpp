#include "core/MemoryLeakDetector.h"
#include <gtest/gtest.h>
#include <fstream>
#include <string>

namespace {
// Helper functions for memory allocation in tests
template<typename T>
class TrackedAlloc {
public:
    static T* newObject(T value, const char* file, int line) {
        T* ptr = static_cast<T*>(std::malloc(sizeof(T)));
        new (ptr) T(value); // placement new
        RebelCAD::Core::MemoryLeakDetector::getInstance().trackAllocation(ptr, sizeof(T), file, line);
        return ptr;
    }

    static T* newArray(size_t size, const char* file, int line) {
        T* ptr = static_cast<T*>(std::malloc(sizeof(T) * size));
        for (size_t i = 0; i < size; ++i) {
            new (ptr + i) T(); // placement new for each element
        }
        RebelCAD::Core::MemoryLeakDetector::getInstance().trackAllocation(ptr, sizeof(T) * size, file, line);
        return ptr;
    }

    static void deleteObject(T* ptr) {
        if (ptr) {
            ptr->~T(); // explicit destructor call
            RebelCAD::Core::MemoryLeakDetector::getInstance().trackDeallocation(ptr);
            std::free(ptr);
        }
    }

    static void deleteArray(T* ptr, size_t size) {
        if (ptr) {
            for (size_t i = 0; i < size; ++i) {
                (ptr + i)->~T(); // explicit destructor call for each element
            }
            RebelCAD::Core::MemoryLeakDetector::getInstance().trackDeallocation(ptr);
            std::free(ptr);
        }
    }
};
}

namespace RebelCAD {
namespace Core {
namespace Tests {

class MemoryLeakDetectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        MemoryLeakDetector::getInstance().reset();
        MemoryLeakDetector::getInstance().enableTracking(true);
    }

    void TearDown() override {
        MemoryLeakDetector::getInstance().reset();
    }
};

TEST_F(MemoryLeakDetectorTest, TrackAllocation) {
    int* ptr = TrackedAlloc<int>::newObject(42, "MemoryLeakDetectorTests.cpp", 1);
    EXPECT_EQ(MemoryLeakDetector::getInstance().getActiveAllocations(), 1);
    EXPECT_GT(MemoryLeakDetector::getInstance().getCurrentMemoryUsage(), 0);
    TrackedAlloc<int>::deleteObject(ptr);
}

TEST_F(MemoryLeakDetectorTest, TrackDeallocation) {
    int* ptr = TrackedAlloc<int>::newObject(42, "MemoryLeakDetectorTests.cpp", 2);
    TrackedAlloc<int>::deleteObject(ptr);
    EXPECT_EQ(MemoryLeakDetector::getInstance().getActiveAllocations(), 0);
    EXPECT_EQ(MemoryLeakDetector::getInstance().getCurrentMemoryUsage(), 0);
}

TEST_F(MemoryLeakDetectorTest, MultipleAllocations) {
    int* ptr1 = TrackedAlloc<int>::newObject(42, "MemoryLeakDetectorTests.cpp", 3);
    int* ptr2 = TrackedAlloc<int>::newObject(43, "MemoryLeakDetectorTests.cpp", 4);
    EXPECT_EQ(MemoryLeakDetector::getInstance().getActiveAllocations(), 2);
    TrackedAlloc<int>::deleteObject(ptr1);
    EXPECT_EQ(MemoryLeakDetector::getInstance().getActiveAllocations(), 1);
    TrackedAlloc<int>::deleteObject(ptr2);
    EXPECT_EQ(MemoryLeakDetector::getInstance().getActiveAllocations(), 0);
}

TEST_F(MemoryLeakDetectorTest, ArrayAllocation) {
    const size_t size = 100;
    int* arr = TrackedAlloc<int>::newArray(size, "MemoryLeakDetectorTests.cpp", 5);
    EXPECT_EQ(MemoryLeakDetector::getInstance().getActiveAllocations(), 1);
    EXPECT_GE(MemoryLeakDetector::getInstance().getCurrentMemoryUsage(), sizeof(int) * size);
    TrackedAlloc<int>::deleteArray(arr, size);
    EXPECT_EQ(MemoryLeakDetector::getInstance().getActiveAllocations(), 0);
}

TEST_F(MemoryLeakDetectorTest, LeakDetection) {
    int* ptr = TrackedAlloc<int>::newObject(42, "MemoryLeakDetectorTests.cpp", 6);
    EXPECT_TRUE(MemoryLeakDetector::getInstance().hasLeaks());
    // Intentionally not deleting to test leak detection
}

TEST_F(MemoryLeakDetectorTest, DumpLeaks) {
    const char* filename = "test_leaks.txt";
    int* ptr = TrackedAlloc<int>::newObject(42, "MemoryLeakDetectorTests.cpp", 7);
    
    MemoryLeakDetector::getInstance().dumpLeaks(filename);
    
    std::ifstream file(filename);
    ASSERT_TRUE(file.is_open());
    
    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    
    EXPECT_TRUE(content.find("Memory Leak Report") != std::string::npos);
    EXPECT_TRUE(content.find("MemoryLeakDetectorTests.cpp") != std::string::npos);
    
    file.close();
    std::remove(filename);
    TrackedAlloc<int>::deleteObject(ptr);
}

TEST_F(MemoryLeakDetectorTest, DisableTracking) {
    MemoryLeakDetector::getInstance().enableTracking(false);
    int* ptr = TrackedAlloc<int>::newObject(42, "MemoryLeakDetectorTests.cpp", 8);
    EXPECT_EQ(MemoryLeakDetector::getInstance().getActiveAllocations(), 0);
    TrackedAlloc<int>::deleteObject(ptr);
}

TEST_F(MemoryLeakDetectorTest, Reset) {
    int* ptr = TrackedAlloc<int>::newObject(42, "MemoryLeakDetectorTests.cpp", 9);
    MemoryLeakDetector::getInstance().reset();
    EXPECT_EQ(MemoryLeakDetector::getInstance().getActiveAllocations(), 0);
    EXPECT_EQ(MemoryLeakDetector::getInstance().getCurrentMemoryUsage(), 0);
    TrackedAlloc<int>::deleteObject(ptr);
}

} // namespace Tests
} // namespace Core
} // namespace RebelCAD
