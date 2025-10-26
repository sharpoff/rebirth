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

    enum Flag : unsigned int
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

    void manipulate(Input *input, Camera *camera, const vec2 &screenDim, mat4 &objectTransform);
    const eastl::vector<MeshDraw> getMeshDraws();

    void setOperation(Gizmo::Operation operation);
    mat4 getTransform();

    bool selected = false;
    bool dragging = false;

private:
    const float kGizmoScale = 0.2f;

    vec3 gizmoPosition = vec3(0.0f);
    float gizmoScale = 1.0f;
    quat gizmoRotation = glm::identity<quat>();

    Gizmo::Operation gizmoOperation = ROTATE;

    eastl::vector<MeshDraw> operationMeshDraws[3];
    eastl::vector<unsigned int> operationFlags[3];
};