#version 410 core

layout(location = 0) out vec4 fragColor;

uniform sampler2D main_texture;
uniform sampler2D hdr_texture;
uniform sampler2D vol_texture;

in VS_OUT
{
	vec2 texcoord;
} fs_in;

uniform int isBloom = 0;
uniform vec2 lightPosOnScreen = vec2(0);
vec3 bloom();

void main()
{

	if (isBloom == 1)
	{
//		fragColor =	0.8f * texture(main_texture, fs_in.texcoord) + 0.25f * bloom();
		vec3 sceneColor = texture(main_texture, fs_in.texcoord).rgb;
		vec3 bloomColor = bloom();
		fragColor = vec4(vec3(sceneColor + bloomColor * 0.6), 1.0);

	}
	else if (isBloom == 2)
	{
//		fragColor = texture(vol_texture, fs_in.texcoord);

		vec3 sceneColor = texture(main_texture, fs_in.texcoord).rgb;
		int NUM_SAMPLES = 100;
		float exposure = 0.2;
		float decay = 0.96815;
		float density = 0.926;
		float weight = 0.4;

		float sampleWeight = 0.58767;

		vec2 tc = fs_in.texcoord;
		vec2 deltaTexCoord = tc - lightPosOnScreen.xy;
		deltaTexCoord *= (1.0 / float(NUM_SAMPLES) * density);

		float illuminationDecay = 1.0;
		
		vec4 volColor = texture2D(vol_texture, tc);

		for(int i=0; i< NUM_SAMPLES; i++){
			tc -= deltaTexCoord;
			vec4 sampleVec = texture2D(vol_texture, tc.xy) * illuminationDecay * weight;
			volColor += sampleVec;
			illuminationDecay *= decay;
		}
		volColor *= exposure;
		fragColor = vec4(sceneColor, 1.0f) + volColor * sampleWeight;


	}
	else
	{
		fragColor = texture(main_texture, fs_in.texcoord);
//		fragColor = ssao();
	}
}

vec3 bloom()
{
	int half_size = 6;
	vec3 color_sum = vec3(0.f);

	for (int i = -half_size; i <= half_size; i++)
	{
		for (int j = -half_size; j <= half_size; j++)
		{
			ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
			color_sum += texelFetch(hdr_texture, coord, 0).rgb;
		}
	}

	int sample_count = (half_size * 2 + 1) * (half_size * 2 + 1);
	
	return color_sum / sample_count;
}