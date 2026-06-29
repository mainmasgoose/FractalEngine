#version 450

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D globalTextures[1000];

layout(set = 1, binding = 0) uniform FrameData {
    mat4 viewProj;
    vec3 cameraPos;
} fd;

layout(push_constant) uniform MaterialConstants {
    layout(offset = 64) vec4 baseColor;
    float metallic;
    float roughness;
    float baseColorTexIdx;
    float normalTexIdx;
    float metallicRoughnessTexIdx;
    float occlusionTexIdx;
} mc;

const vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
const vec3 lightColor = vec3(1.0, 1.0, 1.0);

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159265359 * denom * denom;
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 N = normalize(inNormal);
    vec3 T = normalize(inTangent);
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    vec3 V = normalize(fd.cameraPos - inWorldPos);
    vec3 L = lightDir;
    vec3 H = normalize(V + L);

    vec4 albedo = mc.baseColor;
    if (mc.baseColorTexIdx >= 0.0) {
        albedo *= texture(globalTextures[int(mc.baseColorTexIdx)], inTexCoord);
    }

    float metallic = mc.metallic;
    float roughness = mc.roughness;
    if (mc.metallicRoughnessTexIdx >= 0.0) {
        vec4 mrSample = texture(globalTextures[int(mc.metallicRoughnessTexIdx)], inTexCoord);
        metallic *= mrSample.b;
        roughness *= mrSample.g;
    }

    vec3 normal = N;
    if (mc.normalTexIdx >= 0.0) {
        vec3 normalMap = texture(globalTextures[int(mc.normalTexIdx)], inTexCoord).rgb * 2.0 - 1.0;
        normal = normalize(TBN * normalMap);
    }

    float ao = 1.0;
    if (mc.occlusionTexIdx >= 0.0) {
        ao = texture(globalTextures[int(mc.occlusionTexIdx)], inTexCoord).r;
    }

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo.rgb, metallic);

    float NDF = DistributionGGX(normal, H, roughness);
    float G = GeometrySmith(normal, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(normal, L), 0.0);
    vec3 diffuse = kD * albedo.rgb / 3.14159265359;
    
    vec3 ambient = vec3(0.05) * albedo.rgb;
    vec3 color = (diffuse + specular) * lightColor * NdotL * ao + ambient;
    
    // HDR tone mapping and gamma correction

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    outColor = vec4(color, albedo.a);
}
