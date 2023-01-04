#version 410

layout(location = 0) out vec4 frag_depth;

void main()
{
    float depth = gl_FragCoord.z;
    frag_depth = vec4(vec3(depth), 1.0f);
}
