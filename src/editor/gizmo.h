#pragma once

#include "math/math.h"
#include "core/mesh_draw.h"

#include "EASTL/vector.h"

class Input;
class Camera;

class Gizmo
{
public:
    enum Operation
    {
        TRANSLATE,
        ROTATE,
        SCALE
    };

    enum Flag
    {
        TRANSLATE_X,
        TRANSLATE_Y,
        TRANSLATE_Z,

        ROTATE_X,
        ROTATE_Y,
        ROTATE_Z,

        SCALE_X,
        SCALE_Y,
        SCALE_Z,
    };

    struct OperationInfo
    {
    };

    void manipulate(Input *input, Camera *camera, const vec2 &screenDim, mat4 &objectTransform);
    const eastl::vector<MeshDraw> getMeshDraws();

    void setOperation(Gizmo::Operation operation);
    mat4 getTransform();

    bool selected = false;
    bool dragging = false;

private:
    vec3 gizmoPosition = vec3(0.0f);
    float gizmoScale = 1.0f;
    quat gizmoRotation = glm::identity<quat>();

    Gizmo::Operation gizmoOperation = SCALE;

    eastl::vector<MeshDraw> operationMeshDraws[3];
    eastl::vector<int> operationFlags[3];
};