#version 410

layout(location = 0) in vec2 iv2position;
layout(location = 1) in vec2 iv2tex_coord;

out VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
    vec2 texcoord;
} vertexData;

void main()
{
	gl_Position = vec4(iv2position, 1.0, 1.0);
    vertexData.texcoord = iv2tex_coord;
}