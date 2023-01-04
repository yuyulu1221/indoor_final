#version 410

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec2 iv2tex_coord;
layout(location = 2) in vec3 iv3normal;
layout(location = 3) in vec3 iv3tangent;
layout(location = 4) in vec3 iv3bitangent;

uniform mat4 um4m;
uniform mat4 um4v;
uniform mat4 um4p;
uniform mat4 shadow_matrix;

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

    vec3 noraml = 0.5f * (normalize(mat3(transpose(inverse(um4m))) * iv3normal) + vec3(1.f)); 
    vs_out.normal = vec4(noraml, 0.f);
    vs_out.texcoord = iv2tex_coord;
    vs_out.TBN = mat3(iv3tangent, iv3bitangent, iv3normal);
    vs_out.position = um4m * vec4(iv3vertex, 1.f);
    vs_out.shadowcoord = shadow_matrix * um4m * vec4(iv3vertex, 1.0);
}