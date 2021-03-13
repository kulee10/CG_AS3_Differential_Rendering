#version 410 core

uniform sampler2DShadow shadow_tex;
uniform samplerCube tex_cubemap;

in VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 V; // eye space halfway vector
	vec3 world_normal;
	vec3 world_view;
	vec4 texcoord;
} vertexData;

out vec4 color;

vec3 diffuse_albedo = vec3(0.35, 0.35, 0.35);
vec3 specular_albedo = vec3(0.7, 0.7, 0.7);
float specular_power = 200.0;


void main(void)
{
	vec3 N = normalize(vertexData.N);
	vec3 L = normalize(vertexData.L);
	vec3 V = normalize(vertexData.V);
	vec3 H = normalize(L + V);

	vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo;
	vec3 specular = pow(max(dot(N, H), 0.0), specular_power) * specular_albedo;

	vec3 r = reflect(normalize(vertexData.world_view), normalize(vertexData.world_normal));

	float shadow_factor = textureProj(shadow_tex, vertexData.texcoord);
	if (shadow_factor < 0.2)
		shadow_factor = 0.2;
	else if (shadow_factor > 1.0)
		shadow_factor = 1.0;

	//color = (vec4(diffuse + specular, 1.0) * 0.65 + texture(tex_cubemap, r) * 0.35) * shadow_factor;
	color = vec4(((diffuse + specular) * 0.65 + texture(tex_cubemap, r).rgb * 0.35) * shadow_factor, 1.0);
}