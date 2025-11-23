#include "core/util.h"

namespace util
{
    eastl::vector<Vertex> generateRingVertices(float sphereRadius, float ringThickness, int segments, int thicknessSegments)
    {
        eastl::vector<Vertex> vertices;
        for (int i = 0; i < segments; ++i) {
            float angle = glm::two_pi<float>() * i / segments;
            vec3  ringCenter = vec3(sphereRadius * cos(angle), sphereRadius * sin(angle), 0.0f);
            for (int j = 0; j < thicknessSegments; ++j) {
                float thicknessAngle = glm::two_pi<float>() * j / thicknessSegments;
                vec3  offset = vec3(
                    ringThickness * cos(thicknessAngle) * cos(angle),
                    ringThickness * cos(thicknessAngle) * sin(angle),
                    ringThickness * sin(thicknessAngle));

                Vertex &vertex = vertices.emplace_back();
                vertex.position = ringCenter + offset;
                vertex.normal = glm::normalize(offset);
                vertex.tangent = glm::normalize(vec4(-sin(angle), cos(angle), 0.0f, 0.0f));
            }
        }
        return vertices;
    }

    eastl::vector<uint32_t> generateRingIndices(int segments, int thicknessSegments)
    {
        eastl::vector<uint32_t> indices;
        for (int i = 0; i < segments; ++i) {
            for (int j = 0; j < thicknessSegments; ++j) {
                int nextI = (i + 1) % segments;
                int nextJ = (j + 1) % thicknessSegments;
                // Indices of the quad
                uint32_t v0 = i * thicknessSegments + j;
                uint32_t v1 = nextI * thicknessSegments + j;
                uint32_t v2 = i * thicknessSegments + nextJ;
                uint32_t v3 = nextI * thicknessSegments + nextJ;
                // First triangle
                indices.push_back(v0);
                indices.push_back(v1);
                indices.push_back(v2);
                // Second triangle
                indices.push_back(v2);
                indices.push_back(v1);
                indices.push_back(v3);
            }
        }
        return indices;
    }

    eastl::vector<Vertex> generateSphereVertices(float radius, int segments)
    {
        eastl::vector<Vertex> vertices;
        // Ensure segments are reasonable
        segments = std::max(segments, 4);
        // Angular step sizes
        float thetaStep = glm::two_pi<float>() / segments; // Longitude step
        float phiStep = glm::pi<float>() / segments; // Latitude step
        for (int i = 0; i <= segments; ++i) { // Latitude loop
            float phi = i * phiStep; // Latitude angle
            for (int j = 0; j <= segments; ++j) { // Longitude loop
                float theta = j * thetaStep; // Longitude angle
                // Calculate position on the sphere
                vec3 position = vec3(
                    radius * sin(phi) * cos(theta),
                    radius * cos(phi),
                    radius * sin(phi) * sin(theta));
                // Calculate normal (direction from sphere center)
                vec3 normal = glm::normalize(position);
                // Calculate tangent (partial derivative with respect to theta)
                vec4 tangent = glm::normalize(vec4(
                    -radius * sin(phi) * sin(theta),
                    0.0f,
                    radius * sin(phi) * cos(theta),
                    0.0f));
                // Add vertex to the vector
                Vertex &vertex = vertices.emplace_back();
                vertex.position = position;
                vertex.normal = normal;
                vertex.tangent = tangent;
            }
        }
        return vertices;
    }

    eastl::vector<uint32_t> generateSphereIndices(int segments)
    {
        eastl::vector<uint32_t> indices;
        for (int i = 0; i < segments; ++i) { // Latitude loop
            for (int j = 0; j < segments; ++j) { // Longitude loop
                int nextI = i + 1;
                int nextJ = (j + 1) % (segments + 1);
                // Indices of the quad
                uint32_t v0 = i * (segments + 1) + j;
                uint32_t v1 = nextI * (segments + 1) + j;
                uint32_t v2 = i * (segments + 1) + nextJ;
                uint32_t v3 = nextI * (segments + 1) + nextJ;
                // First triangle
                indices.push_back(v2);
                indices.push_back(v1);
                indices.push_back(v0);
                // Second triangle
                indices.push_back(v3);
                indices.push_back(v1);
                indices.push_back(v2);
            }
        }
        return indices;
    }

