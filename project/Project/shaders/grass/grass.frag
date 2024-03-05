#version 330 core

in vec3 Normal;
in vec2 TexCoord;

out vec4 FragAlbedo;
out vec4 FragNormal;
out vec3 FragSpecular;

uniform sampler2D AlbedoTexture;
uniform sampler2D AmbientOcclusionTexture;
uniform sampler2D RoughnessTexture;

uniform float AmbientOcclusion;

void main()
{
	FragAlbedo = texture(AlbedoTexture, TexCoord);
	FragNormal = vec4(normalize(Normal) * 0.5f + 0.5f, 1.0f);
	FragSpecular = vec3(
		AmbientOcclusion * texture(AmbientOcclusionTexture, TexCoord).x,
		texture(RoughnessTexture, TexCoord).x,
		0.0f);
}
