#pragma once

#include <memory>
#include <string>
#include <vector>
#include <algorithm>

namespace RebelCAD {
namespace Core {

/**
 * @brief Base class for all components in the RebelCAD system
 * 
 * The Component class provides core functionality for:
 * - Hierarchical scene structure (parent-child relationships)
 * - Lifecycle management
 * - Smart pointer integration
 * - Component identification and type information
 */
class Component : public std::enable_shared_from_this<Component> {
public:
    using Ptr = std::shared_ptr<Component>;
    using WeakPtr = std::weak_ptr<Component>;

    /**
     * @brief Creates a new component instance
     * @param name The name of the component
     * @return A shared pointer to the new component
     */
    static Ptr create(const std::string& name = "Component") {
        return std::shared_ptr<Component>(new Component(name));
    }

    virtual ~Component() = default;

    /**
     * @brief Gets the name of the component
     * @return The component name
     */
    const std::string& getName() const { return name_; }

    /**
     * @brief Sets the name of the component
     * @param name The new name
     */
    void setName(const std::string& name) { name_ = name; }

    /**
     * @brief Gets the parent component
     * @return Weak pointer to parent component
     */
    WeakPtr getParent() const { return parent_; }

    /**
     * @brief Gets all child components
     * @return Vector of shared pointers to child components
     */
    const std::vector<Ptr>& getChildren() const { return children_; }

    /**
     * @brief Adds a child component
     * @param child The component to add as a child
     * @return True if successfully added, false if already a child
     */
    bool addChild(const Ptr& child) {
        if (!child || child.get() == this) return false;

        // Check if already a child
        auto it = std::find(children_.begin(), children_.end(), child);
        if (it != children_.end()) return false;

        // Remove from previous parent
        if (auto oldParent = child->getParent().lock()) {
            oldParent->removeChild(child);
        }

        // Set new parent-child relationship
        children_.push_back(child);
        child->parent_ = shared_from_this();
        
        return true;
    }

    /**
     * @brief Removes a child component
     * @param child The component to remove
     * @return True if successfully removed, false if not a child
     */
    bool removeChild(const Ptr& child) {
        if (!child) return false;

        auto it = std::find(children_.begin(), children_.end(), child);
        if (it == children_.end()) return false;

        child->parent_.reset();
        children_.erase(it);
        
        return true;
    }

    /**
     * @brief Removes this component from its parent
     * @return True if successfully removed, false if no parent
     */
    bool removeFromParent() {
        if (auto parent = parent_.lock()) {
            return parent->removeChild(shared_from_this());
        }
        return false;
    }

protected:
    explicit Component(const std::string& name) : name_(name) {}

public:
    /**
     * @brief Lifecycle method called when component is initialized
     */
    virtual void onInitialize() {}

    /**
     * @brief Lifecycle method called when component is being destroyed
     */
    virtual void onDestroy() {}

    /**
     * @brief Lifecycle method called each frame for updates
     * @param deltaTime Time elapsed since last update
     */
    virtual void onUpdate(float deltaTime) {}

private:
    std::string name_;
    WeakPtr parent_;
    std::vector<Ptr> children_;
};

} // namespace Core
} // namespace RebelCAD
