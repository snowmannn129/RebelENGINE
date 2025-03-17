interface PrioritizedCallback<T> {
    callback: (data: T) => void;
    priority: number;
}

interface QueuedEvent<K extends keyof any, T> {
    eventType: K;
    data: T;
    timestamp: number;
    delay?: number;
}

type EventCallback<T> = (data: T) => void;

export class EventEmitter<Events extends Record<string, any>> {
    private listeners: Map<keyof Events, Array<PrioritizedCallback<Events[keyof Events]>>>;
    private eventQueue: Array<QueuedEvent<keyof Events, Events[keyof Events]>>;
    private isProcessing: boolean;
    private maxQueueSize: number;

    constructor(maxQueueSize: number = 1000) {
        this.listeners = new Map();
        this.eventQueue = [];
        this.isProcessing = false;
        this.maxQueueSize = maxQueueSize;
    }

    /**
     * Register an event listener with optional priority
     * @param eventType The type of event to listen for
     * @param callback The callback function to execute
     * @param priority Priority level (higher numbers execute first, default: 0)
     */
    public on<K extends keyof Events>(
        eventType: K, 
        callback: EventCallback<Events[K]>, 
        priority: number = 0
    ): void {
        if (!this.listeners.has(eventType)) {
            this.listeners.set(eventType, []);
        }
        const callbacks = this.listeners.get(eventType) as Array<PrioritizedCallback<Events[K]>>;
        
        // Add the callback with priority
        callbacks.push({ callback, priority });
        
        // Sort callbacks by priority (highest to lowest)
        callbacks.sort((a, b) => b.priority - a.priority);
    }

    public off<K extends keyof Events>(eventType: K, callback: EventCallback<Events[K]>): void {
        if (!this.listeners.has(eventType)) return;
        const callbacks = this.listeners.get(eventType) as Array<PrioritizedCallback<Events[K]>>;
        const index = callbacks.findIndex(cb => cb.callback === callback);
        if (index !== -1) {
            callbacks.splice(index, 1);
        }
    }

    /**
     * Emit an event immediately
     * @param eventType The type of event to emit
     * @param data The event data
     */
    public emit<K extends keyof Events>(eventType: K, data: Events[K]): void {
        this.processEvent({ eventType, data, timestamp: Date.now() });
    }

    /**
     * Queue an event for later processing
     * @param eventType The type of event to queue
     * @param data The event data
     * @param delay Optional delay in milliseconds before processing
     * @returns true if event was queued successfully, false if queue is full
     */
    public queueEvent<K extends keyof Events>(
        eventType: K, 
        data: Events[K], 
        delay?: number
    ): boolean {
        if (this.eventQueue.length >= this.maxQueueSize) {
            console.warn('Event queue is full, dropping event:', eventType);
            return false;
        }

        this.eventQueue.push({
            eventType,
            data,
            timestamp: Date.now(),
            delay
        });

        // Start processing if not already processing
        if (!this.isProcessing) {
            this.processQueue();
        }

        return true;
    }

    /**
     * Process a single event
     */
    private processEvent<K extends keyof Events>(event: QueuedEvent<K, Events[K]>): void {
        if (!this.listeners.has(event.eventType)) return;
        
        const callbacks = this.listeners.get(event.eventType) as Array<PrioritizedCallback<Events[K]>>;
        callbacks.forEach(({ callback }) => {
            try {
                callback(event.data);
            } catch (error) {
                console.error(`Error in event callback for ${String(event.eventType)}:`, error);
            }
        });
    }

    /**
     * Process the event queue
     */
    private processQueue(): void {
        if (this.isProcessing || this.eventQueue.length === 0) return;

        this.isProcessing = true;
        const currentTime = Date.now();

        // Process all events that are ready
        while (this.eventQueue.length > 0) {
            const event = this.eventQueue[0];
            
            // Check if the event should be processed now
            if (event.delay && (currentTime - event.timestamp) < event.delay) {
                break;
            }

            // Remove and process the event
            this.eventQueue.shift();
            this.processEvent(event);
        }

        this.isProcessing = false;

        // If there are still delayed events, schedule the next processing
        if (this.eventQueue.length > 0) {
            const nextEvent = this.eventQueue[0];
            if (nextEvent.delay) {
                const remainingDelay = nextEvent.delay - (currentTime - nextEvent.timestamp);
                if (remainingDelay > 0) {
                    setTimeout(() => this.processQueue(), remainingDelay);
                }
            }
        }
    }

    /**
     * Register a one-time event listener with optional priority
     * @param eventType The type of event to listen for
     * @param callback The callback function to execute
     * @param priority Priority level (higher numbers execute first, default: 0)
     */
    public once<K extends keyof Events>(
        eventType: K, 
        callback: EventCallback<Events[K]>, 
        priority: number = 0
    ): void {
        const wrappedCallback: EventCallback<Events[K]> = (data: Events[K]) => {
            this.off(eventType, wrappedCallback);
            callback(data);
        };
        this.on(eventType, wrappedCallback, priority);
    }

    public removeAllListeners(eventType?: keyof Events): void {
        if (eventType) {
            this.listeners.delete(eventType);
        } else {
            this.listeners.clear();
        }
    }

    /**
     * Get the current queue length
     */
    public getQueueLength(): number {
        return this.eventQueue.length;
    }

    /**
     * Clear all queued events
     */
    public clearQueue(): void {
        this.eventQueue = [];
    }

    /**
     * Get the current queue status
     */
    public getQueueStatus(): {
        length: number;
        maxSize: number;
        isProcessing: boolean;
    } {
        return {
            length: this.eventQueue.length,
            maxSize: this.maxQueueSize,
            isProcessing: this.isProcessing
        };
    }

    public listenerCount(eventType: keyof Events): number {
        return this.listeners.has(eventType) ? this.listeners.get(eventType)!.length : 0;
    }

    /**
     * Get the current listeners for an event type, sorted by priority
     * @param eventType The type of event
     * @returns Array of prioritized callbacks
     */
    public getListeners<K extends keyof Events>(eventType: K): Array<PrioritizedCallback<Events[K]>> {
        return this.listeners.has(eventType) 
            ? [...this.listeners.get(eventType)!] 
            : [];
    }
}
