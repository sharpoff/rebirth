#pragma once

#include <volk.h>

#include <rebirth/math/frustum.h>
#include <rebirth/resource_manager.h>
#include <rebirth/types/camera.h>
#include <rebirth/types/mesh.h>
#include <rebirth/types/mesh_draw.h>

class MeshPipeline
{
public:
    void initialize();
    void destroy();

    void beginFrame(VkCommandBuffer cmd, bool wireframe);
    void endFrame(VkCommandBuffer cmd);
    void draw(VkCommandBuffer cmd, Frustum &frustum, std::vector<MeshDraw> &meshDraws);

private:
    VkPipeline meshPipeline = VK_NULL_HANDLE;
    VkPipeline wireframePipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;

    struct PushConstant
    {
        mat4 transform;
        int materialId;
    };
};