#version 410

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec2 iv2tex_coord;
layout(location = 2) in vec3 iv3normal;

uniform mat4 areaLightViewM;

void main()
{
    vec3 v0 = mat3(areaLightViewM) * iv3vertex;
    vec3 v1 = input.lightPositionView2.xyz - positionView;
    vec3 v2 = input.lightPositionView3.xyz - positionView;
    vec3 v3 = input.lightPositionView4.xyz - positionView; 

    float facingCheck = dot( v0, cross( ( input.lightPositionView3.xyz - input.lightPositionView.xyz ).xyz, ( input.lightPositionView2.xyz - input.lightPositionView.xyz ).xyz ) );

    if (facingCheck > 0.0) 
    {
    return float4(0.0, 0.0, 0.0, 1.0);
}
    gl_Position = areaLightViewM * vec4(iv3vertex, 1.0);
}