    eastl::vector<Vertex> generateConeVertices(float radius, float height, int segments)
    {
        eastl::vector<Vertex> vertices;
        // Ensure segments is at least 3 (minimum for a cone base)
        segments = std::max(segments, 3);
        // Apex vertex
        Vertex &apex = vertices.emplace_back();
        apex.position = vec3(0.0f, height, 0.0f);
        apex.normal = glm::normalize(vec3(0.0f, height, 0.0f)); // Normal is along the cone axis
        apex.tangent = vec4(1.0f, 0.0f, 0.0f, 0.0f); // Arbitrary tangent
        // Base circle vertices
        float angleStep = glm::two_pi<float>() / segments;
        for (int i = 0; i < segments; ++i) {
            float angle = i * angleStep;
            // Base vertex position
            vec3 position(radius * cos(angle), 0.0f, radius * sin(angle));
            // Base vertex normal (pointing outward at an angle)
            vec3 normal = glm::normalize(vec3(position.x, height / 2.0f, position.z));
            // Tangent (aligned with the circle's tangent direction)
            vec4    tangent = glm::normalize(vec4(-sin(angle), 0.0f, cos(angle), 0.0f));
            Vertex &baseVertex = vertices.emplace_back();
            baseVertex.position = position;
            baseVertex.normal = normal;
            baseVertex.tangent = tangent;
        }
        // Base center vertex
        Vertex &baseCenter = vertices.emplace_back();
        baseCenter.position = vec3(0.0f, 0.0f, 0.0f);
        baseCenter.normal = vec3(0.0f, -1.0f, 0.0f); // Normal points downward
        baseCenter.tangent = vec4(1.0f, 0.0f, 0.0f, 0.0f); // Arbitrary tangent
        return vertices;
    }

    eastl::vector<uint32_t> generateConeIndices(int segments)
    {
        eastl::vector<uint32_t> indices;
        // Side triangles
        for (int i = 0; i < segments; ++i) {
            uint32_t apexIndex = 0; // Apex is the first vertex
            uint32_t baseIndex1 = i + 1; // Current base point
            uint32_t baseIndex2 = (i + 1) % segments + 1; // Next base point (wrapping around)
            // Triangle: apex -> baseIndex1 -> baseIndex2
            indices.push_back(baseIndex2);
            indices.push_back(baseIndex1);
            indices.push_back(apexIndex);
        }
        // Base cap triangles
        uint32_t centerIndex = segments + 1; // Base center
        for (int i = 0; i < segments; ++i) {
            uint32_t baseIndex1 = i + 1; // Current base point
            uint32_t baseIndex2 = (i + 1) % segments + 1; // Next base point (wrapping around)
            // Triangle: centerIndex -> baseIndex2 -> baseIndex1
            indices.push_back(baseIndex1);
            indices.push_back(baseIndex2);
            indices.push_back(centerIndex);
        }
        return indices;
    }

    eastl::vector<Vertex> generateCylinderVertices(float radius, float height, int subdivisions)
    {
        eastl::vector<Vertex> vertices;
        const float           angleStep = 2.0f * MATH_PI / subdivisions;

        // Generate vertices for the bottom cap
        vertices.push_back({vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)}); // Center of the bottom cap
        for (int i = 0; i <= subdivisions; ++i) {
            float angle = i * angleStep;
            float x = radius * cos(angle);
            float z = radius * sin(angle);

            vertices.push_back({vec3(x, 0.0f, z), vec3(0.0f, -1.0f, 0.0f)});
        }

        // Generate vertices for the top cap
        vertices.push_back({vec3(0.0f, height, 0.0f), vec3(0.0f, 1.0f, 0.0f)}); // Center of the top cap

        for (int i = 0; i <= subdivisions; ++i) {
            float angle = i * angleStep;
            float x = radius * cos(angle);
            float z = radius * sin(angle);

            vertices.push_back({vec3(x, height, z), vec3(0.0f, 1.0f, 0.0f)});
        }

