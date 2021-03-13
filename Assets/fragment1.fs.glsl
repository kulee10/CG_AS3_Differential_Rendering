#version 410

layout(location = 0) out vec4 fragColor;

in VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
    vec2 texcoord;
} vertexData;

uniform sampler2D tex_Sobj;
uniform sampler2D tex_Snoobj;
uniform sampler2D tex_Sb;
uniform int switch_fbo;

void main()
{	
	if (switch_fbo == 0)
		fragColor = texture(tex_Sb, vertexData.texcoord) + (texture(tex_Sobj, vertexData.texcoord) - texture(tex_Snoobj, vertexData.texcoord));
	else if (switch_fbo == 1)
		fragColor = texture(tex_Sobj, vertexData.texcoord);
	else if (switch_fbo == 2)
		fragColor = texture(tex_Snoobj, vertexData.texcoord);
	else if (switch_fbo == 3)
		fragColor = texture(tex_Sb, vertexData.texcoord);
}