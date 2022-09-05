#version 330 core

layout (location = 0) in mat3 transform;
layout (location = 3) in vec4 color;
layout (location = 4) in vec4 uvs;
layout (location = 5) in vec2 pivot;
layout (location = 6) in float depth;
layout (location = 7) in int textureId;

// in VS_IN
// {
	// mat3 transform;
	// vec4 color;
// } vs_in;

out VS_OUT
{
	mat3 transform;
	vec4 color;
	vec4 uvs;
	vec2 pivot;
	float depth;
	int textureId; // ignore
} vs_out;

void main()
{
	vs_out.transform = transform;
	vs_out.color = color;
	gl_Position = vec4(transform[0][2], transform[1][2], 1, 1);
	vs_out.uvs = uvs;
	vs_out.pivot = pivot;
	vs_out.depth = depth;
	vs_out.textureId = textureId;
}