#version 410 core

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 hdrColor;

uniform sampler2D position_map;
uniform sampler2D normal_map;
uniform sampler2D ambient_map;
uniform sampler2D diffuse_map;
uniform sampler2D specular_map;
uniform sampler2D emissive_map;
uniform sampler2D shadow_map;
uniform sampler2D ptShadow_map;
uniform sampler2D depth_map;
uniform sampler2D noise_map;

uniform int shadingCase;
uniform bool SSAOCase;
uniform vec3 directLightVec = vec3(0.f);
uniform vec3 pointLightPosition = vec3(0.f);
uniform vec3 pointLightColor = vec3(1.f, 1.f, 1.f);
uniform vec3 eyePosition = vec3(0.f);
uniform mat4 view_matirx = mat4(0.f);
uniform mat4 proj_matrix = mat4(0.f);
uniform mat4 inv_proj_matrix = mat4(0.f);
uniform vec2 noise_scale = vec2(0.f);

in VS_OUT
{
	vec2 texcoord;
} fs_in;


layout(std140) uniform Kernals                                                                  
{                                                                                               
	vec4 kernals[128];                                                                           
};  


bool emissive();
vec4 PhongShading();
vec4 SSAO();

void main()
{
	vec4 position = texelFetch(position_map, ivec2(gl_FragCoord.xy), 0);
	vec4 ambient = texelFetch(ambient_map, ivec2(gl_FragCoord.xy), 0);
	vec4 diffuse = texelFetch(diffuse_map, ivec2(gl_FragCoord.xy), 0);
	vec4 specular = texelFetch(specular_map, ivec2(gl_FragCoord.xy), 0);
	vec4 depth = texelFetch(depth_map, ivec2(gl_FragCoord.xy), 0);

	vec4 color = vec4(0.f);
	switch(shadingCase)
	{
	case 0:
		if (emissive())
		{
			color = 0.3f * ambient + 0.7f * diffuse;
		}
		else
		{
			color = PhongShading();
		}
		fragColor = color;

//			fragColor = 0.45f * ambient * diffuse + 0.45f * diffuse + 0.1f * specular;
		break;
	case 1:
		fragColor = texelFetch(position_map, ivec2(gl_FragCoord.xy), 0);
//			fragColor = texelFetch(shadow_map, ivec2(gl_FragCoord.xy), 0);
		break;
	case 2:
		fragColor = texelFetch(normal_map, ivec2(gl_FragCoord.xy), 0);
//			fragColor = vec4(texelFetch(normal_map, ivec2(gl_FragCoord.xy), 0).xyz * 2 - vec3(1.f), 1.f);
		break;
	case 3:
		fragColor = ambient;
		break;
	case 4:
		fragColor = diffuse;
		break;
	case 5:
		fragColor = specular;
		break;
	case 6:
		fragColor = SSAO();
		break;
	default:
		fragColor = 0.2f * ambient + 0.7f * diffuse + 0.1f * specular;
		break;
	}

    float brightness = dot(color.rgb, vec3(0.7152, 0.3126, 0.0722));
    if(brightness > 1.0)
        hdrColor = vec4(color.rgb, 1.0);
    else
        hdrColor = vec4(0.0, 0.0, 0.0, 1.0);
}

bool emissive()
{
	return (texelFetch(emissive_map, ivec2(gl_FragCoord.xy), 0).r == 1.f);
}

