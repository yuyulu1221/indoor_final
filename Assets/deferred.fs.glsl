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
uniform vec3 areaLightPos = vec3(0.f);
uniform float areaLightWidth = .1f;
uniform float areaLightHeight = .1f;
uniform vec3 areaLightDir = vec3(.8f, .6f, .0f);
uniform vec3 eyePosition = vec3(0.f);
uniform mat4 view_matrix = mat4(0.f);
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

vec3 CalculatePlaneIntersection(vec3 viewPosition, vec3 reflectionVector, vec3 lightDirection, vec3 rectangleLightCenter)
{
   return viewPosition + reflectionVector * (dot(lightDirection,rectangleLightCenter-viewPosition)/dot(lightDirection,reflectionVector));
}

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

	color += (0.7 * diffuse + 0.2 * specular) / attenuation  * ptShadow_albedo;

	// drawing areaLight effect

	vec3 p0, p1, p2, p3;
	p0 = areaLightPos + vec3(-areaLightWidth, areaLightHeight, 0.f);
	p1 = areaLightPos + vec3(-areaLightWidth, -areaLightHeight, 0.f);
	p2 = areaLightPos + vec3(areaLightWidth, areaLightHeight, 0.f);
	p3 = areaLightPos + vec3(areaLightWidth, -areaLightHeight, 0.f);

	vec3 v0, v1, v2, v3;
	v0 = p0 - position;
	v1 = p1 - position;
	v2 = p2 - position;
	v3 = p3 - position;

	float facingCheck = dot(v0, cross(p2 - p0, p1 - p0));
	if (facingCheck > 0.0) 
	{
		return vec4(color, 1.f);
	}

	vec3 n0, n1, n2, n3;
	n0 = normalize(cross(v0, v1));
	n1 = normalize(cross(v1, v2));
	n2 = normalize(cross(v2, v3));
	n3 = normalize(cross(v3, v0));

	float g0, g1, g2, g3;
	g0 = acos(dot(-n0, n1));
	g1 = acos(dot(-n1, n2));
	g2 = acos(dot(-n2, n3));
	g3 = acos(dot(-n3, n0));

	float solidAngle = g0 + g1 + g2 + g3 - 2.0 * 3.14159265359;
	float NoL = solidAngle * 0.2 * (
		clamp ( dot( normalize ( v0 ), N ), 0, 1) +
		clamp ( dot( normalize ( v1 ) , N ) , 0, 1)+
		clamp ( dot( normalize ( v2 ) , N ) , 0, 1)+
		clamp ( dot( normalize ( v3 ) , N ) , 0, 1)+
		clamp ( dot( normalize ( areaLightPos - position ) , N ), 0, 1)
	);
	vec3 right = vec3(0.f, 0.f, -1.f);
	vec3 up = vec3(0.f, 1.f, 0.f);

	vec3 intersectPoint = CalculatePlaneIntersection(position, R, areaLightDir, areaLightPos);

	vec3 intersectionVector = intersectPoint - areaLightPos;
	vec2 intersectPlanePoint = vec2(dot(intersectionVector,right), dot(intersectionVector,up));
	vec2 nearest2DPoint = vec2(clamp(intersectPlanePoint.x, -areaLightWidth, areaLightWidth), clamp(intersectPlanePoint.y, -areaLightHeight, areaLightHeight));	

	vec3 specularFactor = vec3(0.f ,0.f, 0.f);
	float specularAmount = dot(R, areaLightDir);
	float surfaceSpec = 1.f;
	float roughness = 0.1f;
	if (specularAmount > 0.0)
	{
		float specFactor = 1.0 - clamp(length(nearest2DPoint - intersectPlanePoint) * pow((1.0 - roughness), 2) * 32.0, 0.0, 1.0);
		specularFactor += surfaceSpec * specFactor * specularAmount * NoL;
	}	
	vec3 nearestPoint = areaLightPos + (right * nearest2DPoint.x + up * nearest2DPoint.y);
	dist = distance(position, nearestPoint);
	//float falloff = 1.0 - clamp(dist / 10.f, 0, 1);	
	attenuation = 1.f + 0.7f * dist + 0.14 * dist * dist;

	float luminosity = 3.f;
	vec3 diffuseFactor = vec3(0.f);
	vec3 lightColor = vec3(0.5f, 0.5f, 0.0f);
	vec3 light = (0.7f * diffuse + 0.2f * specularAmount) / attenuation * lightColor * luminosity;	

	color += light;

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
	
	vec3 N = normalize((view_matrix * (texture(normal_map, fs_in.texcoord) * 2 - vec4(1.f))).xyz);                                           
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