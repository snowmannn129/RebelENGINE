#include "modeling/SubdivisionSurface.hpp"
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <algorithm>

namespace RebelCAD {
namespace Modeling {


SubdivisionSurface::SubdivisionSurface(std::shared_ptr<HalfEdgeMesh> baseMesh)
    : baseMesh_(baseMesh), currentLevel_(0) {
    if (!baseMesh) {
        throw RebelCAD::Error::InvalidArgument("Base mesh cannot be null");
    }
}

std::shared_ptr<HalfEdgeMesh> SubdivisionSurface::subdivide(int levels) {
    if (levels < 0) {
        throw RebelCAD::Error::InvalidArgument("Subdivision levels cannot be negative");
    }

    auto result = baseMesh_;
    for (int i = 0; i < levels; ++i) {
        result = subdivide();
    }
    return result;
}

std::shared_ptr<HalfEdgeMesh> SubdivisionSurface::subdivide() {
    auto newMesh = std::make_shared<HalfEdgeMesh>();
    
    // Maps to store computed points
    std::unordered_map<HalfEdgeMesh::EdgePtr, HalfEdgeMesh::VertexPtr> edgePoints;
    std::unordered_map<HalfEdgeMesh::VertexPtr, HalfEdgeMesh::VertexPtr> vertexPoints;
    std::unordered_map<HalfEdgeMesh::FacePtr, HalfEdgeMesh::VertexPtr> facePoints;

    // Step 1: Compute face points
    for (const auto& face : baseMesh_->getFaces()) {
        auto facePoint = computeFacePoint(face);
        auto newVertex = newMesh->createVertex(facePoint.position, facePoint.texCoord);
        facePoints[face] = newVertex;
    }

    // Step 2: Compute edge points
    for (const auto& edge : baseMesh_->getEdges()) {
        auto edgePoint = computeEdgePoint(edge);
        // Handle sharp edges
        edgePoint.position = handleSharpEdge(edge, edgePoint.position);
        auto newVertex = newMesh->createVertex(edgePoint.position, edgePoint.texCoord);
        edgePoints[edge] = newVertex;
    }

    // Step 3: Compute vertex points
    for (const auto& vertex : baseMesh_->getVertices()) {
        auto vertexPoint = computeVertexPoint(vertex);
        // Handle sharp vertices
        vertexPoint.position = handleSharpVertex(vertex, vertexPoint.position);
        auto newVertex = newMesh->createVertex(vertexPoint.position, vertexPoint.texCoord);
        vertexPoints[vertex] = newVertex;
    }

    // Step 4: Create new faces
    for (const auto& face : baseMesh_->getFaces()) {
        createSubdividedFaceTopology(newMesh, face, facePoints[face], edgePoints, vertexPoints);
    }

    // Update subdivision level
    currentLevel_++;
    
    // Validate and return new mesh
    if (!newMesh->validateTopology()) {
        throw RebelCAD::Error::InvalidOperation("Subdivision resulted in invalid topology");
    }
    
    return newMesh;
}

SubdivisionSurface::SubdivisionPoint SubdivisionSurface::computeFacePoint(const HalfEdgeMesh::FacePtr& face) const {
    // Face point is average of all face vertices
    glm::vec3 posSum(0.0f);
    glm::vec2 uvSum(0.0f);
    int count = 0;
    
    auto start = face->halfEdge;
    auto current = start;
    do {
        posSum += current->vertex->position;
        uvSum += current->vertex->texCoord;
        count++;
        current = current->next;
    } while (current != start);
    
    return {
        posSum / static_cast<float>(count),
        uvSum / static_cast<float>(count)
    };
}

SubdivisionSurface::SubdivisionPoint SubdivisionSurface::computeEdgePoint(const HalfEdgeMesh::EdgePtr& edge) const {
    auto he = edge->halfEdge;
    auto pair = he->pair;
    
    // For boundary edges
    if (!pair) {
        return {
            (he->vertex->position + he->prev->vertex->position) * 0.5f,
            (he->vertex->texCoord + he->prev->vertex->texCoord) * 0.5f
        };
    }
    
    // Regular edge point: average of edge endpoints and adjacent face points
    auto v1Point = he->vertex;
    auto v2Point = he->prev->vertex;
    auto f1Point = computeFacePoint(he->face);
    auto f2Point = computeFacePoint(pair->face);
    
    return {
        (v1Point->position + v2Point->position + f1Point.position + f2Point.position) * 0.25f,
        (v1Point->texCoord + v2Point->texCoord + f1Point.texCoord + f2Point.texCoord) * 0.25f
    };
}

SubdivisionSurface::SubdivisionPoint SubdivisionSurface::computeVertexPoint(const HalfEdgeMesh::VertexPtr& vertex) const {
    auto start = vertex->outgoingHalfEdge;
    auto current = start;
    
    // For boundary vertices
    if (!current->pair) {
        // Use boundary rules
        return {vertex->position, vertex->texCoord};
    }
    
    // Count incident edges and collect surrounding points
    int n = 0;
    SubdivisionPoint Q{glm::vec3(0.0f), glm::vec2(0.0f)};  // Average of face points
    SubdivisionPoint R{glm::vec3(0.0f), glm::vec2(0.0f)};  // Average of edge midpoints
    
    do {
        // Add face point
        auto facePoint = computeFacePoint(current->face);
        Q.position += facePoint.position;
        Q.texCoord += facePoint.texCoord;
        
        // Add edge midpoint
        auto v1 = current->vertex;
        auto v2 = current->prev->vertex;
        R.position += (v1->position + v2->position) * 0.5f;
        R.texCoord += (v1->texCoord + v2->texCoord) * 0.5f;
        
        n++;
        current = current->pair->next;
    } while (current != start);
    
    float n_f = static_cast<float>(n);
    Q.position /= n_f;
    Q.texCoord /= n_f;
    R.position /= n_f;
    R.texCoord /= n_f;
    
    // Apply Catmull-Clark vertex point formula
    return {
        (Q.position + R.position * 2.0f + vertex->position * (n_f - 3.0f)) / n_f,
        (Q.texCoord + R.texCoord * 2.0f + vertex->texCoord * (n_f - 3.0f)) / n_f
    };
}

glm::vec3 SubdivisionSurface::handleSharpEdge(
    const HalfEdgeMesh::EdgePtr& edge,
    const glm::vec3& regularEdgePoint) const {
    if (!edge->isSharp) {
        return regularEdgePoint;
    }
    
    // For fully sharp edges, use simple linear interpolation
    if (edge->sharpness >= 1.0f) {
        auto he = edge->halfEdge;
        return (he->vertex->position + he->prev->vertex->position) * 0.5f;
    }
    
    // For semi-sharp edges, blend between sharp and smooth positions
    auto he = edge->halfEdge;
    glm::vec3 sharpPos = (he->vertex->position + he->prev->vertex->position) * 0.5f;
    return glm::mix(regularEdgePoint, sharpPos, edge->sharpness);
}

glm::vec3 SubdivisionSurface::handleSharpVertex(
    const HalfEdgeMesh::VertexPtr& vertex,
    const glm::vec3& regularVertexPoint) const {
    if (!vertex->isSharp) {
        return regularVertexPoint;
    }
    
    // Sharp vertices maintain their position
    return vertex->position;
}

void SubdivisionSurface::createSubdividedFaceTopology(
    std::shared_ptr<HalfEdgeMesh> newMesh,
    const HalfEdgeMesh::FacePtr& oldFace,
    const HalfEdgeMesh::VertexPtr& facePoint,
    const std::unordered_map<HalfEdgeMesh::EdgePtr, HalfEdgeMesh::VertexPtr>& edgePoints,
    const std::unordered_map<HalfEdgeMesh::VertexPtr, HalfEdgeMesh::VertexPtr>& vertexPoints) {
    
    auto start = oldFace->halfEdge;
    auto current = start;
    
    do {
        // Create new quad face for each edge of the original face
        std::vector<HalfEdgeMesh::VertexPtr> quadVertices;
        
        // Add vertices in CCW order
        quadVertices.push_back(vertexPoints.at(current->vertex));
        quadVertices.push_back(edgePoints.at(current->edge));
        quadVertices.push_back(facePoint);
        quadVertices.push_back(edgePoints.at(current->prev->edge));
        
        // Create the new face
        newMesh->createFace(quadVertices);
        
        current = current->next;
    } while (current != start);
}

std::shared_ptr<SolidBody> SubdivisionSurface::toSolidBody() const {
    std::vector<SolidBody::Vertex> vertices;
    std::vector<SolidBody::Triangle> triangles;
    
    // Maps to track vertex indices
    std::unordered_map<HalfEdgeMesh::VertexPtr, size_t> vertexIndices;
    
    // Convert vertices
    for (const auto& vertex : baseMesh_->getVertices()) {
        // Calculate vertex normal by averaging face normals
        glm::vec3 normal(0.0f);
        int faceCount = 0;
        
        auto start = vertex->outgoingHalfEdge;
        auto current = start;
        do {
            // Calculate face normal
            auto face = current->face;
            auto he = face->halfEdge;
            auto v1 = he->vertex->position;
            auto v2 = he->next->vertex->position;
            auto v3 = he->next->next->vertex->position;
            auto faceNormal = glm::normalize(glm::cross(v2 - v1, v3 - v1));
            
            normal += faceNormal;
            faceCount++;
            
            current = current->pair->next;
        } while (current != start);
        
        normal = glm::normalize(normal / static_cast<float>(faceCount));
        
        // Store vertex with computed normal and texture coordinates
        vertexIndices[vertex] = vertices.size();
        vertices.emplace_back(
            glm::dvec3(vertex->position),
            glm::dvec3(normal),
            glm::dvec2(vertex->texCoord)
        );
    }
    
    // Convert faces to triangles
    for (const auto& face : baseMesh_->getFaces()) {
        // Count vertices in face
        int vertCount = 0;
        std::vector<size_t> faceIndices;
        
        auto start = face->halfEdge;
        auto current = start;
        do {
            faceIndices.push_back(vertexIndices[current->vertex]);
            vertCount++;
            current = current->next;
        } while (current != start);
        
        // Triangulate face
        for (int i = 1; i < vertCount - 1; i++) {
            triangles.emplace_back(
                faceIndices[0],
                faceIndices[i],
                faceIndices[i + 1]
            );
        }
    }
    
    return SolidBody::Create(vertices, triangles);
}

void SubdivisionSurface::fixTJunctions() {
    if (!baseMesh_) {
        throw Error::InvalidOperation("Base mesh is null");
    }

    // Structure to store potential T-junction points
    struct TJunction {
        HalfEdgeMesh::EdgePtr edge;
        HalfEdgeMesh::VertexPtr vertex;
        float t; // Parameter along edge (0-1)
    };
    std::vector<TJunction> tJunctions;

    // Find T-junctions
    for (const auto& vertex : baseMesh_->getVertices()) {
        for (const auto& edge : baseMesh_->getEdges()) {
            // Skip if vertex is already part of the edge
            if (edge->halfEdge->vertex == vertex ||
                edge->halfEdge->prev->vertex == vertex) {
                continue;
            }

            // Check if vertex lies on edge
            auto v1 = edge->halfEdge->vertex->position;
            auto v2 = edge->halfEdge->prev->vertex->position;
            auto edgeVec = v2 - v1;
            auto length = glm::length(edgeVec);
            
            if (length < 1e-6f) continue; // Skip degenerate edges
            
            auto dir = edgeVec / length;
            auto toVertex = vertex->position - v1;
            
            // Project vertex onto edge line
            float t = glm::dot(toVertex, dir);
            if (t < 0.0f || t > length) continue; // Vertex not between endpoints
            
            // Check if vertex is actually on the edge (within tolerance)
            auto projected = v1 + dir * t;
            if (glm::distance(vertex->position, projected) < 1e-4f) {
                tJunctions.push_back({
                    edge,
                    vertex,
                    t / length // Normalize t to [0,1]
                });
            }
        }
    }

    // Sort T-junctions by edge and parameter
    std::sort(tJunctions.begin(), tJunctions.end(),
        [](const TJunction& a, const TJunction& b) {
            return a.edge < b.edge || (a.edge == b.edge && a.t < b.t);
        });

    // Process T-junctions
    for (size_t i = 0; i < tJunctions.size(); ++i) {
        const auto& tj = tJunctions[i];
        
        // Skip if this edge was already processed
        if (!tj.edge->halfEdge) continue;

        // Get vertices of the face containing the edge
        auto he = tj.edge->halfEdge;
        auto v1 = he->vertex;
        auto v2 = he->prev->vertex;
        auto t = tj.t;
        
        // Interpolate position and texture coordinates
        auto newPos = v1->position * (1.0f - t) + v2->position * t;
        auto newUV = v1->texCoord * (1.0f - t) + v2->texCoord * t;
        auto newVertex = baseMesh_->createVertex(newPos, newUV);
        std::vector<HalfEdgeMesh::VertexPtr> faceVertices;
        auto current = he->face->halfEdge;
        do {
            if (current == he) {
                // Insert new vertex after current vertex
                faceVertices.push_back(current->vertex);
                faceVertices.push_back(newVertex);
            } else {
                faceVertices.push_back(current->vertex);
            }
            current = current->next;
        } while (current != he->face->halfEdge);
        
        // Store face vertices for later recreation
        std::vector<HalfEdgeMesh::VertexPtr> newFaceVertices = faceVertices;
        std::vector<HalfEdgeMesh::VertexPtr> newPairFaceVertices;
        
        // If edge has a pair, store pair face vertices
        if (he->pair) {
            current = he->pair->face->halfEdge;
            do {
                if (current == he->pair) {
                    // Insert new vertex after current vertex
                    newPairFaceVertices.push_back(current->vertex);
                    newPairFaceVertices.push_back(newVertex);
                } else {
                    newPairFaceVertices.push_back(current->vertex);
                }
                current = current->next;
            } while (current != he->pair->face->halfEdge);
        }

        // Store all vertices and faces before clearing
        std::vector<glm::vec3> vertexPositions;
        std::vector<std::vector<size_t>> faceIndices;
        std::unordered_map<HalfEdgeMesh::VertexPtr, size_t> vertexIndices;

        // Store vertex positions and build index map
        for (const auto& vertex : baseMesh_->getVertices()) {
            vertexIndices[vertex] = vertexPositions.size();
            vertexPositions.push_back(vertex->position);
        }

        // Store face indices, skipping faces to be replaced
        for (const auto& face : baseMesh_->getFaces()) {
            if (face == he->face || (he->pair && face == he->pair->face)) {
                continue; // Skip faces being replaced
            }

            std::vector<size_t> indices;
            auto current = face->halfEdge;
            do {
                indices.push_back(vertexIndices[current->vertex]);
                current = current->next;
            } while (current != face->halfEdge);
            faceIndices.push_back(indices);
        }

        // Clear and rebuild mesh
        baseMesh_->clear();

        // Recreate vertices
        std::vector<HalfEdgeMesh::VertexPtr> newVertices;
        newVertices.reserve(vertexPositions.size());
        for (const auto& pos : vertexPositions) {
            // Find original vertex to get its texture coordinates
            auto origVertex = std::find_if(baseMesh_->getVertices().begin(), 
                                         baseMesh_->getVertices().end(),
                                         [&pos](const auto& v) {
                                             return glm::distance(v->position, pos) < 1e-6f;
                                         });
            if (origVertex != baseMesh_->getVertices().end()) {
                newVertices.push_back(baseMesh_->createVertex(pos, (*origVertex)->texCoord));
            } else {
                newVertices.push_back(baseMesh_->createVertex(pos, glm::vec2(0.0f)));
            }
        }
        
        // Recreate faces
        for (const auto& indices : faceIndices) {
            std::vector<HalfEdgeMesh::VertexPtr> faceVerts;
            for (size_t idx : indices) {
                faceVerts.push_back(newVertices[idx]);
            }
            baseMesh_->createFace(faceVerts);
        }

        // Create the new split faces
        std::vector<HalfEdgeMesh::VertexPtr> faceVerts;
        for (const auto& v : newFaceVertices) {
            size_t idx = vertexIndices[v];
            faceVerts.push_back(newVertices[idx]);
        }
        baseMesh_->createFace(faceVerts);

        if (he->pair) {
            faceVerts.clear();
            for (const auto& v : newPairFaceVertices) {
                size_t idx = vertexIndices[v];
                faceVerts.push_back(newVertices[idx]);
            }
            baseMesh_->createFace(faceVerts);
        }
    }

    // Validate final topology
    if (!baseMesh_->validateTopology()) {
        throw Error::InvalidOperation("T-junction fix resulted in invalid topology");
    }
}

// Boolean Operations
std::shared_ptr<SubdivisionSurface> SubdivisionSurface::booleanUnion(
    std::shared_ptr<SubdivisionSurface> other) const {
    return performBooleanOperation(other, SolidBody::BooleanOperation::Union);
}

std::shared_ptr<SubdivisionSurface> SubdivisionSurface::booleanSubtract(
    std::shared_ptr<SubdivisionSurface> other) const {
    return performBooleanOperation(other, SolidBody::BooleanOperation::Subtract);
}

std::shared_ptr<SubdivisionSurface> SubdivisionSurface::booleanIntersect(
    std::shared_ptr<SubdivisionSurface> other) const {
    return performBooleanOperation(other, SolidBody::BooleanOperation::Intersect);
}

std::shared_ptr<SubdivisionSurface> SubdivisionSurface::performBooleanOperation(
    std::shared_ptr<SubdivisionSurface> other,
    SolidBody::BooleanOperation operation) const {
    if (!other) {
        throw RebelCAD::Error::InvalidArgument("Other subdivision surface cannot be null");
    }

    // Convert both surfaces to solid bodies
    auto thisBody = toSolidBody();
    auto otherBody = other->toSolidBody();

    // Perform the boolean operation
    auto resultBody = thisBody->PerformBoolean(otherBody, operation);

    // Convert result back to subdivision surface
    auto result = fromSolidBody(resultBody);

    // Transfer material properties if this is a union operation
    if (operation == SolidBody::BooleanOperation::Union) {
        result->material_ = this->material_;
    }

    return result;
}

// Material and Texture Methods
void SubdivisionSurface::setMaterial(
    const std::string& materialName,
    const std::unordered_map<std::string, float>& properties) {
    material_.name = materialName;
    material_.properties = properties;
}

void SubdivisionSurface::setTexture(
    const std::string& materialProperty,
    const std::string& texturePath) {
    material_.texturePaths[materialProperty] = texturePath;
}

std::vector<glm::vec2> SubdivisionSurface::getUVCoordinates() const {
    std::vector<glm::vec2> uvCoords;
    if (!baseMesh_) return uvCoords;

    // Get UV coordinates from vertices
    for (const auto& vertex : baseMesh_->getVertices()) {
        uvCoords.push_back(vertex->texCoord);
    }

    return uvCoords;
}

void SubdivisionSurface::setUVCoordinates(const std::vector<glm::vec2>& uvCoords) {
    if (!baseMesh_) {
        throw RebelCAD::Error::InvalidOperation("Base mesh is null");
    }

    const auto& vertices = baseMesh_->getVertices();
    if (uvCoords.size() != vertices.size()) {
        throw RebelCAD::Error::InvalidArgument(
            "UV coordinates count does not match vertex count");
    }

    // Set UV coordinates for each vertex
    for (size_t i = 0; i < vertices.size(); ++i) {
        vertices[i]->texCoord = uvCoords[i];
    }
}

std::shared_ptr<SubdivisionSurface> SubdivisionSurface::fromSolidBody(
    std::shared_ptr<SolidBody> solidBody) {
    if (!solidBody) {
        throw RebelCAD::Error::InvalidArgument("Solid body cannot be null");
    }
    
    auto mesh = std::make_shared<HalfEdgeMesh>();
    
    // Create vertices
    std::vector<HalfEdgeMesh::VertexPtr> vertices;
    vertices.reserve(solidBody->GetVertices().size());
    
    for (const auto& vert : solidBody->GetVertices()) {
        auto vertex = mesh->createVertex(
            glm::vec3(vert.position),
            glm::vec2(vert.texCoord)
        );
        vertices.push_back(vertex);
    }
    
    // Create faces from triangles
    // We'll try to merge coplanar triangles into larger faces
    std::unordered_set<size_t> processedTriangles;
    const auto& triangles = solidBody->GetTriangles();
    
    for (size_t i = 0; i < triangles.size(); i++) {
        if (processedTriangles.count(i) > 0) continue;
        
        const auto& tri = triangles[i];
        std::vector<HalfEdgeMesh::VertexPtr> faceVertices;
        faceVertices.push_back(vertices[tri.v1]);
        faceVertices.push_back(vertices[tri.v2]);
        faceVertices.push_back(vertices[tri.v3]);
        
        // Create the face
        mesh->createFace(faceVertices);
        processedTriangles.insert(i);
    }
    
    // Set sharp edges based on angle between face normals
    const float SHARP_ANGLE_THRESHOLD = glm::radians(30.0f);
    
    for (const auto& edge : mesh->getEdges()) {
        if (!edge->halfEdge->pair) continue; // Skip boundary edges
        
        auto face1 = edge->halfEdge->face;
        auto face2 = edge->halfEdge->pair->face;
        
        // Calculate face normals
        auto getNormal = [](const HalfEdgeMesh::FacePtr& face) {
            auto he = face->halfEdge;
            auto v1 = he->vertex->position;
            auto v2 = he->next->vertex->position;
            auto v3 = he->next->next->vertex->position;
            return glm::normalize(glm::cross(v2 - v1, v3 - v1));
        };
        
        auto normal1 = getNormal(face1);
        auto normal2 = getNormal(face2);
        
        float angle = std::acos(glm::dot(normal1, normal2));
        if (angle > SHARP_ANGLE_THRESHOLD) {
            edge->isSharp = true;
            edge->sharpness = 1.0f;
        }
    }
    
    return std::make_shared<SubdivisionSurface>(mesh);
}

} // namespace Modeling
} // namespace RebelCAD
