#include <gtest/gtest.h>
#include "core/EventSystem.h"
#include <thread>
#include <chrono>

using namespace RebelCAD::Core;

// Test event types
class TestEvent : public Event {
public:
    TestEvent(const std::string& data) 
        : Event("TestEvent"), data(data) {}
    std::string data;
};

class PriorityEvent : public Event {
public:
    PriorityEvent() 
        : Event("PriorityEvent", EventPriority::Critical) {}
};

TEST(EventSystem, BasicEventHandling) {
    auto& eventBus = EventBus::getInstance();
    eventBus.clear(); // Start fresh

    bool eventHandled = false;
    std::string receivedData;

    auto subscription = eventBus.subscribe<TestEvent>(
        [&](Event& e) {
            auto& testEvent = static_cast<TestEvent&>(e);
            eventHandled = true;
            receivedData = testEvent.data;
        }
    );

    auto event = std::make_shared<TestEvent>("test data");
    eventBus.publish(event).wait(); // Wait for async completion

    EXPECT_TRUE(eventHandled);
    EXPECT_EQ(receivedData, "test data");

    eventBus.unsubscribe(subscription);
}

TEST(EventSystem, PriorityHandling) {
    auto& eventBus = EventBus::getInstance();
    eventBus.clear();

    std::vector<int> executionOrder;

    eventBus.subscribe<TestEvent>(
        [&](Event& e) {
            executionOrder.push_back(1);
        },
        EventPriority::Low
    );

    eventBus.subscribe<TestEvent>(
        [&](Event& e) {
            executionOrder.push_back(2);
        },
        EventPriority::Normal
    );

    eventBus.subscribe<TestEvent>(
        [&](Event& e) {
            executionOrder.push_back(3);
            e.setHandled(true); // Mark as handled
        },
        EventPriority::High
    );

    auto event = std::make_shared<TestEvent>("priority test");
    eventBus.publish(event).wait();

    // Verify execution order (high to low priority)
    ASSERT_EQ(executionOrder.size(), 3);
    EXPECT_EQ(executionOrder[0], 3); // High priority
    EXPECT_EQ(executionOrder[1], 2); // Normal priority
    EXPECT_EQ(executionOrder[2], 1); // Low priority
}

TEST(EventSystem, CriticalEventHandling) {
    auto& eventBus = EventBus::getInstance();
    eventBus.clear();

    int handlerCallCount = 0;

    eventBus.subscribe<PriorityEvent>(
        [&](Event& e) {
            handlerCallCount++;
            e.setHandled(true);
        }
    );

    eventBus.subscribe<PriorityEvent>(
        [&](Event& e) {
            handlerCallCount++;
        }
    );

    auto event = std::make_shared<PriorityEvent>();
    eventBus.publish(event).wait();

    // Critical events should be handled by all subscribers regardless of handled state
    EXPECT_EQ(handlerCallCount, 2);
}

TEST(EventSystem, CategoryFiltering) {
    auto& eventBus = EventBus::getInstance();
    eventBus.clear();

    int handlerCallCount = 0;

    // Create a category filter for "test" category
    auto filter = eventBus.createCategoryFilter({"test"});

    eventBus.subscribe<TestEvent>(
        [&](Event& e) {
            handlerCallCount++;
        },
        EventPriority::Normal,
        filter
    );

    // Create and publish an event with matching category
    auto event1 = std::make_shared<TestEvent>("test data");
    event1->addCategory("test");
    eventBus.publish(event1).wait();

    // Create and publish an event without matching category
    auto event2 = std::make_shared<TestEvent>("other data");
    event2->addCategory("other");
    eventBus.publish(event2).wait();

    // Only the event with matching category should be handled
    EXPECT_EQ(handlerCallCount, 1);
}

TEST(EventSystem, AsyncEventProcessing) {
    auto& eventBus = EventBus::getInstance();
    eventBus.clear();

    std::atomic<int> handlerCallCount{0};
    std::vector<std::future<void>> futures;

    auto subscription = eventBus.subscribe<TestEvent>(
        [&](Event& e) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            handlerCallCount++;
        }
    );

    // Publish multiple events asynchronously
    const int numEvents = 10;
    for (int i = 0; i < numEvents; i++) {
        auto event = std::make_shared<TestEvent>("async test");
        futures.push_back(eventBus.publish(event));
    }

    // Wait for all events to be processed
    for (auto& future : futures) {
        future.wait();
    }

    EXPECT_EQ(handlerCallCount, numEvents);
    eventBus.unsubscribe(subscription);
}

TEST(EventSystem, ThreadSafety) {
    auto& eventBus = EventBus::getInstance();
    eventBus.clear();

    std::atomic<int> eventCount{0};
    const int numThreads = 10;
    const int eventsPerThread = 100;

    auto subscription = eventBus.subscribe<TestEvent>(
        [&](Event& e) {
            eventCount++;
        }
    );

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; i++) {
        threads.push_back(std::thread([&]() {
            for (int j = 0; j < eventsPerThread; j++) {
                auto event = std::make_shared<TestEvent>("thread test");
                eventBus.publish(event).wait();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(eventCount, numThreads * eventsPerThread);
    eventBus.unsubscribe(subscription);
}

TEST(EventSystem, ImmediatePriorityHandling) {
    auto& eventBus = EventBus::getInstance();
    eventBus.clear();

    bool immediateHandled = false;
    bool normalHandled = false;

    eventBus.subscribe<TestEvent>(
        [&](Event& e) {
            if (e.getPriority() == EventPriority::Immediate) {
                immediateHandled = true;
            } else {
                normalHandled = true;
            }
        }
    );

    // Create and publish an immediate priority event
    auto immediateEvent = std::make_shared<TestEvent>("immediate");
    immediateEvent->setPriority(EventPriority::Immediate);
    eventBus.publish(immediateEvent).wait();

    // Create and publish a normal priority event
    auto normalEvent = std::make_shared<TestEvent>("normal");
    eventBus.publish(normalEvent).wait();

    EXPECT_TRUE(immediateHandled);
    EXPECT_TRUE(normalHandled);
}
