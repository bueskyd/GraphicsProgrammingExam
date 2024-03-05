//Inputs
in vec2 TexCoord;

//Outputs
out vec4 FragColor;

//Uniforms
uniform sampler2D DepthTexture;
uniform sampler2D AlbedoTexture;
uniform sampler2D NormalTexture;
uniform sampler2D SpecularTexture;
uniform mat4 InvViewMatrix;
uniform mat4 InvProjMatrix;
uniform vec3 CameraPosition;
uniform sampler2D LightDepthTexture;
uniform mat4 LightSpaceMatrix;
uniform bool ShadowMapEnabled;

float CalculateShadow(vec3 fragPosition, vec3 normalVector, vec3 lightVector)
{
	vec4 fragLightSpacePosition = LightSpaceMatrix * vec4(fragPosition, 1.0f);
	vec3 projectionCoords = fragLightSpacePosition.xyz / fragLightSpacePosition.w;
	projectionCoords = projectionCoords * 0.5f + 0.5f;
	float currentDepth = projectionCoords.z;

	//Remove comment to fix over sampling outside the far plane
	if (currentDepth > 1.0f)
		return 0.0f;

	//Fix shadow acne
	float minBias = 0.001f;
	float maxBias = 0.002f;
	float shadowBias = max(maxBias * (1.0f - dot(normalVector, lightVector)), minBias);
	
	float shadow = 0.0f;
	vec2 texelSize = 1.0f / textureSize(LightDepthTexture, 0);
	for (int x = -1; x <= 1; x++)
		for (int y = -1; y <= 1; y++)
		{
			float depth = texture(LightDepthTexture, projectionCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - shadowBias > depth ? 1.0f : 0.0f;
		}
	shadow /= 9.0f;
	return shadow;
}

void main()
{
	vec3 fragPosition = (InvViewMatrix * vec4(ReconstructViewPosition(DepthTexture, TexCoord, InvProjMatrix), 1.0f)).xyz;
	vec3 viewVector = normalize(CameraPosition - fragPosition);

	vec4 albedoColorA = texture(AlbedoTexture, TexCoord);
	if (albedoColorA.a == 0.0f)
	{
		FragColor = vec4(SkyColor, 1.0f);
		return;
	}
	vec3 albedoColor = albedoColorA.rgb;

	vec4 normalTextureSample = texture(NormalTexture, TexCoord);
	bool ignoreSpecularIndirect = normalTextureSample.w > 0.0f;
	vec3 normal = normalize(normalTextureSample.xyz * 2.0f - 1.0f);
	
	vec3 lightVector = -LightDirection;

	vec3 arm = texture(SpecularTexture, TexCoord).xyz;

	SurfaceData data;
	data.normal = normal;
	data.albedo = albedoColor;
	data.ambientOcclusion = arm.x;
	data.roughness = arm.y;
	data.metalness = arm.z;

	float shadow = 0.0f;
	if (ShadowMapEnabled)
		shadow = CalculateShadow(fragPosition, normal, lightVector);
	vec3 fragColor = ComputeLighting(fragPosition, data, viewVector, shadow, ignoreSpecularIndirect);

	FragColor = vec4(fragColor, 1.0f);
}
