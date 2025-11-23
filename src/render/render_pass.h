#pragma once

#include "render/graphics_types.h"

class RenderPass
{
public:
    void read(const char *name, Texture *renderTarget);
    void write(const char *name);
};