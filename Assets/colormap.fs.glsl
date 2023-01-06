#version 410

layout(location = 0) out vec4 frag_position;
layout(location = 1) out vec4 frag_normal;
layout(location = 2) out vec4 frag_ambient;
layout(location = 3) out vec4 frag_diffuse;
layout(location = 4) out vec4 frag_specular;
layout(location = 5) out vec4 frag_emissive;
layout(location = 6) out vec4 frag_shadow;
layout(location = 7) out vec4 frag_ptShadow;

uniform sampler2D tex_diffuse;
uniform sampler2D tex_normal;
uniform sampler2D tex_disp;
uniform sampler2D tex_shadow;
uniform samplerCube tex_ptShadow;

uniform bool isTexAttached;
uniform bool isTextureNormal;
uniform bool isBump = false;

uniform vec3 u3fvKa;
uniform vec3 u3fvKd;
uniform vec3 u3fvKs;
uniform vec3 u3fvKe;

uniform bool isPtLightShowed = false;
uniform vec3 ptLightPos = vec3(0.f);
uniform vec4 ptLightTex = vec4(1.f);

uniform mat4 um4m;

in VS_OUT
{
    vec4 position;
    vec2 texcoord;
    vec4 normal;
    mat3 TBN;
    vec4 shadowcoord;
} fs_in;

void shadow_test();

void main()
{
    frag_position = fs_in.position;
    if(isTextureNormal)
    {
        if(isBump)
        {   
            vec3 normal = texture(tex_normal, fs_in.texcoord).xyz * 2 - vec3(1.f);
            normal = 0.5f * (normalize(mat3(transpose(inverse(um4m))) * fs_in.TBN * normal)) + vec3(0.5f);
            frag_normal = vec4(normal, 1.f);
        }
         else
        {
            frag_normal = fs_in.normal;
        }
    }
    else
    {
        frag_normal = fs_in.normal;
    }

    frag_ambient = vec4(u3fvKa, 1.f);

    if(isTexAttached)
    {
        vec4 color = texture(tex_diffuse, fs_in.texcoord);
        if (color.a < 0.5f)
            discard;
        frag_diffuse = color * texture(tex_disp, fs_in.texcoord);
//        frag_diffuse = color;
    }
    else
    {
        frag_diffuse = vec4(u3fvKd, 1.f);
    }

    frag_specular = vec4(u3fvKs, 1.f);

    frag_emissive = vec4(u3fvKe, 1.f);

    shadow_test();
//    frag_shadow = texture(tex_shadow, fs_in.shadowcoord.xy);
}

void shadow_test()
{
    // get vector between fragment position and light position
    vec3 fragToLight = fs_in.position.xyz - ptLightPos;
    // ise the fragment to light vector to sample from the depth map    
    float closestDepth = texture(tex_ptShadow, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= 5.f;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
//    float bias = 0.01; // we use a much larger bias since depth is now in [near_plane, far_plane] range
//    float shadow = currentDepth -  0.05 > closestDepth ? 0.5f : 1.f;        
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    

    bool dirShadowTest = fs_in.shadowcoord.z - 0.005 < texture(tex_shadow, fs_in.shadowcoord.xy).z;
    bool ptShadowTest = currentDepth - 0.009 < closestDepth;

    if(dirShadowTest)
    {  
        frag_shadow = vec4(1.f);
    }
    else
    {
        frag_shadow = vec4(0.4f);
    }
    if (isPtLightShowed) {
        if(ptShadowTest)
        {  
            frag_ptShadow = vec4(1.f);
        }
        else
        {
            frag_ptShadow = vec4(0.f);
        }
    }
}