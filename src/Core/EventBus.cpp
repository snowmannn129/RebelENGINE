#include "Core/EventBus.h"
#include "Core/Error.h"
#include "Core/Log.h"
#include <stdexcept>
#include <sstream>
#include <numeric>
#include <thread>

namespace RebelCAD {
namespace Core {

EventBus EventBus::instance_;
std::mutex EventBus::instanceMutex_;

EventBus::EventBus() : processingThread_(&EventBus::processEventQueue, this) {
    REBEL_LOG_INFO("Event bus initialized with processing thread");
}

EventBus::~EventBus() {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        running_ = false;
    }
    queueCondition_.notify_one();
    
    if (processingThread_.joinable()) {
        processingThread_.join();
    }
    REBEL_LOG_INFO("Event bus shutdown complete");
}

void EventBus::processEventQueue() {
    while (running_) {
        QueuedEvent event;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCondition_.wait(lock, [this] { 
                return !eventQueue_.empty() || !running_; 
            });
            
            if (!running_) break;
            
            event = std::move(eventQueue_.front());
            eventQueue_.pop();
        }
        
        try {
            updateEventState(event.metadata, EventState::Processing);
            auto start = std::chrono::high_resolution_clock::now();

            // Process event through priority levels
            for (auto currentPriority : {EventPriority::High, EventPriority::Normal, EventPriority::Low}) {
                std::lock_guard<std::mutex> lock(subscriptionMutex_);
                auto& subscriptions = prioritySubscriptions_[currentPriority];
                
                for (const auto& [_, subscription] : subscriptions) {
                    if (*subscription.eventType == *event.type) {
                        // Check if subscription has a filter
                        bool shouldProcess = true;
                        if (subscription.filter) {
                            // Type-safe filter check using std::any
                            try {
                                const void* eventData = std::any_cast<const void*>(event.eventData);
                                if (auto* typedFilter = static_cast<const EventFilter<void>*>(subscription.filter.get())) {
                                    shouldProcess = typedFilter->shouldProcess(eventData, event.metadata);
                                }
                            } catch (const std::bad_any_cast&) {
                                REBEL_LOG_ERROR("Failed to cast event data for filter check");
                                continue;
                            }
                        }
                        
                        if (shouldProcess) {
                            if (const void* eventData = std::any_cast<const void*>(event.eventData)) {
                                subscription.callback(eventData, event.metadata);
                            }
                        }
                    }
                }
                
                if (currentPriority == event.metadata.priority) {
                    break;
                }
            }

            auto end = std::chrono::high_resolution_clock::now();
            event.metadata.processingTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            updateEventState(event.metadata, EventState::Completed);

            // Track performance
            std::lock_guard<std::mutex> metricsLock(metricsMutex_);
            processingTimes_[event.metadata.eventTypeName].push_back(
                event.metadata.processingTime.count()
            );
        }
        catch (const std::exception& e) {
            updateEventState(event.metadata, EventState::Failed);
            std::stringstream ss;
            ss << "Error processing event: " << e.what();
            REBEL_LOG_ERROR(ss.str());
        }
    }
}

void EventBus::updateEventState(EventMetadata& metadata, EventState newState) {
    metadata.state = newState;
    
    std::stringstream ss;
    ss << "Event " << metadata.eventTypeName 
       << " state changed to " << static_cast<int>(newState)
       << " (Queue position: " << metadata.queuePosition << ")";
       
    if (newState == EventState::Completed) {
        ss << " Processing time: " << metadata.processingTime.count() << "µs";
    }
    
    REBEL_LOG_DEBUG(ss.str());
}

EventBus& EventBus::getInstance() {
    return instance_;
}

void EventBus::unsubscribe(size_t subscriptionId) {
    try {
        std::lock_guard<std::mutex> lock(subscriptionMutex_);
        
        // Search through all priority levels
        for (auto& [priority, subscriptions] : prioritySubscriptions_) {
            auto it = subscriptions.find(subscriptionId);
            if (it != subscriptions.end()) {
                subscriptions.erase(it);
                std::stringstream ss;
                ss << "Successfully unsubscribed: " << subscriptionId;
                REBEL_LOG_DEBUG(ss.str());
                return;
            }
        }
        
        std::stringstream ss;
        ss << "Attempted to unsubscribe non-existent subscription: " << subscriptionId;
        REBEL_LOG_WARNING(ss.str());
    }
    catch (const std::exception& e) {
        std::stringstream ss;
        ss << "Error in unsubscribe: " << e.what();
        REBEL_LOG_ERROR(ss.str());
        throw core::Error(core::ErrorCode::InvalidOperation, 
                         "Failed to unsubscribe from event: " + std::string(e.what()));
    }
}

std::unordered_map<std::string_view, double> EventBus::getPerformanceMetrics() const {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    
    std::unordered_map<std::string_view, double> averages;
    for (const auto& [eventType, times] : processingTimes_) {
        if (!times.empty()) {
            double sum = std::accumulate(times.begin(), times.end(), 0.0);
            averages[eventType] = sum / times.size();
        }
    }
    
    return averages;
}

} // namespace Core
} // namespace RebelCAD
