#version 450

//shader input
// layout (location = 0) in vec2 texCoord;

//output write

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

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
	float dist = dot(fragOffset,fragOffset);

	if (dist > 1.0) discard;

	outColor = vec4(fragColor, 1.0);
}