vec4 PhongShading()
{
	vec3 position = texelFetch(position_map, ivec2(gl_FragCoord.xy), 0).xyz;
	vec3 N = normalize(texelFetch(normal_map, ivec2(gl_FragCoord.xy), 0).xyz * 2 - vec3(1.f)); // 0~1 -> -1~1
	vec3 ambient_albedo = texelFetch(ambient_map, ivec2(gl_FragCoord.xy), 0).rgb;
	vec3 diffuse_albedo = texelFetch(diffuse_map, ivec2(gl_FragCoord.xy), 0).rgb;
	vec3 specular_albedo = texelFetch(specular_map, ivec2(gl_FragCoord.xy), 0).rgb;
	vec3 shadow_albedo = texelFetch(shadow_map, ivec2(gl_FragCoord.xy), 0).rgb;
	vec3 ptShadow_albedo = texelFetch(ptShadow_map, ivec2(gl_FragCoord.xy), 0).rgb;
	
	vec3 color = vec3(0.f);

	vec3 L = normalize(-directLightVec);
	vec3 V = normalize(eyePosition - position);
	vec3 R = reflect(-L, N);
	vec3 H = normalize(L + V);

	vec3 ambient = ambient_albedo * diffuse_albedo;
	vec3 diffuse = vec3(5.f) * max(dot(N, L), 0.f) * diffuse_albedo;
	vec3 specular = pow(max(dot(N, H), 0.f), 255.f) * specular_albedo;
	//vec3 specular = pow(max(dot(R, V), 0.f), 900.f) * specular_albedo;
	
	if(SSAOCase)
	{
		color += 0.1f * ambient * SSAO().xyz + (0.7f * diffuse + 0.2f * specular) * shadow_albedo;
	}
	else
	{
		color += 0.1f * ambient + (0.7f * diffuse + 0.2f * specular) * shadow_albedo;
	}
	// drawing ptLight effect
	float dist = length(pointLightPosition - position);
	L = normalize(pointLightPosition - position);
//	H = normalize(L + V);
	R = reflect(-L, N);
	diffuse = pointLightColor * max(dot(N, L), 0.f) * diffuse_albedo;
//	specular = pow(max(dot(N, H), 0.f), 255.f) * specular_albedo;
	specular = pow(max(dot(R, V), 0.f), 900.f) * specular_albedo;
	//float attenuation = 3.0f / (pow(dist, 2.0) + 1.f);
	float attenuation = 1.f + 0.7f * dist + 0.14 * dist * dist;

	color += (diffuse + specular) / attenuation  * ptShadow_albedo;

	// drawing areaLight effect


	return vec4(color, 1.f);
}

vec4 SSAO()
{
	float depth = texture(depth_map, fs_in.texcoord).r;
	if (depth == 1.0)
	{
		discard;
	}                                                                                                                      
	vec4 depth_position = inv_proj_matrix * vec4(vec3(fs_in.texcoord, depth) * 2.0 - 1.0, 1.0);               
	depth_position /= depth_position.w;
	
	vec3 N = normalize((view_matirx * (texture(normal_map, fs_in.texcoord) * 2 - vec4(1.f))).xyz);                                           
	vec3 randvec = normalize(texture(noise_map, fs_in.texcoord * noise_scale).xyz * 2.0 - 1.0); 
	vec3 T = normalize(randvec - N * dot(randvec, N));                                          
	vec3 B = cross(N, T);                                                                       
	mat3 tbn = mat3(T, B, N); // tangent to eye matrix                                          
	const float radius = 2.0;                                                                   
	float ao = 0.0;
	
	for(int i = 0; i < 128; ++i)                                                                 
	{                                                                                           
	    vec4 sampleEye = depth_position + vec4(tbn * kernals[i].xyz * radius, 0.0);                   
	    vec4 sampleP = proj_matrix * sampleEye;                                                        
	    sampleP /= sampleP.w;                                                                   
	    sampleP = sampleP * 0.5 + 0.5;                                                          
	    float sampleDepth = texture(depth_map, sampleP.xy).r;                                   
	    vec4 invP = inv_proj_matrix * vec4(vec3(sampleP.xy, sampleDepth) * 2.0 - 1.0, 1.0);             
	    invP /= invP.w;                                                                         
	    if(sampleDepth > sampleP.z || length(invP - depth_position) > radius)                         
	    {                                                                                       
	        ao += 1.0;                                                                          
	    }                                                                                       
	}                                                                                           
	return vec4(vec3(ao / 128.0), 1.0);
}