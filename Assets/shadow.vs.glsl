#version 410 core

uniform mat4 um4mvp;

layout (location = 0) in vec3 position;

void main(void)
{
	gl_Position = um4mvp * vec4(position, 1.0);
}