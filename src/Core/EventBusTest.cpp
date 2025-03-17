#include "Core/EventBus.h"
#include "Core/Error.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <atomic>
#include <chrono>

using namespace RebelCAD::Core;

// Test events
struct TestEvent {
    int value;
};

struct AnotherTestEvent {
    std::string message;
};

class EventBusTest : public ::testing::Test {
protected:
    void SetUp() override {
        eventBus = &EventBus::getInstance();
    }

    EventBus* eventBus;
};

TEST_F(EventBusTest, SubscribeAndPublish) {
    bool eventReceived = false;
    int receivedValue = 0;
    EventMetadata receivedMetadata;
    
    auto subscription = eventBus->subscribe<TestEvent>(
        [&](const TestEvent& event, const EventMetadata& metadata) {
            eventReceived = true;
            receivedValue = event.value;
            receivedMetadata = metadata;
        }
    );

    TestEvent event{42};
    eventBus->publish(event);

    EXPECT_TRUE(eventReceived);
    EXPECT_EQ(receivedValue, 42);
    EXPECT_EQ(receivedMetadata.priority, EventPriority::Normal);
    EXPECT_EQ(receivedMetadata.eventTypeName, typeid(TestEvent).name());
}

TEST_F(EventBusTest, UnsubscribeStopsEvents) {
    bool eventReceived = false;
    
    auto subscription = eventBus->subscribe<TestEvent>(
        [&](const TestEvent& event, const EventMetadata&) {
            eventReceived = true;
        }
    );

    eventBus->unsubscribe(subscription);

    TestEvent event{42};
    eventBus->publish(event);

    EXPECT_FALSE(eventReceived);
}

TEST_F(EventBusTest, PriorityBasedDispatch) {
    std::vector<std::string> callOrder;
    
    // Subscribe with different priorities
    eventBus->subscribe<TestEvent>(
        [&](const TestEvent&, const EventMetadata&) {
            callOrder.push_back("High");
        },
        EventPriority::High
    );

    eventBus->subscribe<TestEvent>(
        [&](const TestEvent&, const EventMetadata&) {
            callOrder.push_back("Normal");
        },
        EventPriority::Normal
    );

    eventBus->subscribe<TestEvent>(
        [&](const TestEvent&, const EventMetadata&) {
            callOrder.push_back("Low");
        },
        EventPriority::Low
    );

    TestEvent event{42};
    eventBus->publish(event, EventPriority::Normal);

    // Should only process High and Normal priority
    ASSERT_EQ(callOrder.size(), 2);
    EXPECT_EQ(callOrder[0], "High");
    EXPECT_EQ(callOrder[1], "Normal");
}

TEST_F(EventBusTest, TypeSafeEventDispatch) {
    bool testEventReceived = false;
    bool anotherEventReceived = false;

    eventBus->subscribe<TestEvent>(
        [&](const TestEvent&, const EventMetadata&) {
            testEventReceived = true;
        }
    );

    eventBus->subscribe<AnotherTestEvent>(
        [&](const AnotherTestEvent&, const EventMetadata&) {
            anotherEventReceived = true;
        }
    );

    TestEvent testEvent{42};
    eventBus->publish(testEvent);

    EXPECT_TRUE(testEventReceived);
    EXPECT_FALSE(anotherEventReceived);
}

TEST_F(EventBusTest, ThreadSafety) {
    std::atomic<int> counter{0};
    constexpr int NUM_THREADS = 10;
    constexpr int EVENTS_PER_THREAD = 100;

    // Create subscription
    auto subscription = eventBus->subscribe<TestEvent>(
        [&](const TestEvent&, const EventMetadata&) {
            counter++;
        }
    );

    // Spawn multiple threads to publish events
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.push_back(std::thread([&]() {
            for (int j = 0; j < EVENTS_PER_THREAD; ++j) {
                TestEvent event{j};
                eventBus->publish(event);
            }
        }));
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(counter, NUM_THREADS * EVENTS_PER_THREAD);
}

TEST_F(EventBusTest, ExceptionHandling) {
    // Subscribe with a handler that throws
    auto subscription = eventBus->subscribe<TestEvent>(
        [](const TestEvent&, const EventMetadata&) {
            throw std::runtime_error("Test exception");
        }
    );

    TestEvent event{42};
    // Should not throw, but log error and continue
    EXPECT_NO_THROW(eventBus->publish(event));
}

