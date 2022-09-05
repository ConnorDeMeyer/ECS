#version 330 core

in GS_OUT
{
	vec4 color;
	vec2 uvCoord;
} fs_in;

out vec4 color;

uniform sampler2D Texture;

void main()
{
	//color = vec4(1,1,1,1);
	//color = fs_in.color * vec4(fs_in.uvCoord.x, fs_in.uvCoord.y, 1, 1);// * texture(Texture, fs_in.uvCoord);
	color = fs_in.color * texture(Texture, fs_in.uvCoord);
}