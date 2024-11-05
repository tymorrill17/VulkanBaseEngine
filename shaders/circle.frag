#version 450

//shader input
// layout (location = 0) in vec2 texCoord;

//output write
layout (location = 0) out vec4 fragColor;

const vec3 color = vec3(1.0f, 1.0f, 1.0f);

void main() 
{
	vec2 uv = 2.0*(gl_FragCoord.xy - 0.5);
	//vec2 centered = 2.0f * (texCoord - 0.5);
	//if (dot(centered, centered) > 1.0f) {
	if (dot(uv, uv) > 1.0f) {
		discard;
	}
	fragColor = vec4(color, 1.0f);
}
