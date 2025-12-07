#include "math/frustum_culling.h"

#include "core/stl.h"
#include <math.h>

namespace math
{
    bool isSphereVisible(const Bounds &sphere, mat4 viewProj, mat4 transform)
    {
        Array<vec3, 8> corners{
            vec3{1, 1, 1},
            vec3{1, 1, -1},
            vec3{1, -1, 1},
            vec3{1, -1, -1},
            vec3{-1, 1, 1},
            vec3{-1, 1, -1},
            vec3{-1, -1, 1},
            vec3{-1, -1, -1},
        };

        mat4 matrix = viewProj * transform;

        vec3 vmin = {1.5, 1.5, 1.5};
        vec3 vmax = {-1.5, -1.5, -1.5};

        for (int c = 0; c < 8; c++) {
            // project each corner into clip space
            vec4 v = matrix * vec4(sphere.origin + (corners[c] * sphere.extents), 1.f);

            // perspective correction
            v[0] = v[0] / v[3];
            v[1] = v[1] / v[3];
            v[2] = v[2] / v[3];

            vmin[0] = fmin(v.x(), vmin.x());
            vmin[1] = fmin(v.y(), vmin.y());
            vmin[2] = fmin(v.z(), vmin.z());

            vmax[0] = fmax(v.x(), vmax.x());
            vmax[1] = fmax(v.y(), vmax.y());
            vmax[2] = fmax(v.z(), vmax.z());
        }

        // check the clip space box is within the view
        if (vmin.z() > 1.f || vmax.z() < 0.f || vmin.x() > 1.f || vmax.x() < -1.f || vmin.y() > 1.f || vmax.y() < -1.f) {
            return false;
        } else {
            return true;
        }
    }
} // namespace math