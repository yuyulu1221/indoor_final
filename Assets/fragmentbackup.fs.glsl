#version 410

layout(location = 0) out vec4 fragColor;

uniform bool isTexAttached;
uniform vec3 u3fvKd;

in VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
    vec2 texcoord;
} vertexData;

in VS_OUT
{
    vec3 normal;
} fs_in;

uniform sampler2D tex_diffuse;

void main()
{
    if(isTexAttached)
    {
        vec4 color = texture(tex_diffuse, vertexData.texcoord).rgba;
        if (color.a < 0.5f)
            discard;
        fragColor = color;
    }
    else
    {
        fragColor = vec4(u3fvKd, 1.0f);
    }

}