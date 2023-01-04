#version 410 core

layout(location = 0) out vec4 fragColor;

uniform sampler2D main_texture;
uniform sampler2D hdr_texture;

in VS_OUT
{
	vec2 texcoord;
} fs_in;

uniform bool isBloom = true;

vec4 bloom();

void main()
{
//	fragColor = texture(main_texture, fs_in.texcoord);

	if (isBloom)
	{
		fragColor =	0.8f * texture(main_texture, fs_in.texcoord) + 0.25f * bloom();
//		fragColor =	texture(main_texture, fs_in.texcoord);
	}
	else
	{
		fragColor = texture(main_texture, fs_in.texcoord);
//		fragColor = ssao();
	}
}

vec4 bloom()
{
	int half_size = 6;
	vec4 color_sum = vec4(0.f);

	for (int i = -half_size; i <= half_size; i++)
	{
		for (int j = -half_size; j <= half_size; j++)
		{
			ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
			color_sum += texelFetch(hdr_texture, coord, 0);
		}
	}

	int sample_count = (half_size * 2 + 1) * (half_size * 2 + 1);
	
	return color_sum / sample_count;
}