TEST_F(EventBusTest, UnsubscribeNonExistent) {
    // Should log warning but not throw
    EXPECT_NO_THROW(eventBus->unsubscribe(999999));
}

TEST_F(EventBusTest, PerformanceMetricsTracking) {
    // Subscribe and publish some events
    eventBus->subscribe<TestEvent>(
        [](const TestEvent&, const EventMetadata&) {
            // Simulate some work
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    );

    TestEvent event{42};
    for (int i = 0; i < 5; ++i) {
        eventBus->publish(event);
    }

    auto metrics = eventBus->getPerformanceMetrics();
    EXPECT_FALSE(metrics.empty());
    EXPECT_GT(metrics[typeid(TestEvent).name()], 0.0);
}

TEST_F(EventBusTest, EventMetadataAccuracy) {
    EventMetadata capturedMetadata;
    auto before = std::chrono::system_clock::now();
    
    eventBus->subscribe<TestEvent>(
        [&](const TestEvent&, const EventMetadata& metadata) {
            capturedMetadata = metadata;
        }
    );

    TestEvent event{42};
    eventBus->publish(event, EventPriority::High);
    
    auto after = std::chrono::system_clock::now();

    EXPECT_EQ(capturedMetadata.priority, EventPriority::High);
    EXPECT_EQ(capturedMetadata.eventTypeName, typeid(TestEvent).name());
    EXPECT_GE(capturedMetadata.timestamp, before);
    EXPECT_LE(capturedMetadata.timestamp, after);
}

// Custom event filter for testing
class TestEventFilter : public EventFilter<TestEvent> {
public:
    explicit TestEventFilter(int threshold) : threshold_(threshold) {}
    
    bool shouldProcess(const TestEvent& event, const EventMetadata&) const override {
        return event.value > threshold_;
    }

private:
    int threshold_;
};

TEST_F(EventBusTest, EventFilteringSystem) {
    bool eventReceived = false;
    auto filter = std::make_shared<TestEventFilter>(50);
    
    auto subscription = eventBus->addFilter<TestEvent>(
        filter,
        [&](const TestEvent& event, const EventMetadata&) {
            eventReceived = true;
            EXPECT_GT(event.value, 50);
        }
    );

    // This event should be filtered out
    TestEvent lowEvent{42};
    eventBus->publish(lowEvent);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(eventReceived);

    // This event should pass the filter
    TestEvent highEvent{100};
    eventBus->publish(highEvent);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(eventReceived);
}

TEST_F(EventBusTest, EventLifecycleTracking) {
    std::vector<EventState> stateTransitions;
    
    eventBus->subscribe<TestEvent>(
        [&](const TestEvent&, const EventMetadata& metadata) {
            stateTransitions.push_back(metadata.state);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    );

    TestEvent event{42};
    eventBus->publish(event);
    
    // Wait for event processing
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Verify state transitions
    ASSERT_FALSE(stateTransitions.empty());
    EXPECT_EQ(stateTransitions.back(), EventState::Processing);
}

TEST_F(EventBusTest, QueuePositionTracking) {
    std::vector<size_t> capturedPositions;
    
    eventBus->subscribe<TestEvent>(
        [&](const TestEvent&, const EventMetadata& metadata) {
            capturedPositions.push_back(metadata.queuePosition);
        }
    );

    // Publish multiple events rapidly
    for (int i = 0; i < 5; ++i) {
        TestEvent event{i};
        eventBus->publish(event);
    }
    
    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT_EQ(capturedPositions.size(), 5);
    // Verify positions are sequential
    for (size_t i = 0; i < capturedPositions.size(); ++i) {
        EXPECT_EQ(capturedPositions[i], i);
    }
}

TEST_F(EventBusTest, ProcessingTimeTracking) {
    std::vector<std::chrono::microseconds> processingTimes;
    
    eventBus->subscribe<TestEvent>(
        [&](const TestEvent&, const EventMetadata& metadata) {
            // Simulate varying processing times
            std::this_thread::sleep_for(std::chrono::milliseconds(metadata.queuePosition * 10));
            processingTimes.push_back(metadata.processingTime);
        }
    );

    // Publish events
    for (int i = 0; i < 3; ++i) {
        TestEvent event{i};
        eventBus->publish(event);
    }
    
    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT_EQ(processingTimes.size(), 3);
    // Verify processing times increase with queue position
    for (size_t i = 1; i < processingTimes.size(); ++i) {
        EXPECT_GT(processingTimes[i], processingTimes[i-1]);
    }
}
