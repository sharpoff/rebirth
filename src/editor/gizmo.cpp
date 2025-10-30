#include "editor/gizmo.h"

#include "core/camera.h"
#include "core/mesh.h"
#include "core/mesh_draw.h"
#include "core/resource_manager.h"
#include "input/input.h"
#include "math/bounds.h"
#include "math/math.h"
#include "util/logger.h"
#include "util/util.h"

void Gizmo::manipulate(Input *input, Camera *camera, const vec2 &screenDim, mat4 &objectTransform)
{
    assert(input && camera);

    if (input->getKey(KeyboardKey::T, InputAction::JustPressed)) {
        gizmoOperation = TRANSLATE;
    }

    if (input->getKey(KeyboardKey::R, InputAction::JustPressed)) {
        gizmoOperation = ROTATE;
    }

    if (input->getKey(KeyboardKey::E, InputAction::JustPressed)) {
        gizmoOperation = SCALE;
    }

    const vec3 objectPosition = math::getPosition(objectTransform);
    gizmoPosition = objectPosition;

    float distanceToObject = glm::distance(camera->getPosition(), objectPosition);
    gizmoScale = distanceToObject * kGizmoScale * camera->getFov();

    // TODO: make it work!!!
    // try to raycast to find hit axis
    if (!dragging && input->getMouseButton(MouseButton::RIGHT, InputAction::Pressed)) {
        auto      &vertices = ResourceManager::get()->getVertices();
        const vec3 rayDir = util::mouseToWorldDirection(input->getMousePosition(), screenDim, camera->getView(), camera->getProjection());

        bool hit = false;
        if (gizmoOperation == TRANSLATE) {
            for (MeshDraw &meshDraw : operationMeshDraws[TRANSLATE]) {
                Mesh *mesh = ResourceManager::get()->getMeshByIndex(meshDraw.meshId);
                assert(mesh);

                for (auto &prim : mesh->primitives) {
                    if (hit)
                        break;

                    for (size_t j = 0; j < prim.vertexCount; j += 3) {
                        if (hit)
                            break;

                        const vec3 &v0 = vertices[j + prim.vertexOffset + 0].position;
                        const vec3 &v1 = vertices[j + prim.vertexOffset + 1].position;
                        const vec3 &v2 = vertices[j + prim.vertexOffset + 2].position;

                        float distance = 0.0f;
                        if (util::rayIntersectVertex(camera->getPosition(), rayDir, v0, v1, v2, distance)) {
                            dragging = true;
                            hit = true;
                            LOGI("%s", "HIT!");
                            LOGI("distance: %f", distance);
                        }
                    }
                }
            }
        }
    }

    if (input->getMouseButton(MouseButton::LEFT, InputAction::Released))
        dragging = false;

    if (dragging) {
        LOGI("%s", "DRAGGING");
    }

    operationMeshDraws[gizmoOperation].clear();
}

void Gizmo::setOperation(Gizmo::Operation operation)
{
    this->gizmoOperation = operation;
}

mat4 Gizmo::getTransform()
{
    return math::calculateTransform(gizmoPosition, gizmoRotation, vec3(gizmoScale));
}

const eastl::vector<MeshDraw> Gizmo::getMeshDraws()
{
    if (!selected)
        return {};

    eastl::string gizmoName = "Gizmo";
    switch (gizmoOperation) {
        case Gizmo::TRANSLATE:
            gizmoName += "Translation";
            break;
        case Gizmo::ROTATE:
            gizmoName += "Rotation";
            break;
        case Gizmo::SCALE:
            gizmoName += "Scale";
            break;
    }

    Mesh *meshX = ResourceManager::get()->getMeshByName(gizmoName + "X");
    Mesh *meshY = ResourceManager::get()->getMeshByName(gizmoName + "Y");
    Mesh *meshZ = ResourceManager::get()->getMeshByName(gizmoName + "Z");
    Mesh *meshXYZ = ResourceManager::get()->getMeshByName(gizmoName + "XYZ");

    Mesh* meshes[] = {meshX, meshY, meshZ, meshXYZ};
    const char *colorMaterialNames[] = {"red", "green", "blue", "white_transparent"};

    assert(ARRAY_SIZE(meshes) == ARRAY_SIZE(colorMaterialNames));

    for (unsigned int i = 0; i < ARRAY_SIZE(meshes); i++) {
        Mesh *mesh = meshes[i];

        MeshDraw &meshDraw = operationMeshDraws[gizmoOperation].emplace_back();
        meshDraw.meshId = ResourceManager::get()->getMeshIndex(mesh);
        meshDraw.overrideMaterialId = ResourceManager::get()->getMaterialIndexByName(colorMaterialNames[i]);
        meshDraw.transform = getTransform() * glm::translate(math::getPosition(mesh->transform));
        meshDraw.drawMask = DrawMask::Transparent | DrawMask::Overlay;

        if (i == 3) { // all flags
            operationFlags[gizmoOperation].push_back((gizmoOperation * 3 + 0) | (gizmoOperation * 3 + 1) | (gizmoOperation * 3 + 2));
        } else { // specific flag
            operationFlags[gizmoOperation].push_back(gizmoOperation * 3 + i);
        }
    }

    return operationMeshDraws[gizmoOperation];
}