        // Generate vertices for the side
        for (int i = 0; i <= subdivisions; ++i) {
            float angle = i * angleStep;
            float x = radius * cos(angle);
            float z = radius * sin(angle);
            vec3  normal = glm::normalize(vec3(cos(angle), 0.0f, sin(angle)));

            // Bottom vertex
            vertices.push_back({vec3(x, 0.0f, z), normal});

            // Top vertex
            vertices.push_back({vec3(x, height, z), normal});
        }
        return vertices;
    }

    eastl::vector<uint32_t> generateCylinderIndices(int subdivisions)
    {
        eastl::vector<uint32_t> indices;

        // Indices for the bottom cap
        for (int i = 1; i <= subdivisions; ++i) {
            indices.push_back(0); // Center of the bottom cap
            indices.push_back(i); // Current vertex
            indices.push_back(i % subdivisions + 1); // Next vertex (wraps around)
        }

        // Indices for the top cap
        int topCenterIndex = subdivisions + 2; // Index of the top center
        for (int i = 1; i <= subdivisions; ++i) {
            indices.push_back(topCenterIndex + (i % subdivisions) + 1); // Next vertex (wraps around)
            indices.push_back(topCenterIndex + i); // Current vertex
            indices.push_back(topCenterIndex); // Center of the top cap
        }

        // Indices for the side
        int sideStartIndex = (subdivisions + 2) * 2;
        for (int i = 0; i < subdivisions; ++i) {
            int bottomIndex = sideStartIndex + i * 2;
            int topIndex = bottomIndex + 1;

            indices.push_back(bottomIndex); // Bottom vertex of the quad
            indices.push_back(topIndex); // Top vertex of the quad
            indices.push_back(bottomIndex + 2); // Next bottom vertex of the quad

            indices.push_back(topIndex); // Top vertex of the quad
            indices.push_back(topIndex + 2); // Next top vertex of the quad
            indices.push_back(bottomIndex + 2); // Next bottom vertex of the quad
        }

        return indices;
    }

    eastl::vector<Vertex> generateCubeVertices()
    {
        // clang-format off
        vec3 normals[] = {
            { 0.0f,  0.0f,  1.0f}, // Front
            { 0.0f,  0.0f, -1.0f}, // Back
            { 1.0f,  0.0f,  0.0f}, // Right
            {-1.0f,  0.0f,  0.0f}, // Left
            { 0.0f,  1.0f,  0.0f}, // Top
            { 0.0f, -1.0f,  0.0f}, // Bottom
        };

        vec3 positions[] = {
            {-1.0f, -1.0f,  1.0f}, {1.0f, -1.0f,  1.0f}, {1.0f,  1.0f,  1.0f}, {-1.0f,  1.0f,  1.0f},   // Front
            {-1.0f, -1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f}, {1.0f,  1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},   // Back
            {1.0f, -1.0f,  1.0f}, {1.0f, -1.0f, -1.0f}, {1.0f,  1.0f, -1.0f}, {1.0f,  1.0f,  1.0f},     // Right
            {-1.0f, -1.0f,  1.0f}, {-1.0f,  1.0f,  1.0f}, {-1.0f,  1.0f, -1.0f}, {-1.0f, -1.0f, -1.0f}, // Left
            {-1.0f,  1.0f,  1.0f}, {1.0f,  1.0f,  1.0f}, {1.0f,  1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f},   // Top
            {-1.0f, -1.0f,  1.0f}, {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f}, {1.0f, -1.0f,  1.0f},   // Bottom
        };

        vec2 uvs[] = {
            {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, // Front
            {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, // Back
            {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, // Right
            {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, // Left
            {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, // Top
            {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, // Bottom
        };
        // clang-format on

        eastl::vector<Vertex> vertices;
        for (int i = 0; i < 6; ++i) { // Each face
            for (int j = 0; j < 4; ++j) { // Each vertex per face
                vertices.push_back({positions[i * 4 + j], normals[i], uvs[i * 4 + j]});
            }
        }
        return vertices;
    }

    eastl::vector<uint32_t> generateCubeIndices()
    {
        eastl::vector<uint32_t> indices = {
            // clang-format off
            0, 1, 2, 2, 3, 0,       // Front face
            4, 5, 6, 6, 7, 4,       // Back face
            8, 9, 10, 10, 11, 8,    // Right face
            12, 13, 14, 14, 15, 12, // Left face
            16, 17, 18, 18, 19, 16, // Top face
            20, 21, 22, 22, 23, 20  // Bottom face
            // clang-format on
        };
        return indices;
    }
} // namespace util