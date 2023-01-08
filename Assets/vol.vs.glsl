#version 410 core

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec2 iv2tex_coord;
layout(location = 2) in vec3 iv3normal;
layout(location = 3) in vec3 iv3tangent;
layout(location = 4) in vec3 iv3bitangent;

uniform mat4 um4m;
uniform mat4 um4v;
uniform mat4 um4p;

out VS_OUT
{
    vec4 position;
    vec2 texcoord;
    vec4 normal;
    mat3 TBN;
    vec4 shadowcoord;
} vs_out;


void main()
{
	gl_Position = um4p * um4v * um4m * vec4(iv3vertex, 1.0);
    vs_out.position = um4m * vec4(iv3vertex, 1.f);
}