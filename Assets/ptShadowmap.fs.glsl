#version 410
in vec4 FragPos;

uniform vec3 ptLightPos;
uniform float far_plane = 5.f;

void main()
{
    float lightDistance = length(FragPos.xyz - ptLightPos);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;
    
    // write this as modified depth
    gl_FragDepth = lightDistance;
}