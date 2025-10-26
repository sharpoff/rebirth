#version 450

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require

#include "vertex.glsl"
#include "material.glsl"
#include "light.glsl"

#include "pbr.glsl"
#include "scene_data.glsl"
#include "mesh_pc.glsl"
#include "textures.glsl"

layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inTangent;
layout (location = 4) in mat3 inTBN;

layout (location = 0) out vec4 fragColor;

void main()
{
    vec3 cameraPos = scene_data.cameraPosAndLightNum.xyz;
    uint lightCount = uint(scene_data.cameraPosAndLightNum.w);

    float ambient = 0.05;
    vec4 diffuse = vec4(1.0, 1.0, 1.0, 1.0);
    float metallic = 0.0;
    float roughness = 1.0;

    vec3 normal = inNormal;
    vec3 emissive = vec3(0);

    if (pc.materialId > -1) {
        Material material = materials[pc.materialId];

        if (material.diffuseId > -1) {
            diffuse = TEX_2D(material.diffuseId, inUV) * material.diffuseFactor;
        }

        if (material.metallicRoughnessId > -1) {
            vec4 metallicRoughness = vec4(0);
            metallicRoughness = TEX_2D(material.metallicRoughnessId, inUV);
            metallic = metallicRoughness.g * material.roughnessFactor;
            roughness = metallicRoughness.b * material.metallicFactor;
        }

        if ((material.materialFlags & MATERIAL_FLAG_COLOR) == MATERIAL_FLAG_COLOR)
            diffuse *= material.color;

        if (material.normalId > -1) {
            normal = TEX_2D(material.normalId, inUV).rgb;
        }

        if (material.emissiveId > -1) {
            emissive = TEX_2D(material.emissiveId, inUV).rgb;
        }

        ambient = material.ambient;
    } else {
        fragColor = vec4(1.0);
        return;
    }

    // if (diffuse.a < 0.5)
    //     discard;

    if (inTangent != vec4(0.0)) {
        normal = inTBN * normalize(normal * 2.0 - 1.0);
    }

    normal = normalize(normal);
    vec3 viewDir = normalize(cameraPos - inWorldPos);

    roughness = max(0.05, roughness);
    float reflectance = 0.4; // constant
    vec3 diffuseColor = (1.0 - metallic) * vec3(diffuse);

    vec3 f0 = 0.16 * reflectance * reflectance * (1.0 - metallic) + diffuseColor * metallic;

    vec3 finalColor = vec3(0.0);
    for (int i = 0; i < lightCount; i++) {
        Light light = lights[i];

        // Lighting
        vec3 lightDir = vec3(0.0);
        float NoL = 0.0;

        if (light.type == LIGHT_TYPE_DIRECTIONAL) {
            lightDir = normalize(-light.direction);
            NoL = clamp(dot(normal, lightDir), 0.0, 1.0);
        }

        if ((pc.drawMask & DRAW_MASK_LIGHT) == DRAW_MASK_LIGHT) {
            vec3 lightColor = vec3(0.0);

            if (light.type == LIGHT_TYPE_DIRECTIONAL)
                lightColor = pbrBRDF(lightDir, viewDir, normal, roughness, f0, diffuseColor) * NoL * light.color;

            finalColor += lightColor;
        }

        // Shadows
        if ((pc.drawMask & DRAW_MASK_SHADOW) == DRAW_MASK_SHADOW) {
            float visibility = 1.0;
            if (scene_data.shadowMapId > -1) {
                vec4 lightSpace = light.mvp * vec4(inWorldPos, 1.0);
                vec3 projCoords = lightSpace.xyz / lightSpace.w;

                vec2 coords = (projCoords.xy * 0.5 + 0.5);
                float closestDepth = TEX_2D(scene_data.shadowMapId, coords).r;
                float currentDepth = projCoords.z;

                float bias = max(0.0005 * (1.0 - NoL), 0.0001);

                // poisson sampling
                for (int i = 0; i < 4; i++) {
                    if (TEX_2D(scene_data.shadowMapId, coords + poissonDisk[i] / 5000.0).r > currentDepth - bias) {
                        visibility -= 0.2;
                    }
                }
            }

            finalColor *= (NoL * visibility);
        }
    }

    Material material = materials[pc.materialId];
    if ((material.materialFlags & MATERIAL_FLAG_AMBIENT) == MATERIAL_FLAG_AMBIENT)
        finalColor += vec3(diffuse) * ambient;
    else
        finalColor += vec3(diffuse);

    finalColor += emissive;

    fragColor = vec4(finalColor, diffuse.a);
}
