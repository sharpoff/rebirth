#pragma once

#include <rebirth/types/mesh.h>

namespace rebirth
{

std::pair<std::vector<Vertex>, std::vector<uint32_t>> generateCube();
std::pair<std::vector<Vertex>, std::vector<uint32_t>> generateUVSphere(float radius);

} // namespace rebirth
