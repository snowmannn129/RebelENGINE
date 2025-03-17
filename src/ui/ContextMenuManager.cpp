#include "ui/ContextMenuManager.h"
#include "imgui.h"
#include "core/Log.h"

namespace RebelCAD {
namespace UI {

void ContextMenuManager::registerContextMenu(const std::string& context, const std::vector<ContextMenuItem>& items) {
    m_ContextMenus[context] = items;
    LOG_INFO("Registered context menu for context: {}", context);
}

void ContextMenuManager::showContextMenu(const std::string& context, const MenuPosition& pos) {
    auto it = m_ContextMenus.find(context);
    if (it == m_ContextMenus.end()) {
        LOG_WARNING("No context menu registered for context: {}", context);
        return;
    }

    // If position is specified, set the next window position
    if (pos.isValid()) {
        ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y));
    }

    // Open a popup at the specified position or default location
    if (!ImGui::IsPopupOpen(context.c_str())) {
        ImGui::OpenPopup(context.c_str());
    }

    if (ImGui::BeginPopup(context.c_str())) {
        for (const auto& item : it->second) {
            renderMenuItem(item);
        }
        ImGui::EndPopup();
    }
}

void ContextMenuManager::showRightClickMenu(const std::string& context) {
    auto it = m_ContextMenus.find(context);
    if (it == m_ContextMenus.end()) {
        LOG_WARNING("No context menu registered for context: {}", context);
        return;
    }

    if (ImGui::BeginPopupContextWindow(context.c_str())) {
        for (const auto& item : it->second) {
            renderMenuItem(item);
        }
        ImGui::EndPopup();
    }
}

bool ContextMenuManager::hasContextMenu(const std::string& context) const {
    return m_ContextMenus.find(context) != m_ContextMenus.end();
}

void ContextMenuManager::clearContextMenus() {
    m_ContextMenus.clear();
    LOG_INFO("Cleared all context menus");
}

void ContextMenuManager::renderMenuItem(const ContextMenuItem& item) {
    if (item.separator) {
        ImGui::Separator();
        return;
    }

    if (!item.subItems.empty()) {
        if (ImGui::BeginMenu(item.label.c_str(), item.enabled)) {
            for (const auto& subItem : item.subItems) {
                renderMenuItem(subItem);
            }
            ImGui::EndMenu();
        }
    } else {
        if (ImGui::MenuItem(item.label.c_str(), item.shortcut.c_str(), false, item.enabled)) {
            if (item.callback) {
                item.callback();
            }
        }
    }
}

} // namespace UI
} // namespace RebelCAD
