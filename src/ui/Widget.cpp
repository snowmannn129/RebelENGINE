#include "ui/Widget.h"
#include "core/Log.h"
#include "ui/InputManager.h"
#include <algorithm>

namespace RebelCAD {
namespace UI {

Widget::Widget(const std::string& id)
    : m_Id(id)
    , m_Position(0, 0)
    , m_Size(0, 0)
    , m_MinSize(0, 0)
    , m_MaxSize(FLT_MAX, FLT_MAX)
    , m_Padding(0, 0)
    , m_Margin(0, 0)
    , m_LayoutInvalid(true)
    , m_Focusable(false)
    , m_Focused(false)
    , m_Visible(true)
    , m_Enabled(true)
{
    setupEventHandlers();
}

Widget::~Widget() {
    cleanupEventHandlers();
}

void Widget::update() {
    if (m_LayoutInvalid) {
        updateLayout();
    }

    // Update children
    for (auto& child : m_Children) {
        if (child) {
            child->update();
        }
    }
}

// Widget hierarchy management
void Widget::addChild(std::shared_ptr<Widget> child) {
    if (!child) return;

    // Remove from previous parent
    if (auto parent = child->getParent()) {
        parent->removeChild(child);
    }

    // Add to our children
    m_Children.push_back(child);
    child->setParent(shared_from_this());
    
    onChildAdded(child);
    invalidateLayout();
}

void Widget::removeChild(std::shared_ptr<Widget> child) {
    if (!child) return;

    auto it = std::find(m_Children.begin(), m_Children.end(), child);
    if (it != m_Children.end()) {
        (*it)->setParent(nullptr);
        m_Children.erase(it);
        onChildRemoved(child);
        invalidateLayout();
    }
}

void Widget::removeAllChildren() {
    while (!m_Children.empty()) {
        removeChild(m_Children.back());
    }
}

std::shared_ptr<Widget> Widget::getChild(size_t index) const {
    return index < m_Children.size() ? m_Children[index] : nullptr;
}

std::shared_ptr<Widget> Widget::getChildById(const std::string& id) const {
    auto it = std::find_if(m_Children.begin(), m_Children.end(),
        [&id](const auto& child) { return child && child->getId() == id; });
    return it != m_Children.end() ? *it : nullptr;
}

std::vector<std::shared_ptr<Widget>> Widget::getChildren() const {
    return m_Children;
}

size_t Widget::getChildCount() const {
    return m_Children.size();
}

void Widget::setParent(std::shared_ptr<Widget> parent) {
    if (auto oldParent = m_Parent.lock()) {
        if (oldParent != parent) {
            oldParent->removeChild(shared_from_this());
        }
    }
    m_Parent = parent;
    onParentChanged(parent);
    invalidateLayout();
}

std::shared_ptr<Widget> Widget::getParent() const {
    return m_Parent.lock();
}

bool Widget::isRoot() const {
    return !m_Parent.lock();
}

// Layout management
void Widget::setPosition(const ImVec2& position) {
    if (m_Position.x != position.x || m_Position.y != position.y) {
        m_Position = position;
        invalidateLayout();
    }
}

void Widget::setSize(const ImVec2& size) {
    if (m_Size.x != size.x || m_Size.y != size.y) {
        m_Size = size;
        onResize(size);
        invalidateLayout();
    }
}

void Widget::setMinSize(const ImVec2& minSize) {
    if (m_MinSize.x != minSize.x || m_MinSize.y != minSize.y) {
        m_MinSize = minSize;
        invalidateLayout();
    }
}

void Widget::setMaxSize(const ImVec2& maxSize) {
    if (m_MaxSize.x != maxSize.x || m_MaxSize.y != maxSize.y) {
        m_MaxSize = maxSize;
        invalidateLayout();
    }
}

void Widget::setPadding(const ImVec2& padding) {
    if (m_Padding.x != padding.x || m_Padding.y != padding.y) {
        m_Padding = padding;
        invalidateLayout();
    }
}

void Widget::setMargin(const ImVec2& margin) {
    if (m_Margin.x != margin.x || m_Margin.y != margin.y) {
        m_Margin = margin;
        invalidateLayout();
    }
}

ImVec2 Widget::getPosition() const { return m_Position; }
ImVec2 Widget::getSize() const { return m_Size; }
ImVec2 Widget::getMinSize() const { return m_MinSize; }
ImVec2 Widget::getMaxSize() const { return m_MaxSize; }
ImVec2 Widget::getPadding() const { return m_Padding; }
ImVec2 Widget::getMargin() const { return m_Margin; }

ImVec2 Widget::calculatePreferredSize() const {
    // Base implementation just returns current size
    // Derived classes should override this
    return m_Size;
}

// Style management
void Widget::setStyle(const nlohmann::json& style) {
    m_Style = style;
    onStyleChanged();
    invalidateLayout();
}

nlohmann::json Widget::getStyle() const {
    return m_Style;
}

void Widget::resetStyle() {
    m_Style = nlohmann::json::object();
    onStyleChanged();
    invalidateLayout();
}

void Widget::pushStyle() {
    auto& styleManager = StyleManager::getInstance();
    
    // Apply widget-specific styles
    if (!m_Style.empty()) {
        styleManager.pushStyleVar(ImGuiStyleVar_WindowPadding, 
            ImVec2(m_Style.value("paddingX", 0.0f), m_Style.value("paddingY", 0.0f)));
        // ... apply other style variables
    }
}

void Widget::popStyle() {
    auto& styleManager = StyleManager::getInstance();
    
    // Pop all pushed styles
    if (!m_Style.empty()) {
        styleManager.popStyleVar();
        // ... pop other style variables
    }
}

// Event handling
void Widget::addEventListener(const std::string& event, EventCallback callback) {
    m_EventListeners[event].push_back(callback);
}

void Widget::removeEventListener(const std::string& event) {
    m_EventListeners.erase(event);
}

void Widget::triggerEvent(const std::string& event) {
    auto it = m_EventListeners.find(event);
    if (it != m_EventListeners.end()) {
        for (const auto& callback : it->second) {
            callback(*this);
        }
    }
    propagateEvent(event);
}

// State management
void Widget::setState(const std::string& key, const nlohmann::json& value) {
    m_State[key] = value;
    onStateChanged(key);
}

nlohmann::json Widget::getState(const std::string& key) const {
    auto it = m_State.find(key);
    return it != m_State.end() ? it->second : nlohmann::json();
}

void Widget::clearState(const std::string& key) {
    m_State.erase(key);
    onStateChanged(key);
}

bool Widget::hasState(const std::string& key) const {
    return m_State.find(key) != m_State.end();
}

// Focus management
void Widget::setFocusable(bool focusable) {
    m_Focusable = focusable;
}

bool Widget::isFocusable() const {
    return m_Focusable;
}

void Widget::focus() {
    if (m_Focusable && !m_Focused) {
        m_Focused = true;
        onFocus();
    }
}

void Widget::blur() {
    if (m_Focused) {
        m_Focused = false;
        onBlur();
    }
}

bool Widget::isFocused() const {
    return m_Focused;
}

// Visibility
void Widget::setVisible(bool visible) {
    if (m_Visible != visible) {
        m_Visible = visible;
        invalidateLayout();
    }
}

bool Widget::isVisible() const {
    return m_Visible;
}

// Enable/disable
void Widget::setEnabled(bool enabled) {
    if (m_Enabled != enabled) {
        m_Enabled = enabled;
        if (!enabled && m_Focused) {
            blur();
        }
    }
}

bool Widget::isEnabled() const {
    return m_Enabled;
}

// Accessors
const std::string& Widget::getId() const {
    return m_Id;
}

void Widget::setId(const std::string& id) {
    m_Id = id;
}

// Protected methods
void Widget::onMouseEnter() {
    triggerEvent("mouseenter");
}

void Widget::onMouseLeave() {
    triggerEvent("mouseleave");
}

void Widget::onMouseMove(const ImVec2& position) {
    triggerEvent("mousemove");
}

void Widget::onMouseDown(int button) {
    triggerEvent("mousedown");
}

void Widget::onMouseUp(int button) {
    triggerEvent("mouseup");
}

void Widget::onClick() {
    triggerEvent("click");
}

void Widget::onDoubleClick() {
    triggerEvent("dblclick");
}

void Widget::onFocus() {
    triggerEvent("focus");
}

void Widget::onBlur() {
    triggerEvent("blur");
}

void Widget::onKeyDown(int key, int mods) {
    triggerEvent("keydown");
}

void Widget::onKeyUp(int key, int mods) {
    triggerEvent("keyup");
}

void Widget::onChar(unsigned int c) {
    triggerEvent("char");
}

void Widget::onScroll(const ImVec2& offset) {
    triggerEvent("scroll");
}

void Widget::onResize(const ImVec2& size) {
    triggerEvent("resize");
}

void Widget::onStyleChanged() {
    triggerEvent("stylechanged");
}

void Widget::onStateChanged(const std::string& key) {
    triggerEvent("statechanged");
}

void Widget::onChildAdded(std::shared_ptr<Widget> child) {
    triggerEvent("childadded");
}

void Widget::onChildRemoved(std::shared_ptr<Widget> child) {
    triggerEvent("childremoved");
}

void Widget::onParentChanged(std::shared_ptr<Widget> parent) {
    triggerEvent("parentchanged");
}

void Widget::updateLayout() {
    // Base implementation just updates children
    for (auto& child : m_Children) {
        if (child) {
            child->updateLayout();
        }
    }
    m_LayoutInvalid = false;
}

void Widget::propagateEvent(const std::string& event) {
    // Propagate event to parent (bubbling phase)
    if (auto parent = getParent()) {
        parent->triggerEvent(event);
    }
}

bool Widget::isPointInside(const ImVec2& point) const {
    return point.x >= m_Position.x && point.x <= m_Position.x + m_Size.x &&
           point.y >= m_Position.y && point.y <= m_Position.y + m_Size.y;
}

void Widget::invalidateLayout() {
    m_LayoutInvalid = true;
    
    // Invalidate parent layout as well
    if (auto parent = getParent()) {
        parent->invalidateLayout();
    }
}

void Widget::setupEventHandlers() {
    auto& eventBus = Core::EventBus::getInstance();
    
    // Subscribe to input events
    m_InputEventSubscription = eventBus.subscribe<InputManager::InputEvent>(
        [this](const InputManager::InputEvent& event, const Core::EventMetadata&) {
            if (!m_Visible || !m_Enabled) return;

            switch (event.type) {
                case InputManager::InputEventType::MouseMove:
                    if (isPointInside(event.mousePos)) {
                        onMouseMove(event.mousePos);
                    }
                    break;
                case InputManager::InputEventType::MouseButton:
                    if (isPointInside(event.mousePos)) {
                        if (event.mouseButton.action == GLFW_PRESS) {
                            onMouseDown(event.mouseButton.button);
                        } else if (event.mouseButton.action == GLFW_RELEASE) {
                            onMouseUp(event.mouseButton.button);
                        }
                    }
                    break;
                case InputManager::InputEventType::KeyPress:
                    if (m_Focused) {
                        onKeyDown(event.key.code, event.key.mods);
                    }
                    break;
                case InputManager::InputEventType::KeyRelease:
                    if (m_Focused) {
                        onKeyUp(event.key.code, event.key.mods);
                    }
                    break;
                case InputManager::InputEventType::Char:
                    if (m_Focused) {
                        onChar(event.charCode);
                    }
                    break;
                case InputManager::InputEventType::Scroll:
                    if (isPointInside(event.mousePos)) {
                        onScroll(event.scrollOffset);
                    }
                    break;
            }
        }
    );

    // Subscribe to style changes
    m_StyleChangedSubscription = eventBus.subscribe<StyleManager::StyleChangedEvent>(
        [this](const StyleManager::StyleChangedEvent& event, const Core::EventMetadata&) {
            if (event.type == StyleManager::StyleChangeType::ThemeChanged ||
                event.type == StyleManager::StyleChangeType::WidgetStyleChanged) {
                onStyleChanged();
            }
        }
    );
}

void Widget::cleanupEventHandlers() {
    auto& eventBus = Core::EventBus::getInstance();
    eventBus.unsubscribe(m_InputEventSubscription);
    eventBus.unsubscribe(m_StyleChangedSubscription);
}

} // namespace UI
} // namespace RebelCAD
