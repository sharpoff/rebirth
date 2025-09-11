#pragma once

#include <stdint.h>
#include <volk.h>

enum class ModelID : uint32_t { Invalid = UINT32_MAX };
enum class GPUMeshID : uint32_t { Invalid = UINT32_MAX  };
enum class CPUMeshID : uint32_t { Invalid = UINT32_MAX  };
enum class MaterialID : uint32_t { Invalid = UINT32_MAX  };
enum class ImageID : uint32_t { Invalid = UINT32_MAX  };
enum class LightID : uint32_t { Invalid = UINT32_MAX  };
enum class RigidBodyID : uint32_t { Invalid = UINT32_MAX  };
enum class EntityID : uint32_t { Invalid = UINT32_MAX  };

enum class SceneNodeID : uint32_t { Invalid = UINT32_MAX };
enum class SkinID : uint32_t { Invalid = UINT32_MAX };
enum class SamplerID : uint32_t { Invalid = UINT32_MAX };

#define ID(x) static_cast<uint32_t>((x))