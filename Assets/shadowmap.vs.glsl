#version 410

layout(location = 0) in vec3 iv3vertex;

uniform mat4 lightViewVP;
uniform mat4 lightViewM;

void main()
{
	gl_Position = lightViewVP * lightViewM * vec4(iv3vertex, 1.f);
}