#version 410 core

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec3 iv3normal;

uniform mat4 um4m;
uniform mat4 um4p;
uniform mat4 um4v;
uniform mat4 shadowM;
uniform vec3 eyePos;

out VS_OUT
{
    vec4 tc;
} vs_out;

void main(void)
{
	gl_Position = um4p * um4v * um4m * vec4(iv3vertex, 1.0);
	vs_out.tc = shadowM * vec4(iv3vertex, 1.0);
}
                                                                    