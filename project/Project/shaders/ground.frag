#version 330 core

in vec2 TexCoord;
in mat3 TBN;

out vec4 FragAlbedo;
out vec4 FragNormal;
out vec3 FragSpecular;

uniform sampler2D AlbedoTexture;
uniform sampler2D NormalsTexture;
uniform sampler2D SpecularTexture;

uniform float AmbientOcclusion;

void main()
{
	FragAlbedo = texture(AlbedoTexture, TexCoord);


	vec3 normal = texture(NormalsTexture, TexCoord).xyz;
	normal = normal * 2.0f - 1.0f;
	normal = normalize(TBN * normal);
	FragNormal = vec4(normalize(normal) * 0.5f + 0.5f, 0.0f);

	vec3 specular = texture(SpecularTexture, TexCoord).xyz;
	specular.x *= AmbientOcclusion;
	FragSpecular = specular;
}
