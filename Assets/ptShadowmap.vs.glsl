#version 410

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec2 iv2tex_coord;
layout(location = 2) in vec3 iv3normal;

uniform mat4 ptLightViewM;

void main()
{
    gl_Position = ptLightViewM * vec4(iv3vertex, 1.0);
}