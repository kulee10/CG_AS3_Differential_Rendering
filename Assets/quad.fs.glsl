#version 410 core

uniform sampler2DShadow shadow_tex;
uniform int shadowUse;

in VS_OUT
{
    vec4 tc;
} fs_in;

layout (location = 0) out vec4 color;

void main(void)
{
	float shadow_factor = textureProj(shadow_tex, fs_in.tc);
	if (shadowUse == 1){
		color = vec4(0.64, 0.57, 0.49, 1.0);
	}
	else {
		if (shadow_factor > 0.5)
			color = vec4(0.64, 0.57, 0.49, 1.0);
		else
			color = vec4(0.41, 0.36, 0.37, 1.0);
	}
}