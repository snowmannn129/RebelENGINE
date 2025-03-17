#include "core/Application.h"
#include "core/Log.h"
#include "core/Error.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        // Create and initialize the application
        auto& app = rebel::core::Application::getInstance();
        app.initialize();

        // Log startup information
        rebel::core::Logger::getInstance().info("RebelCAD v0.1.0 starting up...");
        rebel::core::Logger::getInstance().info("Command line arguments:");
        for (int i = 0; i < argc; ++i) {
            rebel::core::Logger::getInstance().info(std::string("  arg[") + 
                                                  std::to_string(i) + 
                                                  "]: " + 
                                                  argv[i]);
        }

        // Run the application
        app.run();

        return 0;
    }
    catch (const rebel::core::Error& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        std::cerr << "File: " << e.file() << ":" << e.line() << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return 2;
    }
    catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 3;
    }
}
