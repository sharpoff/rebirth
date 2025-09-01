#version 450

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require

#include "types.glsl"
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

    vec4 baseColor = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 metallicRoughtness = vec4(0);
    vec3 normal = inNormal;
    vec3 emissive = vec3(0);

    if (pc.materialIdx > -1) {
        Material material = materials[pc.materialIdx];

        if (material.baseColorIdx > -1) {
            baseColor = TEX(material.baseColorIdx, inUV) * material.baseColorFactor;
        }

        if (material.metallicRoughnessIdx > -1) {
            metallicRoughtness = TEX(material.metallicRoughnessIdx, inUV);
            metallicRoughtness.g *= material.roughnessFactor; // roughness
            metallicRoughtness.b *= material.metallicFactor; // metallic
        }

        if (material.normalIdx > -1) {
            normal = TEX(material.normalIdx, inUV).rgb;
        }

        if (material.emissiveIdx > -1) {
            emissive = TEX(material.emissiveIdx, inUV).rgb;
        }
    }

    if (baseColor.a < 0.5)
        discard;

    if (inTangent != vec4(0.0)) {
        normal = inTBN * normalize(normal * 2.0 - 1.0);
    }

    normal = normalize(normal);
    vec3 viewDir = normalize(cameraPos - inWorldPos);

    float roughness = max(0.05, metallicRoughtness.g);
    float metallic = metallicRoughtness.b;
    float reflectance = 0.4; // constant
    vec3 diffuseColor = (1.0 - metallic) * vec3(baseColor);

    vec3 f0 = 0.16 * reflectance * reflectance * (1.0 - metallic) + diffuseColor * metallic;

    vec3 finalColor = vec3(0.0);
    for (int i = 0; i < lightCount; i++) {
        Light light = lights[i];

        // Lighting
        vec3 lightDir = normalize(light.position - inWorldPos);
        float NoL = clamp(dot(normal, lightDir), 0.0, 1.0);

        vec3 lightColor = pbrBRDF(lightDir, viewDir, normal, roughness, f0, diffuseColor) * NoL * light.color;

        // Shadow mapping
        float visibility = 1.0;
        if (scene_data.shadowMapIndex > -1) {
            vec4 lightSpace = light.mvp * vec4(inWorldPos, 1.0);
            vec3 projCoords = lightSpace.xyz / lightSpace.w;

            vec2 coords = (projCoords.xy * 0.5 + 0.5);
            float closestDepth = TEX(scene_data.shadowMapIndex, coords).r;
            float currentDepth = projCoords.z;

            float bias = max(0.0005 * (1.0 - NoL), 0.0001);

            // poisson sampling
            for (int i = 0; i < 4; i++) {
                if (TEX(scene_data.shadowMapIndex, coords + poissonDisk[i] / 5000.0).r > currentDepth - bias) {
                    visibility -= 0.2;
                }
            }
        }

        finalColor += lightColor * (NoL * visibility);
    }

    float ambient = 0.05;
    finalColor += vec3(baseColor) * ambient;
    finalColor += emissive;

    fragColor = vec4(finalColor, 1.0);
}
