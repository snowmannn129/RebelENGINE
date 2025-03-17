#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>
#include "modeling/KnotVector.h"

namespace RebelCAD {
namespace Modeling {

class KnotVector::Impl {
public:
    // Input data
    std::vector<float> knots;
    KnotVectorParams params;
    bool is_configured = false;

    // Cached data
    struct KnotData {
        float value;                     // Knot parameter value
        int multiplicity;                // Knot multiplicity
        int start_index;                 // Start index in knot vector
        int end_index;                   // End index in knot vector
    };
    mutable std::vector<KnotData> unique_knots;  // Unique knots with metadata
    mutable bool cache_valid = false;            // Whether cached data is valid

    Impl() = default;

    /**
     * @brief Clear all cached data
     */
    void Clear() {
        unique_knots.clear();
        cache_valid = false;
    }

    /**
     * @brief Update cached knot data
     */
    void UpdateCache() const {
        if (cache_valid) {
            return;
        }

        std::vector<KnotData> temp_knots;
        if (!knots.empty()) {
            // Sort knots if not already sorted
            std::vector<float> sorted_knots = knots;
            if (!std::is_sorted(sorted_knots.begin(), sorted_knots.end())) {
                std::sort(sorted_knots.begin(), sorted_knots.end());
            }

            // Build unique knots with multiplicities
            float current_value = sorted_knots[0];
            int multiplicity = 1;
            int start_index = 0;

            for (size_t i = 1; i < sorted_knots.size(); ++i) {
                if (std::abs(sorted_knots[i] - current_value) <= params.tolerance) {
                    // Same knot value
                    multiplicity++;
                } else {
                    // New knot value
                    temp_knots.push_back({
                        current_value,
                        multiplicity,
                        start_index,
                        static_cast<int>(i - 1)
                    });
                    current_value = sorted_knots[i];
                    multiplicity = 1;
                    start_index = i;
                }
            }

            // Add final knot
            temp_knots.push_back({
                current_value,
                multiplicity,
                start_index,
                static_cast<int>(sorted_knots.size() - 1)
            });
        }

        unique_knots = std::move(temp_knots);
        cache_valid = true;
    }

    /**
     * @brief Find knot span containing parameter value
     * @param t Parameter value
     * @return Index of knot span, or -1 if not found
     */
    int FindSpan(float t) const {
        if (knots.empty()) {
            return -1;
        }

        // Handle boundary cases
        if (t <= knots.front()) {
            return 0;
        }
        if (t >= knots.back()) {
            return knots.size() - 2;
        }

        // Binary search for knot span
        auto it = std::upper_bound(knots.begin(), knots.end(), t);
        return std::distance(knots.begin(), it) - 1;
    }

    /**
     * @brief Get multiplicity of knot at index
     * @param index Knot index
     * @return Multiplicity of knot
     */
    int GetMultiplicity(int index) const {
        if (index < 0 || index >= static_cast<int>(knots.size())) {
            return 0;
        }

        UpdateCache();
        float value = knots[index];
        
        // Find knot in unique knots
        auto it = std::find_if(unique_knots.begin(), unique_knots.end(),
            [value, this](const KnotData& data) {
                return std::abs(data.value - value) <= params.tolerance;
            });

        return it != unique_knots.end() ? it->multiplicity : 0;
    }

    /**
     * @brief Insert knot value
     * @param t Parameter value
     * @param multiplicity Desired multiplicity
     * @return true if insertion successful
     */
    bool InsertKnot(float t, int multiplicity) {
        if (multiplicity <= 0) {
            return false;
        }

        // Find insertion point
        auto it = std::lower_bound(knots.begin(), knots.end(), t);
        size_t index = std::distance(knots.begin(), it);

        // Check if knot already exists
        bool knot_exists = (it != knots.end() && std::abs(*it - t) <= params.tolerance);
        
        if (knot_exists) {
            // Update existing knot multiplicity
            int current_mult = GetMultiplicity(index);
            int additional_mult = std::min(
                multiplicity,
                params.max_multiplicity > 0 ? 
                    params.max_multiplicity - current_mult : 
                    multiplicity
            );

            if (additional_mult <= 0) {
                return false;
            }

            // Insert additional knots
            knots.insert(it, additional_mult, t);
        } else {
            // Insert new knot
            knots.insert(it, multiplicity, t);
        }

        cache_valid = false;
        return true;
    }

    /**
     * @brief Remove knot value
     * @param t Parameter value
     * @param multiplicity Number of times to remove
     * @return true if removal successful
     */
    bool RemoveKnot(float t, int multiplicity) {
        if (multiplicity <= 0) {
            return false;
        }

        // Find knot in vector
        auto it = std::find_if(knots.begin(), knots.end(),
            [t, this](float value) {
                return std::abs(value - t) <= params.tolerance;
            });

        if (it == knots.end()) {
            return false;
        }

        // Count current multiplicity
        auto range_end = std::find_if(it, knots.end(),
            [t, this](float value) {
                return std::abs(value - t) > params.tolerance;
            });
        int current_mult = std::distance(it, range_end);

        // Check if removal would violate minimum multiplicity
        int remaining_mult = current_mult - multiplicity;
        if (remaining_mult < params.min_multiplicity) {
            return false;
        }

        // Remove knots
        knots.erase(it, it + std::min(multiplicity, current_mult));
        cache_valid = false;
        return true;
    }

    /**
     * @brief Normalize knot vector to range [0,1]
     * @return true if normalization successful
     */
    bool Normalize() {
        if (knots.empty()) {
            return false;
        }

        float min_val = knots.front();
        float max_val = knots.back();
        float range = max_val - min_val;

        if (std::abs(range) <= params.tolerance) {
            return false;
        }

        // Normalize each knot
        for (float& knot : knots) {
            knot = (knot - min_val) / range;
        }

        cache_valid = false;
        return true;
    }

    /**
     * @brief Validate knot vector properties
     * @return true if properties are valid
     */
    bool ValidateProperties() const {
        if (knots.empty()) {
            return false;
        }

        UpdateCache();

        // Check knot ordering
        if (!std::is_sorted(knots.begin(), knots.end())) {
            return false;
        }

        // Check multiplicities
        for (const auto& knot : unique_knots) {
            if (knot.multiplicity < params.min_multiplicity) {
                return false;
            }
            if (params.max_multiplicity > 0 && knot.multiplicity > params.max_multiplicity) {
                return false;
            }
        }

        return true;
    }
};

} // namespace Modeling
} // namespace RebelCAD
