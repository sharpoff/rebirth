#include "utility/common.h"

/*

namespace utility
{
    vec3 mouseToWorldDirection(const vec2 &mouseCoords, const vec2 &screenDim, const mat4 &cameraView, const mat4 &cameraProjection)
    {
        double ndc_x = (2.0 * mouseCoords.x / screenDim.x) - 1.0;
        double ndc_y = (2.0 * mouseCoords.y / screenDim.y) - 1.0;

        vec4 clipSpace = vec4(ndc_x, ndc_y, 0.0, 1.0);
        vec4 worldSpace = math::inverse(cameraProjection * cameraView) * clipSpace;

        vec3 direction = vec3(math::normalize(worldSpace));
        worldSpace /= worldSpace.w; // perspective division

        return direction;
    }
} // namespace utility

*/