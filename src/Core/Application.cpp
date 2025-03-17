#include "core/Application.h"
#include "graphics/GraphicsSystem.h"
#include <chrono>
#include <thread>
#include <cmath>

#define _USE_MATH_DEFINES

using namespace RebelCAD::Graphics;

namespace rebel::core {

class Application::ApplicationImpl {
public:
    ApplicationImpl() {}
    
    void initialize() {
        REBEL_LOG_INFO("Initializing graphics subsystem...");
        GraphicsSystem::getInstance().initialize(
            GraphicsAPI::Vulkan,
            WindowProperties("RebelCAD", 1280, 720)
        );
        
        REBEL_LOG_INFO("Initializing UI subsystem...");
        // UI initialization will be implemented here
        
        REBEL_LOG_INFO("Loading plugins...");
        // Plugin loading will be implemented here
    }
    
    void shutdown() {
        REBEL_LOG_INFO("Shutting down application...");
        GraphicsSystem::getInstance().shutdown();
    }
};

// Singleton instance
Application& Application::getInstance() {
    static Application instance;
    return instance;
}

Application::Application(const std::string& name)
    : impl(std::make_unique<ApplicationImpl>())
    , appName(name)
    , running(false) {
    REBEL_LOG_INFO("Creating RebelCAD application instance");
}

Application::~Application() {
    if (running) {
        shutdown();
    }
}

void Application::initialize() {
    try {
        REBEL_LOG_INFO("Initializing " + appName);
        
        initializeLogging();
        impl->initialize();
        
        running = true;
        REBEL_LOG_INFO("Initialization complete");
    }
    catch (const Error& e) {
        REBEL_LOG_CRITICAL("Failed to initialize application: " + std::string(e.what()));
        throw;
    }
}

void Application::run() {
    if (!running) {
        REBEL_THROW_ERROR(ErrorCode::SystemError, "Application not initialized");
    }

    REBEL_LOG_INFO("Starting main loop");
    
    try {
        while (running) {
            processEvents();
            update();
            render();
            
            // Temporary: Add a small sleep to prevent CPU overuse
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        }
    }
    catch (const Error& e) {
        REBEL_LOG_CRITICAL("Fatal error in main loop: " + std::string(e.what()));
        throw;
    }
}

void Application::shutdown() {
    if (!running) {
        return;
    }

    try {
        impl->shutdown();
        running = false;
        REBEL_LOG_INFO("Application shutdown complete");
    }
    catch (const Error& e) {
        REBEL_LOG_CRITICAL("Error during shutdown: " + std::string(e.what()));
        throw;
    }
}

void Application::initializeLogging() {
    // Logging system is self-initializing via singleton
    REBEL_LOG_INFO("Logging system initialized");
}

void Application::initializeGraphics() {
    REBEL_LOG_INFO("Graphics system initialization placeholder");
}

void Application::initializeUI() {
    REBEL_LOG_INFO("UI system initialization placeholder");
}

void Application::initializePlugins() {
    REBEL_LOG_INFO("Plugin system initialization placeholder");
}

void Application::processEvents() {
    // Event processing handled by graphics system in beginFrame
}

void Application::update() {
    // Update logic will be implemented here
}

void Application::render() {
    auto& graphics = GraphicsSystem::getInstance();
    
    graphics.beginFrame();
    
    // Rendering will be implemented here
    
    graphics.endFrame();
    graphics.present();
}

} // namespace rebel::core
