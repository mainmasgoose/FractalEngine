#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outTexCoord;
layout(location = 3) out vec3 outTangent;

layout(set = 1, binding = 0) uniform FrameData {
    mat4 viewProj;
    vec3 cameraPos;
} fd;

layout(push_constant) uniform PushConstants {
    mat4 model;
} pc;

void main() {
    vec4 worldPos = pc.model * vec4(inPosition, 1.0);
    outWorldPos = worldPos.xyz;
    outNormal = mat3(pc.model) * inNormal;
    outTangent = mat3(pc.model) * inTangent;
    outTexCoord = inTexCoord;
    
    gl_Position = fd.viewProj * worldPos;
}
