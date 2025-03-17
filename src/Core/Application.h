#pragma once

#include <memory>
#include <string>
#include "core/Log.h"
#include "core/Error.h"

namespace rebel::core {

class Application {
public:
    Application(const std::string& appName = "RebelCAD");
    ~Application();

    // Delete copy and move constructors/assignments
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    // Core application functions
    void initialize();
    void run();
    void shutdown();

    // Getters
    bool isRunning() const { return running; }
    const std::string& getAppName() const { return appName; }

    // Static instance access
    static Application& getInstance();

private:
    class ApplicationImpl;
    std::unique_ptr<ApplicationImpl> impl;
    
    std::string appName;
    bool running;

    // Initialize subsystems
    void initializeLogging();
    void initializeGraphics();
    void initializeUI();
    void initializePlugins();

    // Main loop handlers
    void processEvents();
    void update();
    void render();
};

} // namespace rebel::core
