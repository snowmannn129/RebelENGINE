#include "modeling/KnotVector.h"
#include "modeling/KnotVectorImpl.h"
#include <algorithm>
#include "core/Error.h"

namespace RebelCAD {
namespace Modeling {

KnotVector::KnotVector() : impl(std::make_unique<Impl>()) {}
KnotVector::~KnotVector() = default;

KnotVector::ErrorCode KnotVector::SetKnots(const std::vector<float>& knots) {
    if (knots.empty()) {
        return ErrorCode::InvalidMesh;
    }

    // Check for valid knot values
    if (std::any_of(knots.begin(), knots.end(),
        [](float k) { return !std::isfinite(k); })) {
        return ErrorCode::InvalidMesh;
    }

    impl->knots = knots;
    impl->Clear();

    return ErrorCode::None;
}

KnotVector::ErrorCode KnotVector::Configure(const KnotVectorParams& params) {
    // Validate parameters
    if (params.tolerance <= 0.0f) {
        return ErrorCode::InvalidMesh;
    }
    if (params.min_multiplicity < 1) {
        return ErrorCode::InvalidMesh;
    }
    if (params.max_multiplicity > 0 && params.max_multiplicity < params.min_multiplicity) {
        return ErrorCode::InvalidMesh;
    }

    impl->params = params;
    impl->is_configured = true;
    impl->Clear();

    // Normalize if requested
    if (params.normalize) {
        if (!impl->Normalize()) {
            return ErrorCode::CADError;
        }
    }

    return ErrorCode::None;
}

const std::vector<float>& KnotVector::GetKnots() const {
    return impl->knots;
}

int KnotVector::GetMultiplicity(int index) const {
    return impl->GetMultiplicity(index);
}

int KnotVector::FindSpan(float t) const {
    return impl->FindSpan(t);
}

KnotVector::ErrorCode KnotVector::InsertKnot(float t, int multiplicity) {
    if (!impl->is_configured) {
        return ErrorCode::CADError;
    }

    if (multiplicity < 1) {
        return ErrorCode::InvalidMesh;
    }

    if (!std::isfinite(t)) {
        return ErrorCode::InvalidMesh;
    }

    // Check if knot is in valid range
    if (!impl->knots.empty()) {
        if (t < impl->knots.front() || t > impl->knots.back()) {
        return ErrorCode::InvalidMesh;
        }
    }

    if (!impl->InsertKnot(t, multiplicity)) {
        return ErrorCode::CADError;
    }

    return ErrorCode::None;
}

KnotVector::ErrorCode KnotVector::RemoveKnot(float t, int multiplicity) {
    if (!impl->is_configured) {
        return ErrorCode::CADError;
    }

    if (multiplicity < 1) {
        return ErrorCode::InvalidMesh;
    }

    if (!std::isfinite(t)) {
        return ErrorCode::InvalidMesh;
    }

    if (!impl->RemoveKnot(t, multiplicity)) {
        return ErrorCode::CADError;
    }

    return ErrorCode::None;
}

KnotVector::ErrorCode KnotVector::Normalize() {
    if (!impl->is_configured) {
        return ErrorCode::CADError;
    }

    if (!impl->Normalize()) {
        return ErrorCode::CADError;
    }

    return ErrorCode::None;
}

bool KnotVector::IsValid() const {
    if (!impl->is_configured) {
        return false;
    }

    return impl->ValidateProperties();
}

int KnotVector::GetUniqueCount() const {
    impl->UpdateCache();
    return impl->unique_knots.size();
}

int KnotVector::GetTotalCount() const {
    return impl->knots.size();
}

} // namespace Modeling
} // namespace RebelCAD
