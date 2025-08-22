#version 450

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : require

struct Vertex
{
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
};

struct Material
{
    int baseColorIdx;
    int metallicRoughnessIdx;
    int normalIdx;
    int emissiveIdx;

    vec4 baseColorFactor;
    float metallicFactor;
    float roughnessFactor;
    float _pad0;
    float _pad1;
};

const uint LIGHT_TYPE_DIRECTIONAL = 1;
const uint LIGHT_TYPE_POINT = 2;
const uint LIGHT_TYPE_SPOT = 3;

struct Light
{
    mat4 mvp;
    vec3 color;
    vec3 position;
    vec3 direction; // only for directional light
    int type; // enum LightType
    float cutOff; // only for spot light
};

layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inTangent;
layout (location = 4) in mat3 inTBN;

layout (location = 0) out vec4 fragColor;

layout (buffer_reference, std430) readonly buffer VertexBuffer { Vertex vertices[]; };

layout (push_constant) uniform PushConstant
{
    mat4 worldMatrix;
    VertexBuffer vertexBuffer;
    int materialIdx;
} pushConstant;

layout (binding = 0) uniform UBO
{
    mat4 projection;
    mat4 view;
    vec4 cameraPosAndLightNum; // vec4 -> vec3 (camera position) / int (number of lights)
} ubo;

layout (binding = 1) readonly buffer MaterialsBuffer
{
    Material materials[];
};

layout (binding = 2) readonly buffer LightsBuffer
{
    Light lights[];
};

layout (binding = 3) uniform sampler2D textures[];

layout (binding = 4) uniform sampler2D shadowMap;

#define TEX(id, uv) texture(textures[nonuniformEXT(id)], uv)
#define PI 3.14159265359

// https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
vec2 poissonDisk[4] = vec2[](
    vec2(-0.94201624, -0.39906216),
    vec2(0.94558609, -0.76890725),
    vec2(-0.094184101, -0.92938870),
    vec2(0.34495938, 0.29387760)
);

// https://google.github.io/filament/Filament.md.html
float D_GGX(float NoH, float a)
{
    float a2 = a * a;
    float f = (NoH * a2 - NoH) * NoH + 1.0;
    return a2 / (PI * f * f);
}

vec3 F_Schlick(float u, vec3 f0)
{
    return f0 + (vec3(1.0) - f0) * pow(1.0 - u, 5.0);
}

float V_SmithGGXCorrelated(float NoV, float NoL, float a)
{
    float a2 = a * a;
    float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    return 0.5 / (GGXV + GGXL);
}

float Fd_Lambert()
{
    return 1.0 / PI;
}

vec3 BRDF(vec3 l, vec3 v, vec3 n, float perceptualRoughness, vec3 f0, vec3 diffuseColor)
{
    vec3 h = normalize(v + l);

    float NoV = abs(dot(n, v)) + 1e-5;
    float NoL = clamp(dot(n, l), 0.0, 1.0);
    float NoH = clamp(dot(n, h), 0.0, 1.0);
    float LoH = clamp(dot(l, h), 0.0, 1.0);

    float roughness = perceptualRoughness * perceptualRoughness;

    float D = D_GGX(NoH, roughness);
    vec3  F = F_Schlick(LoH, f0);
    float V = V_SmithGGXCorrelated(NoV, NoL, roughness);

    // specular BRDF
    vec3 Fr = (D * V) * F;

    // diffuse BRDF
    vec3 Fd = diffuseColor * Fd_Lambert();

    return (Fr + Fd);
}

void main()
{
    vec3 cameraPos = ubo.cameraPosAndLightNum.xyz;
    uint lightCount = uint(ubo.cameraPosAndLightNum.w);

    vec3 baseColor = vec3(0);
    vec4 metallicRoughtness = vec4(0);
    vec3 normal = inNormal;
    vec3 emissive = vec3(0);

    if (pushConstant.materialIdx > -1) {
        Material material = materials[pushConstant.materialIdx];

        if (material.baseColorIdx > -1) {
            baseColor = (TEX(material.baseColorIdx, inUV) * material.baseColorFactor).rgb;
        }

        if (material.metallicRoughnessIdx > -1) {
            metallicRoughtness = TEX(material.metallicRoughnessIdx, inUV);
            metallicRoughtness.g *= material.roughnessFactor; // roughness
            metallicRoughtness.b *= material.metallicFactor; // metallic
        }

        if (material.normalIdx > -1) {
            normal = normalize(TEX(material.normalIdx, inUV).rgb);
        }

        if (material.emissiveIdx > -1) {
            emissive = TEX(material.emissiveIdx, inUV).rgb;
        }
    }

    if (inTangent != vec4(0.0)) {
        normal = inTBN * normalize(normal * 2.0 - 1.0);
        normal = normalize(normal);
    }

    vec3 viewDir = normalize(cameraPos - inWorldPos);

    float roughness = max(0.05, metallicRoughtness.g);
    float metallic = metallicRoughtness.b;
    float reflectance = 0.5; // constant

    vec3 f0 = 0.16 * reflectance * reflectance * (1.0 - metallic) + baseColor * metallic;

    vec3 lightColor = vec3(0);
    float shadow = 1.0;

    for (int i = 0; i < lightCount; i++) {
        Light light = lights[i];

        // Lighting
        vec3 lightDir = normalize(light.position - inWorldPos);
        float NoL = clamp(dot(normal, lightDir), 0.0, 1.0);
        vec3 diffuseColor = (1.0 - metallic) * baseColor;

        lightColor += BRDF(lightDir, viewDir, normal, roughness, f0, diffuseColor) * light.color;

        // Shadow mapping
        vec4 lightSpace = light.mvp * vec4(inWorldPos, 1.0);
        vec3 projCoords = lightSpace.xyz / lightSpace.w;

        vec2 coords = (projCoords.xy * 0.5 + 0.5);
        float closestDepth = texture(shadowMap, projCoords.xy).r;
        float currentDepth = projCoords.z;

        // poisson sampling
        float bias = max(0.0005 * (1.0 - NoL), 0.0001);
        for (int i = 0; i < 4; i++) {
            if (texture(shadowMap, coords + poissonDisk[i] / 700.0).r  < currentDepth - bias) {
                shadow -= 0.2;
            }
        }
    }

    // FIXME: shadows are broken...
    // vec3 color = (baseColor * shadow) + lightColor + emissive;
    vec3 color = baseColor + lightColor + emissive;

    fragColor = vec4(color, 1.0);
}
