#include "util/common.h"

namespace util
{
    vec3 mouseToWorldDirection(vec2 mouseCoords, vec2 screenDim, mat4 cameraView, mat4 cameraProjection)
    {
        double ndc_x = (2.0 * mouseCoords.x / screenDim.x) - 1.0;
        double ndc_y = (2.0 * mouseCoords.y / screenDim.y) - 1.0;

        vec4 clipSpace = vec4(ndc_x, ndc_y, 0.0, 1.0);
        vec4 worldSpace = glm::inverse(cameraProjection * cameraView) * clipSpace;

        vec3 direction = vec3(glm::normalize(worldSpace));
        worldSpace /= worldSpace.w; // perspective division

        return direction;
    }
} // namespace util