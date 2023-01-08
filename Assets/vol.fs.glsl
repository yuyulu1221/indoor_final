#version 410 core

layout(location = 0) out vec4 fragColor;

uniform sampler2D main_texture;


in VS_OUT
{
    vec4 position;
    vec2 texcoord;
    vec4 normal;
    mat3 TBN;
    vec4 shadowcoord;
} fs_in;
void main()
{
	fragColor = texture(main_texture, fs_in.texcoord);

}