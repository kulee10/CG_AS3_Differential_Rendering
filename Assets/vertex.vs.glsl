#version 410 core

out VS_OUT
{
    vec3 tc;
} vs_out;

uniform mat4 um4p;
uniform mat4 um4v;
uniform vec3 eyePos;

void main(void)
{
    vec3[4] vertices = vec3[4](vec3(-1.0, -1.0, 1.0),
                               vec3( 1.0, -1.0, 1.0),
                               vec3(-1.0,  1.0, 1.0),
                               vec3( 1.0,  1.0, 1.0));
    vec4 tc = inverse(um4p*um4v) * vec4(vertices[gl_VertexID], 1.0);
	tc /= tc.w;
    vs_out.tc = normalize(tc.xyz - eyePos);
    gl_Position = vec4(vertices[gl_VertexID], 1.0);
}
                                                                    