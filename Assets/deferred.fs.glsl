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
uniform bool isPtLightShowed;
uniform vec3 pointLightPosition = vec3(0.f);
uniform vec3 pointLightColor = vec3(.8f, 0.f, .6f);
uniform bool isAreaLightShowed;
uniform vec3 areaLightPos = vec3(0.f);
uniform float areaLightWidth = 4.f;
uniform float areaLightHeight = 2.f;
uniform vec3 areaLightDir = vec3(.0f);
uniform vec3 areaLightColor = vec3(.8f, .6f, .0f);
uniform vec3 eyePosition = vec3(0.f);
uniform vec3 eyeCenter = vec3(0.f);
uniform mat4 view_matrix = mat4(0.f);
uniform mat4 proj_matrix = mat4(0.f);
uniform mat4 inv_proj_matrix = mat4(0.f);
uniform vec2 noise_scale = vec2(0.f);
uniform bool isNPR = true;

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
	// drawing directLight effect
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

//	if(isNPR) {
//		vec2 matCapCoord = ((N * inverse(mat3(view_matrix)) + 1) * 0.5).xy; // "normal" is the unmodified input normal  
//		matCapCoord.y = 1 - matCapCoord.y; // Depending on our environment we need to invert the y coordinate
//
//		float stepCount = 3;
//		float nDotL = dot(N, directLightVec);  
//		float light = floor(nDotL * stepCount) / stepCount; // _Stepcount is an input value that depends on how many color tones we want
//
//		nDotL = dot(N, directLightVec);  
//		float light = texelFetch(depth_map, vec2(nDotL, 0.5));
//
//		nDotL = dot(N, directLightVec);  
//		float nDotV = dot(N, normalize(eyeCenter - eyePosition)); // The basic view dot normal calculation  
//		float fresnel = 1 - nDotV; // We want to invert our fresnel here so that the glow goes to the outside
//
//		float index = texelFetch(ambient_albedo,gl_FragCoord); // Or vertexcolor.r depending on our setup  
//		float color = tex2D(_GradientSampler, float2(light, index));
//	}

	float dist, attenuation;
	// drawing ptLight effect
	if (isPtLightShowed) {
		dist = length(pointLightPosition - position);
		L = normalize(pointLightPosition - position);
	//	H = normalize(L + V);
		R = reflect(-L, N);
		diffuse = pointLightColor * max(dot(N, L), 0.f) * diffuse_albedo;
	//	specular = pow(max(dot(N, H), 0.f), 255.f) * specular_albedo;
		specular = pow(max(dot(R, V), 0.f), 900.f) * specular_albedo;
		//float attenuation = 3.0f / (pow(dist, 2.0) + 1.f);
		attenuation = 1.f + 0.7f * dist + 0.14 * dist * dist;

		color += (0.7 * diffuse + 0.2 * specular) / attenuation  * ptShadow_albedo;
	}

	// drawing areaLight effect
	if (isAreaLightShowed) {
		vec3 up = vec3(0.f, 1.f, 0.f);
		vec3 viewDir = normalize(eyeCenter - eyePosition);
		vec3 viewRight = normalize(cross(viewDir, up));
		vec3 viewUp = normalize(cross(viewRight, viewDir));
		vec3 areaLightRight = normalize(cross(areaLightDir, up));
		L = normalize(areaLightPos - position);
		R = reflect(-L, N);
		vec3 p0, p1, p2, p3;
		p0 = areaLightPos + vec3(0.f, areaLightHeight, 0.f) - areaLightRight * areaLightWidth;
		p1 = areaLightPos + vec3(0.f, -areaLightHeight, 0.f) - areaLightRight * areaLightWidth;
		p2 = areaLightPos + vec3(0.f, -areaLightHeight, 0.f) + areaLightRight * areaLightWidth;
		p3 = areaLightPos + vec3(0.f, areaLightHeight, 0.f) + areaLightRight * areaLightWidth;

		vec3 v0, v1, v2, v3;
		v0 = p0 - position;
		v1 = p1 - position;
		v2 = p2 - position;
		v3 = p3 - position;

		float facingCheck = dot(v0, cross(p3 - p0, p1 - p0));
		if (facingCheck > 0.f) 
		{
			return vec4(color, 1.f);
		}

		vec3 n0, n1, n2, n3;
		n0 = normalize(cross(v0, v1));
		n1 = normalize(cross(v1, v2));
		n2 = normalize(cross(v2, v3));
		n3 = normalize(cross(v3, v0));

		float g0, g1, g2, g3;
		g0 = acos(dot(n0, -n1));
		g1 = acos(dot(n1, -n2));
		g2 = acos(dot(n2, -n3));
		g3 = acos(dot(n3, -n0));

		float solidAngle = g0 + g1 + g2 + g3 - 2.0 * 3.14159265359;
		float NoL = solidAngle * 0.2 * (
			clamp ( dot( normalize ( v0 ), N ), 0.f, 1.f) +
			clamp ( dot( normalize ( v1 ) , N ) , 0.f, 1.f)+
			clamp ( dot( normalize ( v2 ) , N ) , 0.f, 1.f)+
			clamp ( dot( normalize ( v3 ) , N ) , 0.f, 1.f)+
			clamp ( dot( normalize ( areaLightPos - position ) , N ), 0.f, 1.f)
		);
	

		vec3 intersectPoint = CalculatePlaneIntersection(position, R, areaLightDir, areaLightPos);
		vec3 intersectionVector = intersectPoint - areaLightPos;
		vec2 intersectPlanePoint = vec2(dot(intersectionVector,areaLightRight), dot(intersectionVector,up));
		vec2 nearest2DPoint = vec2(clamp(intersectPlanePoint.x, -areaLightWidth, areaLightWidth), clamp(intersectPlanePoint.y, -areaLightHeight, areaLightHeight));	

		vec3 specularFactor = vec3(0.f ,0.f, 0.f);
		float specularAmount = dot(R, areaLightDir);
		float roughness = .9f;
		float effectFactor = 0.f;
		if (specularAmount > 0.f)
		{
			float specFactor = 1.0 - clamp(length(nearest2DPoint - intersectPlanePoint) * pow((1.0 - roughness), 2) * 32.0, 0.0, 1.0);
			//effectFactor += specFactor * specularAmount * NoL;
			specularFactor = specular_albedo * specFactor * specularAmount * NoL;
		}	
		vec3 nearestPoint = areaLightPos + (areaLightRight * nearest2DPoint.x + up * nearest2DPoint.y);
		dist = distance(position, nearestPoint);
		vec3 nL = normalize(vec3(nearestPoint - position));
		vec3 nR = reflect(-nL, N);
		float luminosity = 2.f;
		attenuation = 1.f + 0.7f * dist + 0.14 * dist * dist;
		//diffuse = areaLightColor * max(dot(N, nL), 0.f)  * diffuse_albedo;
		diffuse = areaLightColor * max(dot(N, L), 0.f)  * diffuse_albedo * specularFactor * 10.f;
		specular = areaLightColor * pow(max(dot(R, V), 0.f), 900.f) * effectFactor * specular_albedo ;
		vec3 light = (0.7f * diffuse + 0.2f * specular) / attenuation * luminosity;	
		
		color += light;
	}

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