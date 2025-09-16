#include <rebirth/graphics/primitives.h>

#include <rebirth/resource_manager.h>

ModelID generateCube()
{
    std::vector<Vertex> vertices = {
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
        {{-0.5f, -0.5f, -0.5f}, 1.0f, {0.0f, -1.0f, 0.0f}, 1.0f}};

    std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0, 4, 6, 5, 6, 4, 7, 8, 9, 10, 10, 11, 8, 12, 14, 13, 14, 12, 15, 16, 17, 18, 18, 19, 16, 20, 22, 21, 22, 20, 23};

    Mesh mesh(MaterialID::Invalid, g_resourceManager.addVerticesAndIndices(vertices, indices), indices.size());

    Model model;
    model.meshes.push_back(g_resourceManager.addMesh(mesh));
    return g_resourceManager.addModel(model);
}

// credit: https://www.songho.ca/opengl/gl_sphere.html
ModelID generateUVSphere(float radius)
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    float x, y, z, xy; // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius; // vertex normal
    float s, t; // vertex texCoord

    int sectorCount = 10;
    int stackCount = 10;

    float sectorStep = 2 * glm::pi<float>() / sectorCount;
    float stackStep = glm::pi<float>() / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = glm::pi<float>() / 2 - i * stackStep; // starting from pi/2 to -pi/2
        xy = radius * std::cosf(stackAngle); // r * cos(u)
        z = radius * std::sinf(stackAngle); // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= sectorCount; ++j) {
            Vertex vert;

            sectorAngle = j * sectorStep; // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * std::cosf(sectorAngle); // r * cos(u) * cos(v)
            y = xy * std::sinf(sectorAngle); // r * cos(u) * sin(v)
            vert.position = vec3(x, y, z);

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            vert.normal = vec3(nx, ny, nz);

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            vert.uv_x = s;
            vert.uv_y = t;

            vertices.push_back(vert);
        }
    }

    int k1, k2;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1); // beginning of current stack
        k2 = k1 + sectorCount + 1; // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    Mesh mesh(MaterialID::Invalid, g_resourceManager.addVerticesAndIndices(vertices, indices), indices.size());

    Model model;
    model.meshes.push_back(g_resourceManager.addMesh(mesh));
    return g_resourceManager.addModel(model);
}