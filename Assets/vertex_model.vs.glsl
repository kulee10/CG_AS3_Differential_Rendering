#version 410 core

uniform mat4 um4m;
uniform mat4 um4p;
uniform mat4 um4v;
uniform mat4 shadowM;
uniform vec3 eyePos;

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec2 iv2tex_coord;
layout(location = 2) in vec3 iv3normal;

//vec3 light_pos = vec3(-31.75, 26.05, -97.72);
vec3 light_pos = vec3(0.0, 0.0, -100.0);


out VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 V; // eye space halfway vector
	vec3 world_normal;
	vec3 world_view;
    vec4 texcoord;
} vertexData;

void main(void)
{
	vec4 P = um4v * um4m * vec4(iv3vertex, 1.0);
	vec4 world_P = um4m * vec4(iv3vertex, 1.0);

	vertexData.N = mat3(um4v * um4m) * iv3normal;
	vertexData.L = light_pos;
	vertexData.V = -P.xyz;

	vertexData.world_normal =  mat3(transpose(inverse(um4m))) * iv3normal;
	vertexData.world_view = world_P.xyz;

	vertexData.texcoord =  shadowM * vec4(iv3vertex, 1.0);
	gl_Position = um4p * P;
}