#pragma once

#include "Log.h"
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <typeindex>
#include <any>
#include <queue>
#include <future>
#include <condition_variable>
#include <thread>
#include <sstream>

namespace RebelCAD {
namespace Core {

// Forward declarations
class Event;
class IEventFilter;
class EventBus;

// Event priority levels
enum class EventPriority {
    Lowest = 0,
    Low = 1,
    Normal = 2,
    High = 3,
    Critical = 4,
    Immediate = 5  // Bypass queue and process immediately
};

// Type aliases
using EventHandlerFn = std::function<void(Event&)>;

// Event traits
template<typename T>
struct EventTraits {
    static constexpr EventPriority DefaultPriority = EventPriority::Normal;
};

/**
 * @brief Base class for all events in the system
 */
class Event {
public:
    virtual ~Event() = default;

    EventPriority getPriority() const { return priority_; }
    void setPriority(EventPriority priority) { priority_ = priority; }
    
    bool isHandled() const { return handled_; }
    void setHandled(bool handled) { handled_ = handled; }

    const std::string& getName() const { return name_; }
    double getTimestamp() const { return timestamp_; }
    
    // Allow events to be categorized and filtered
    const std::vector<std::string>& getCategories() const { return categories_; }
    void addCategory(const std::string& category) { categories_.push_back(category); }

protected:
    Event(const std::string& name, EventPriority priority = EventPriority::Normal)
        : name_(name)
        , priority_(priority)
        , handled_(false)
        , timestamp_(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0) {}

private:
    std::string name_;
    EventPriority priority_;
    bool handled_;
    double timestamp_;
    std::vector<std::string> categories_;
};

/**
 * @brief Interface for event filters
 */
class IEventFilter {
public:
    virtual ~IEventFilter() = default;
    virtual bool shouldProcess(const Event& event) const = 0;
};

/**
 * @brief Category-based event filter
 */
class CategoryFilter : public IEventFilter {
public:
    explicit CategoryFilter(std::vector<std::string> categories) 
        : categories_(std::move(categories)) {}

    bool shouldProcess(const Event& event) const override {
        const auto& eventCategories = event.getCategories();
        return std::any_of(categories_.begin(), categories_.end(),
            [&eventCategories](const std::string& category) {
                return std::find(eventCategories.begin(), eventCategories.end(), category) 
                       != eventCategories.end();
            });
    }

private:
    std::vector<std::string> categories_;
};

/**
 * @brief Priority-based event filter
 */
class PriorityFilter : public IEventFilter {
public:
    explicit PriorityFilter(EventPriority minPriority) 
        : minPriority_(minPriority) {}

    bool shouldProcess(const Event& event) const override {
        return static_cast<int>(event.getPriority()) >= static_cast<int>(minPriority_);
    }

private:
    EventPriority minPriority_;
};

/**
 * @brief Represents a subscription to an event
 */
struct EventSubscription {
    std::size_t id;
    EventHandlerFn handler;
    EventPriority priority;
    std::shared_ptr<IEventFilter> filter;
};

/**
 * @brief Queued event for async processing
 */
class QueuedEvent {
public:
    QueuedEvent(std::shared_ptr<Event> e, std::promise<void>&& p)
        : event(std::move(e))
        , completionPromise(std::move(p)) {}
    
    // Move constructor
    QueuedEvent(QueuedEvent&& other) noexcept
        : event(std::move(other.event))
        , completionPromise(std::move(other.completionPromise)) {}
    
    // Move assignment
    QueuedEvent& operator=(QueuedEvent&& other) noexcept {
        if (this != &other) {
            event = std::move(other.event);
            completionPromise = std::move(other.completionPromise);
        }
        return *this;
    }
    
    // Delete copy operations
    QueuedEvent(const QueuedEvent&) = delete;
    QueuedEvent& operator=(const QueuedEvent&) = delete;
    
    // Comparison operators for priority queue
    bool operator<(const QueuedEvent& other) const {
        return static_cast<int>(event->getPriority()) < 
               static_cast<int>(other.event->getPriority());
    }
    
    bool operator>(const QueuedEvent& other) const {
        return static_cast<int>(event->getPriority()) > 
               static_cast<int>(other.event->getPriority());
    }

    std::shared_ptr<Event> event;
    std::promise<void> completionPromise;
};

/**
 * @brief Central event bus that manages event distribution
 */
class EventBus {
public:
    static EventBus& getInstance() {
        static EventBus instance;
        return instance;
    }

