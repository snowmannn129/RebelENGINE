#pragma once

#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <queue>
#include <chrono>
#include <future>
#include <condition_variable>
#include <type_traits>
#include <concepts>
#include <any>

namespace RebelCAD {
namespace Core {

/**
 * @brief Event priority levels for controlling dispatch order
 */
enum class EventPriority {
    High,    // Critical system events, immediate dispatch required
    Normal,  // Standard application events
    Low      // Background or non-critical events
};

/**
 * @brief Event lifecycle states for tracking event processing
 */
enum class EventState {
    Created,    // Event just created
    Queued,     // Event added to processing queue
    Processing, // Event being dispatched to subscribers
    Completed,  // Event successfully processed
    Failed      // Event processing failed
};

/**
 * @brief Event metadata for tracking and filtering
 */
struct EventMetadata {
    std::chrono::system_clock::time_point timestamp;
    EventPriority priority;
    std::string_view eventTypeName;
    EventState state{EventState::Created};
    size_t queuePosition{0};
    std::chrono::microseconds processingTime{0};
};

/**
 * @brief Event filter interface for controlling event processing
 */
template<typename EventType>
class EventFilter {
public:
    virtual ~EventFilter() = default;
    virtual bool shouldProcess(const EventType& event, const EventMetadata& metadata) const = 0;
};

/**
 * @brief Central event bus system for handling application-wide events
 * 
 * Thread-safe event system that supports event subscription, unsubscription,
 * and event dispatch with priority levels.
 */
class EventBus {
public:
    /**
     * @brief Gets the singleton instance of the event bus
     * @return Reference to the event bus instance
     */
    static EventBus& getInstance();

    /**
     * @brief Subscribes to an event type with specified priority
     * @param callback Function to call when event occurs
     * @param priority Priority level for the subscription
     * @return Subscription ID for later unsubscription
     */
    template<typename EventType>
    size_t subscribe(
        std::function<void(const EventType&, const EventMetadata&)> callback,
        EventPriority priority = EventPriority::Normal
    );

    /**
     * @brief Unsubscribes from an event
     * @param subscriptionId ID returned from subscribe()
     */
    void unsubscribe(size_t subscriptionId);

    /**
     * @brief Publishes an event to all subscribers
     * @param event Event data to publish
     * @param priority Priority level for this event
     */
    template<typename EventType>
    void publish(
        const EventType& event,
        EventPriority priority = EventPriority::Normal
    );

    /**
     * @brief Gets performance metrics for event processing
     * @return Map of event type to average processing time
     */
    std::unordered_map<std::string_view, double> getPerformanceMetrics() const;

    /**
     * @brief Adds a filter for event processing
     * @param filter Custom filter implementation
     * @param callback Function to call when event passes filter
     * @param priority Priority level for the subscription
     * @return Subscription ID that includes this filter
     */
    template<typename EventType>
    size_t addFilter(
        std::shared_ptr<EventFilter<EventType>> filter,
        std::function<void(const EventType&, const EventMetadata&)> callback,
        EventPriority priority = EventPriority::Normal
    );

private:
    EventBus();
    ~EventBus();
    
    static EventBus instance_;
    static std::mutex instanceMutex_;

    // Event processing thread
    std::thread processingThread_;
    std::atomic<bool> running_{true};
    std::condition_variable queueCondition_;
    std::mutex queueMutex_;

    // Event queue management
    struct QueuedEvent {
        std::any eventData;
        EventMetadata metadata;
        const std::type_info* type;
    };
    std::queue<QueuedEvent> eventQueue_;

    struct Subscription {
        size_t id;
        std::function<void(const void*, const EventMetadata&)> callback;
        const std::type_info* eventType;
        EventPriority priority;
        std::shared_ptr<void> filter; // Type-erased EventFilter
    };

    void processEventQueue();
    void updateEventState(EventMetadata& metadata, EventState newState);

    // Map of priority queues for subscriptions
    std::unordered_map<EventPriority, std::unordered_map<size_t, Subscription>> prioritySubscriptions_;
    std::mutex subscriptionMutex_;
    size_t nextSubscriptionId_{0};

    // Performance tracking
    mutable std::mutex metricsMutex_;
    std::unordered_map<std::string_view, std::vector<double>> processingTimes_;

    // Helper to type-erase callbacks
    template<typename EventType>
    static std::function<void(const void*, const EventMetadata&)> wrapCallback(
        std::function<void(const EventType&, const EventMetadata&)> callback
    );
};

// Template implementation
template<typename EventType>
size_t EventBus::subscribe(
    std::function<void(const EventType&, const EventMetadata&)> callback,
    EventPriority priority
) {
    std::lock_guard<std::mutex> lock(subscriptionMutex_);
    
    size_t id = nextSubscriptionId_++;
    prioritySubscriptions_[priority][id] = Subscription{
        id,
        wrapCallback<EventType>(callback),
        &typeid(EventType),
        priority
    };
    
    return id;
}

template<typename EventType>
void EventBus::publish(const EventType& event, EventPriority priority) {
    static_assert(std::is_copy_constructible_v<EventType>, 
                  "Event type must be copy constructible");

    EventMetadata metadata{
        std::chrono::system_clock::now(),
        priority,
        typeid(EventType).name()
    };

    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        metadata.queuePosition = eventQueue_.size();
        updateEventState(metadata, EventState::Queued);
        
        eventQueue_.push(QueuedEvent{
            std::make_any<EventType>(event),
            metadata,
            &typeid(EventType)
        });
    }

    queueCondition_.notify_one();
}

template<typename EventType>
std::function<void(const void*, const EventMetadata&)> EventBus::wrapCallback(
    std::function<void(const EventType&, const EventMetadata&)> callback
) {
    return [callback](const void* event, const EventMetadata& metadata) {
        callback(*static_cast<const EventType*>(event), metadata);
    };
}

/**
 * @brief Adds a filter for event processing
 * @param filter Custom filter implementation
 * @return Subscription ID that includes this filter
 */
template<typename EventType>
size_t EventBus::addFilter(
    std::shared_ptr<EventFilter<EventType>> filter,
    std::function<void(const EventType&, const EventMetadata&)> callback,
    EventPriority priority
) {
    std::lock_guard<std::mutex> lock(subscriptionMutex_);
    
    size_t id = nextSubscriptionId_++;
    prioritySubscriptions_[priority][id] = Subscription{
        id,
        wrapCallback<EventType>(callback),
        &typeid(EventType),
        priority,
        filter
    };
    
    return id;
}

} // namespace Core
} // namespace RebelCAD
