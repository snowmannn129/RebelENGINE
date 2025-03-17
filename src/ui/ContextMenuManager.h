#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>

namespace RebelCAD {
namespace UI {

// Forward declarations
class MainWindow;

struct MenuPosition {
    float x;
    float y;
    
    MenuPosition(float xPos = -1, float yPos = -1) : x(xPos), y(yPos) {}
    bool isValid() const { return x >= 0 && y >= 0; }
};

// Represents a single menu item in a context menu
struct ContextMenuItem {
    std::string label;
    std::string shortcut;  // Keyboard shortcut hint (e.g., "Ctrl+C")
    std::function<void()> callback;
    bool enabled;
    bool separator;
    std::vector<ContextMenuItem> subItems;

    // Constructor for regular menu items
    ContextMenuItem(const std::string& lbl, 
                   std::function<void()> cb, 
                   const std::string& sc = "", 
                   bool en = true)
        : label(lbl), shortcut(sc), callback(cb), enabled(en), separator(false) {}
    
    // Constructor for separators
    static ContextMenuItem Separator() {
        return ContextMenuItem("", nullptr, "", false, true);
    }

private:
    ContextMenuItem(const std::string& lbl, 
                   std::function<void()> cb, 
                   const std::string& sc,
                   bool en, 
                   bool sep)
        : label(lbl), shortcut(sc), callback(cb), enabled(en), separator(sep) {}
};

// Manages context menus for different parts of the application
class ContextMenuManager {
public:
    static ContextMenuManager& getInstance() {
        static ContextMenuManager instance;
        return instance;
    }

    // Delete copy constructor and assignment operator
    ContextMenuManager(const ContextMenuManager&) = delete;
    ContextMenuManager& operator=(const ContextMenuManager&) = delete;

    // Register context menu items for a specific context
    void registerContextMenu(const std::string& context, const std::vector<ContextMenuItem>& items);
    
    // Show context menu for the given context
    void showContextMenu(const std::string& context, const MenuPosition& pos = MenuPosition());
    
    // Show right-click context menu
    void showRightClickMenu(const std::string& context);
    
    // Check if a context has registered menu items
    bool hasContextMenu(const std::string& context) const;
    
    // Clear all registered context menus
    void clearContextMenus();

private:
    ContextMenuManager() = default;
    ~ContextMenuManager() = default;

    // Stores context menus for different contexts (e.g., "sketch", "3d_model", "panel")
    std::unordered_map<std::string, std::vector<ContextMenuItem>> m_ContextMenus;

    // Helper function to render a submenu
    void renderMenuItem(const ContextMenuItem& item);
};

} // namespace UI
} // namespace RebelCAD