    /**
     * @brief Subscribe to an event type with optional filter
     */
    template<typename T>
    std::size_t subscribe(
        const EventHandlerFn& handler, 
        EventPriority priority = EventTraits<T>::DefaultPriority,
        std::shared_ptr<IEventFilter> filter = nullptr) {
        
        static_assert(std::is_base_of<Event, T>::value, "T must inherit from Event");
        
        std::unique_lock<std::shared_mutex> lock(mutex_);
        auto& handlers = subscribers_[std::type_index(typeid(T))];
        
        EventSubscription subscription{
            nextSubscriptionId_++,
            handler,
            priority,
            filter
        };
        
        handlers.push_back(subscription);
        std::sort(handlers.begin(), handlers.end(), 
            [](const EventSubscription& a, const EventSubscription& b) {
                return static_cast<int>(a.priority) > static_cast<int>(b.priority);
            });
            
        return subscription.id;
    }

    /**
     * @brief Create a category filter
     */
    std::shared_ptr<IEventFilter> createCategoryFilter(std::vector<std::string> categories) {
        return std::make_shared<CategoryFilter>(std::move(categories));
    }

    /**
     * @brief Unsubscribe from an event using the subscription ID
     */
    void unsubscribe(std::size_t subscriptionId) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        for (auto& [type, handlers] : subscribers_) {
            auto it = std::remove_if(handlers.begin(), handlers.end(),
                [subscriptionId](const EventSubscription& sub) {
                    return sub.id == subscriptionId;
                });
            handlers.erase(it, handlers.end());
        }
    }

    /**
     * @brief Publish an event to all subscribers
     */
    template<typename T>
    std::future<void> publish(std::shared_ptr<T> event) {
        static_assert(std::is_base_of<Event, T>::value, "T must inherit from Event");
        
        // Handle immediate priority events directly
        if (event->getPriority() == EventPriority::Immediate) {
            std::promise<void> promise;
            auto future = promise.get_future();
            processEvent(*event);
            promise.set_value();
            return future;
        }
        
        // Queue other events for async processing
        std::promise<void> promise;
        auto future = promise.get_future();
        
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            eventQueue_.emplace(std::move(event), std::move(promise));
        }
        queueCV_.notify_one();
        
        return future;
    }

    void start() {
        if (!processingThread_.joinable()) {
            running_ = true;
            processingThread_ = std::thread(&EventBus::processEvents, this);
        }
    }

    void stop() {
        running_ = false;
        queueCV_.notify_all();
        if (processingThread_.joinable()) {
            processingThread_.join();
        }
    }

    void clear() {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        subscribers_.clear();
        nextSubscriptionId_ = 0;
    }

private:
    // Container types
    using EventSubscriptionVector = std::vector<EventSubscription>;
    using SubscriberMap = std::unordered_map<std::type_index, EventSubscriptionVector>;
    using EventQueueContainer = std::vector<QueuedEvent>;
    using EventQueue = std::priority_queue<QueuedEvent, EventQueueContainer, std::greater<QueuedEvent>>;

    SubscriberMap subscribers_;
    std::shared_mutex mutex_;
    std::size_t nextSubscriptionId_;
    EventQueue eventQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCV_;
    std::thread processingThread_;
    std::atomic<bool> running_{false};

    EventBus() 
        : nextSubscriptionId_(0)
    {
        start();
    }
    
    ~EventBus() {
        stop();
    }
    
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    void processEvent(Event& event) {
        std::stringstream ss;
        ss << "Processing event: " << event.getName() 
           << " (Priority: " << static_cast<int>(event.getPriority()) << ")";
        Logger::getInstance().debug(ss.str());

        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = subscribers_.find(std::type_index(typeid(event)));
        if (it != subscribers_.end()) {
            for (const auto& subscription : it->second) {
                if (!event.isHandled() || event.getPriority() == EventPriority::Critical) {
                    // Apply filters
                    if (subscription.filter && !subscription.filter->shouldProcess(event)) {
                        continue;
                    }
                    
                    try {
                        subscription.handler(event);
                    } catch (const std::exception& e) {
                        Logger::getInstance().error(std::string("Error in event handler: ") + e.what());
                    }
                }
            }
        }
    }

    void processEvents() {
        while (running_) {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCV_.wait(lock, [this] { 
                return !eventQueue_.empty() || !running_; 
            });
            
            if (!running_) break;
            
            // Process all events in the queue
            while (!eventQueue_.empty()) {
                auto queuedEvent = std::move(const_cast<QueuedEvent&>(eventQueue_.top()));
                eventQueue_.pop();
                lock.unlock();
                
                processEvent(*queuedEvent.event);
                queuedEvent.completionPromise.set_value();
                
                lock.lock();
            }
        }
    }
};

} // namespace Core
} // namespace RebelCAD
