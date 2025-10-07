#include <rebirth/graphics/primitives.h>

#include <rebirth/graphics/renderer.h>

Primitive generateCube(Renderer &renderer)
{
    eastl::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.5f}, 0.0f, {0.0f, 0.0f, 1.0f}, 0.0f},
        {{0.5f, -0.5f, 0.5f}, 1.0f, {0.0f, 0.0f, 1.0f}, 0.0f},
        {{0.5f, 0.5f, 0.5f}, 1.0f, {0.0f, 0.0f, 1.0f}, 1.0f},
        {{-0.5f, 0.5f, 0.5f}, 0.0f, {0.0f, 0.0f, 1.0f}, 1.0f},
        {{-0.5f, -0.5f, -0.5f}, 1.0f, {0.0f, 0.0f, -1.0f}, 0.0f},
        {{0.5f, -0.5f, -0.5f}, 0.0f, {0.0f, 0.0f, -1.0f}, 0.0f},
        {{0.5f, 0.5f, -0.5f}, 0.0f, {0.0f, 0.0f, -1.0f}, 1.0f},
        {{-0.5f, 0.5f, -0.5f}, 1.0f, {0.0f, 0.0f, -1.0f}, 1.0f},
        {{0.5f, -0.5f, 0.5f}, 0.0f, {1.0f, 0.0f, 0.0f}, 0.0f},
        {{0.5f, -0.5f, -0.5f}, 1.0f, {1.0f, 0.0f, 0.0f}, 0.0f},
        {{0.5f, 0.5f, -0.5f}, 1.0f, {1.0f, 0.0f, 0.0f}, 1.0f},
        {{0.5f, 0.5f, 0.5f}, 0.0f, {1.0f, 0.0f, 0.0f}, 1.0f},
        {{-0.5f, -0.5f, 0.5f}, 1.0f, {-1.0f, 0.0f, 0.0f}, 0.0f},
        {{-0.5f, -0.5f, -0.5f}, 0.0f, {-1.0f, 0.0f, 0.0f}, 0.0f},
        {{-0.5f, 0.5f, -0.5f}, 0.0f, {-1.0f, 0.0f, 0.0f}, 1.0f},
        {{-0.5f, 0.5f, 0.5f}, 1.0f, {-1.0f, 0.0f, 0.0f}, 1.0f},
        {{-0.5f, 0.5f, 0.5f}, 0.0f, {0.0f, 1.0f, 0.0f}, 0.0f},
        {{0.5f, 0.5f, 0.5f}, 1.0f, {0.0f, 1.0f, 0.0f}, 0.0f},
        {{0.5f, 0.5f, -0.5f}, 1.0f, {0.0f, 1.0f, 0.0f}, 1.0f},
        {{-0.5f, 0.5f, -0.5f}, 0.0f, {0.0f, 1.0f, 0.0f}, 1.0f},
        {{-0.5f, -0.5f, 0.5f}, 1.0f, {0.0f, -1.0f, 0.0f}, 0.0f},
        {{0.5f, -0.5f, 0.5f}, 0.0f, {0.0f, -1.0f, 0.0f}, 0.0f},
        {{0.5f, -0.5f, -0.5f}, 0.0f, {0.0f, -1.0f, 0.0f}, 1.0f},
        {{-0.5f, -0.5f, -0.5f}, 1.0f, {0.0f, -1.0f, 0.0f}, 1.0f},
    };

    eastl::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0, 4, 6, 5, 6, 4, 7, 8, 9, 10, 10, 11, 8, 12, 14, 13, 14, 12, 15, 16, 17, 18, 18, 19, 16, 20, 22, 21, 22, 20, 23};

    Primitive primitive;
    primitive.indexCount = indices.size();
    primitive.indexOffset = renderer.indices.size();
    primitive.vertexCount = vertices.size();
    primitive.vertexOffset = renderer.vertices.size();
    primitive.materialIndex = -1;

    renderer.vertices.insert(renderer.vertices.end(), vertices.begin(), vertices.end());
    renderer.indices.insert(renderer.indices.end(), indices.begin(), indices.end());

    return primitive;
}