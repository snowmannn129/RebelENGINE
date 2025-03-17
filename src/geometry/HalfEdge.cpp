#include "modeling/HalfEdge.hpp"
#include <algorithm>
#include <cassert>

namespace rebel_cad {
namespace modeling {

std::vector<Vertex*> Vertex::getAdjacentVertices() const {
    std::vector<Vertex*> adjacent;
    if (!outgoing) return adjacent;

    // Start with the first outgoing edge
    HalfEdge* current = outgoing;
    do {
        // Add the vertex at the end of this half-edge
        adjacent.push_back(current->vertex);
        
        // Move to the next outgoing edge by following pair->next
        current = current->pair ? current->pair->next : nullptr;
        
        // Stop if we've hit a boundary or completed the circuit
    } while (current && current != outgoing);

    return adjacent;
}

std::vector<Face*> Vertex::getIncidentFaces() const {
    std::vector<Face*> faces;
    if (!outgoing) return faces;

    // Start with the first outgoing edge
    HalfEdge* current = outgoing;
    do {
        // Add the face if it exists (might be null for boundary edges)
        if (current->face) {
            faces.push_back(current->face);
        }
        
        // Move to the next outgoing edge
        current = current->pair ? current->pair->next : nullptr;
        
    } while (current && current != outgoing);

    return faces;
}

std::vector<HalfEdge*> Vertex::getOutgoingHalfEdges() const {
    std::vector<HalfEdge*> edges;
    if (!outgoing) return edges;

    // Start with the first outgoing edge
    HalfEdge* current = outgoing;
    do {
        edges.push_back(current);
        
        // Move to the next outgoing edge
        current = current->pair ? current->pair->next : nullptr;
        
    } while (current && current != outgoing);

    return edges;
}

float HalfEdge::getLength() const {
    Vertex* start = getStartVertex();
    if (!start || !vertex) return 0.0f;
    
    return glm::length(vertex->position - start->position);
}

std::vector<Vertex*> Face::getVertices() const {
    std::vector<Vertex*> vertices;
    if (!edge) return vertices;

    // Traverse the face's half-edges
    HalfEdge* current = edge;
    do {
        vertices.push_back(current->vertex);
        current = current->next;
    } while (current && current != edge);

    return vertices;
}

std::vector<HalfEdge*> Face::getEdges() const {
    std::vector<HalfEdge*> edges;
    if (!edge) return edges;

    // Traverse the face's half-edges
    HalfEdge* current = edge;
    do {
        edges.push_back(current);
        current = current->next;
    } while (current && current != edge);

    return edges;
}

std::vector<Face*> Face::getAdjacentFaces() const {
    std::vector<Face*> adjacent;
    if (!edge) return adjacent;

    // Get all edges of this face
    auto edges = getEdges();
    
    // For each edge, if it has a pair, get its face
    for (HalfEdge* e : edges) {
        if (e->pair && e->pair->face) {
            adjacent.push_back(e->pair->face);
        }
    }

    return adjacent;
}

glm::vec3 Face::getCentroid() const {
    auto vertices = getVertices();
    if (vertices.empty()) return glm::vec3(0.0f);

    glm::vec3 centroid(0.0f);
    for (const Vertex* v : vertices) {
        centroid += v->position;
    }
    return centroid / static_cast<float>(vertices.size());
}

void Face::updateNormal() {
    auto vertices = getVertices();
    if (vertices.size() < 3) {
        normal = glm::vec3(0.0f, 1.0f, 0.0f);
        return;
    }

    // Use Newell's method for robust normal calculation
    normal = glm::vec3(0.0f);
    for (size_t i = 0; i < vertices.size(); ++i) {
        const glm::vec3& current = vertices[i]->position;
        const glm::vec3& next = vertices[(i + 1) % vertices.size()]->position;
        
        normal.x += (current.y - next.y) * (current.z + next.z);
        normal.y += (current.z - next.z) * (current.x + next.x);
        normal.z += (current.x - next.x) * (current.y + next.y);
    }
    
    normal = glm::normalize(normal);
}

size_t Face::getEdgeCount() const {
    size_t count = 0;
    if (!edge) return count;

    HalfEdge* current = edge;
    do {
        ++count;
        current = current->next;
    } while (current && current != edge);

    return count;
}

namespace HalfEdgeFactory {

std::unique_ptr<Vertex> createVertex(
    const glm::vec3& position,
    const glm::vec3& normal,
    const glm::vec2& uv
) {
    auto vertex = std::make_unique<Vertex>();
    vertex->position = position;
    vertex->normal = normal;
    vertex->uv = uv;
    vertex->outgoing = nullptr;
    vertex->isSharp = false;
    vertex->index = 0; // Should be set by mesh
    return vertex;
}

std::unique_ptr<HalfEdge> createHalfEdge() {
    auto edge = std::make_unique<HalfEdge>();
    edge->vertex = nullptr;
    edge->pair = nullptr;
    edge->next = nullptr;
    edge->face = nullptr;
    edge->isSharp = false;
    return edge;
}

std::unique_ptr<Face> createFace() {
    auto face = std::make_unique<Face>();
    face->edge = nullptr;
    face->normal = glm::vec3(0.0f, 1.0f, 0.0f);
    face->index = 0; // Should be set by mesh
    return face;
}

} // namespace HalfEdgeFactory

} // namespace modeling
} // namespace rebel_cad
