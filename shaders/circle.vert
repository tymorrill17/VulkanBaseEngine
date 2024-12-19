#version 450
#extension GL_KHR_vulkan_glsl : enable

const int NUM_OFFSETS = 6;

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragOffset;

const int NUM_PARTICLES = 1;

layout (set = 0, binding = 0) readonly buffer ParticleInfo {
	vec2 positions[NUM_PARTICLES];
	vec2 velocities[NUM_PARTICLES];
	vec3 colors[NUM_PARTICLES];
	vec3 defaultColor;
	float radius;
} pinfo;

void main() 
{
	fragOffset = OFFSETS[gl_VertexIndex % NUM_OFFSETS];
	fragColor = pinfo.colors[gl_VertexIndex / NUM_OFFSETS];
	vec3 worldPosition = vec3(pinfo.positions[gl_VertexIndex / NUM_OFFSETS], 0.0) + pinfo.radius*fragOffset.x + pinfo.radius*fragOffset.y;
	gl_Position = vec4(worldPosition, 1.0);
}
