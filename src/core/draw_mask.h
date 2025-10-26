#pragma once

// should match shader
enum DrawMask
{
    Opaque = 1 << 0,
    Transparent = 1 << 1,
    Shadow = 1 << 2,
    Light = 1 << 3,
    Wireframe = 1 << 4,
    Overlay = 1 << 5,
};