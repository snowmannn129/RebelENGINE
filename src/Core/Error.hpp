#pragma once

#include <string>
#include <stdexcept>

namespace RebelCAD {

/**
 * Custom error class for RebelCAD-specific exceptions.
 * Provides detailed error information and categorization.
 */
class Error : public std::runtime_error {
public:
    /**
     * Error categories for different types of failures
     */
    enum class Category {
        InvalidArgument,    // Invalid input parameters
        OutOfRange,        // Index/parameter out of valid range
        InvalidOperation,  // Operation not valid in current state
        ResourceError,     // Resource allocation/access failure
        InternalError     // Unexpected internal error
    };

    /**
     * Creates a new Error with the specified message and category.
     * 
     * @param message Detailed error description
     * @param category Error category for classification
     */
    Error(const std::string& message, Category category)
        : std::runtime_error(message)
        , m_category(category)
    {}

    /**
     * Gets the error category.
     * 
     * @return Category classification of the error
     */
    Category GetCategory() const { return m_category; }

    /**
     * Creates an InvalidArgument error.
     * 
     * @param message Error description
     * @return Error instance
     */
    static Error InvalidArgument(const std::string& message) {
        return Error(message, Category::InvalidArgument);
    }

    /**
     * Creates an OutOfRange error.
     * 
     * @param message Error description
     * @return Error instance
     */
    static Error OutOfRange(const std::string& message) {
        return Error(message, Category::OutOfRange);
    }

    /**
     * Creates an InvalidOperation error.
     * 
     * @param message Error description
     * @return Error instance
     */
    static Error InvalidOperation(const std::string& message) {
        return Error(message, Category::InvalidOperation);
    }

    /**
     * Creates a ResourceError.
     * 
     * @param message Error description
     * @return Error instance
     */
    static Error ResourceError(const std::string& message) {
        return Error(message, Category::ResourceError);
    }

    /**
     * Creates an InternalError.
     * 
     * @param message Error description
     * @return Error instance
     */
    static Error InternalError(const std::string& message) {
        return Error(message, Category::InternalError);
    }

private:
    Category m_category;
};

} // namespace RebelCAD
