#pragma once

#include <vector>
#include <memory>
#include "core/Error.h"

namespace RebelCAD {
namespace Modeling {

/**
 * @brief Parameters for configuring a knot vector
 */
struct KnotVectorParams {
    bool uniform = false;                ///< Whether knots are uniformly spaced
    float tolerance = 1e-6f;             ///< Tolerance for knot operations
    bool normalize = true;               ///< Normalize knot vector to [0,1]
    int min_multiplicity = 1;            ///< Minimum knot multiplicity
    int max_multiplicity = 0;            ///< Maximum knot multiplicity (0 = no limit)
};

/**
 * @class KnotVector
 * @brief Class for managing NURBS knot vectors
 * 
 * The KnotVector class provides functionality for creating and
 * manipulating knot vectors used in NURBS curves and surfaces.
 * It handles knot insertion, removal, and validation, while
 * maintaining proper multiplicity and continuity properties.
 */
class KnotVector {
public:
    using ErrorCode = RebelCAD::Core::ErrorCode;

    KnotVector();
    ~KnotVector();

    /**
     * @brief Set knot values
     * @param knots Vector of knot values
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode SetKnots(const std::vector<float>& knots);

    /**
     * @brief Configure knot vector parameters
     * @param params KnotVectorParams struct containing settings
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode Configure(const KnotVectorParams& params);

    /**
     * @brief Get knot vector values
     * @return Vector of knot values
     */
    const std::vector<float>& GetKnots() const;

    /**
     * @brief Get knot multiplicity at index
     * @param index Knot index
     * @return Multiplicity of knot
     */
    int GetMultiplicity(int index) const;

    /**
     * @brief Find span containing parameter value
     * @param t Parameter value
     * @return Index of knot span
     */
    int FindSpan(float t) const;

    /**
     * @brief Insert knot value
     * @param t Parameter value for new knot
     * @param multiplicity Multiplicity of new knot
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode InsertKnot(float t, int multiplicity = 1);

    /**
     * @brief Remove knot value
     * @param t Parameter value of knot to remove
     * @param multiplicity Number of times to remove knot
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode RemoveKnot(float t, int multiplicity = 1);

    /**
     * @brief Normalize knot vector to range [0,1]
     * @return ErrorCode::None on success, appropriate error code otherwise
     */
    ErrorCode Normalize();

    /**
     * @brief Check if knot vector is valid
     * @return true if valid, false otherwise
     */
    bool IsValid() const;

    /**
     * @brief Get number of unique knots
     * @return Count of unique knot values
     */
    int GetUniqueCount() const;

    /**
     * @brief Get total number of knots including multiplicities
     * @return Total knot count
     */
    int GetTotalCount() const;

protected:
    class Impl;
    std::unique_ptr<Impl> impl;

    /**
     * @brief Validate knot vector properties
     * @return true if properties are valid
     */
    bool ValidateProperties() const;

    /**
     * @brief Check if knot insertion is valid
     * @param t Parameter value for new knot
     * @param multiplicity Desired multiplicity
     * @return true if insertion is valid
     */
    bool ValidateInsertion(float t, int multiplicity) const;

    /**
     * @brief Check if knot removal is valid
     * @param t Parameter value of knot to remove
     * @param multiplicity Number of times to remove
     * @return true if removal is valid
     */
    bool ValidateRemoval(float t, int multiplicity) const;

private:
    /**
     * @brief Find index of knot value
     * @param t Parameter value to find
     * @return Index of knot, or -1 if not found
     */
    int FindKnotIndex(float t) const;

    /**
     * @brief Update internal data after knot changes
     */
    void UpdateInternalData();

    /**
     * @brief Sort and merge duplicate knots
     */
    void CleanupKnots();
};

} // namespace Modeling
} // namespace RebelCAD
