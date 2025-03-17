#pragma once

namespace RebelCAD {
namespace Graphics {

/**
 * @struct Color
 * @brief Represents an RGBA color value
 */
struct Color {
    float r; ///< Red component (0.0 - 1.0)
    float g; ///< Green component (0.0 - 1.0)
    float b; ///< Blue component (0.0 - 1.0)
    float a; ///< Alpha component (0.0 - 1.0)

    /**
     * @brief Constructs a color with specified RGBA values
     * @param red Red component (0.0 - 1.0)
     * @param green Green component (0.0 - 1.0)
     * @param blue Blue component (0.0 - 1.0)
     * @param alpha Alpha component (0.0 - 1.0)
     */
    Color(float red = 0.0f, float green = 0.0f, float blue = 0.0f, float alpha = 1.0f)
        : r(red), g(green), b(blue), a(alpha) {}

    // Predefined colors
    static const Color Black;   ///< Black color (0, 0, 0)
    static const Color White;   ///< White color (1, 1, 1)
    static const Color Red;     ///< Red color (1, 0, 0)
    static const Color Green;   ///< Green color (0, 1, 0)
    static const Color Blue;    ///< Blue color (0, 0, 1)
    static const Color Yellow;  ///< Yellow color (1, 1, 0)
    static const Color Magenta; ///< Magenta color (1, 0, 1)
    static const Color Cyan;    ///< Cyan color (0, 1, 1)
    static const Color Gray;    ///< Gray color (0.5, 0.5, 0.5)
};

} // namespace Graphics
} // namespace RebelCAD
