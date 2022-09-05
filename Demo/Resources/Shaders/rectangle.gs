#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat3 cameraTransform = mat3(1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0);

in VS_OUT
{
	mat3 transform;
	vec4 color;
	vec4 uvs;
	vec2 pivot;
	float depth;
	int textureId; // ignore
} gs_in[];

out GS_OUT
{
	vec4 color;
	vec2 uvCoord;
} gs_out;



void main()
{
	// Set the color for all output vertices
	gs_out.color = gs_in[0].color;
	
	vec2 centerPos2D = gl_in[0].gl_Position.xy;
	
	gl_Position = vec4(cameraTransform * (gs_in[0].transform * vec3(centerPos2D + vec2( 1,-1) + gs_in[0].pivot, 1)), 1);
	gl_Position.z = gs_in[0].depth;
	gs_out.uvCoord = gs_in[0].uvs.yz;// vec2(1,0);
	EmitVertex();
	
	gl_Position = vec4(cameraTransform * (gs_in[0].transform * vec3(centerPos2D + vec2(-1,-1) + gs_in[0].pivot, 1)), 1);
	gl_Position.z = gs_in[0].depth;
	gs_out.uvCoord = gs_in[0].uvs.xz;// vec2(0,0);
	EmitVertex();
	
	gl_Position = vec4(cameraTransform * (gs_in[0].transform * vec3(centerPos2D + vec2( 1, 1) + gs_in[0].pivot, 1)), 1);
	gl_Position.z = gs_in[0].depth;
	gs_out.uvCoord = gs_in[0].uvs.yw;// vec2(1,1);
	EmitVertex();
	
	gl_Position = vec4(cameraTransform * (gs_in[0].transform * vec3(centerPos2D + vec2(-1, 1) + gs_in[0].pivot, 1)), 1);
	gl_Position.z = gs_in[0].depth;
	gs_out.uvCoord = gs_in[0].uvs.xw;// vec2(0,1);
	EmitVertex();
	
	EndPrimitive();
}