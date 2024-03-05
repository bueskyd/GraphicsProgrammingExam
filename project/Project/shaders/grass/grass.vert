layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 VertexTexCoord;
layout (location = 2) in vec3 InstanceOffset;
layout (location = 3) in float InstanceRotation;
layout (location = 4) in float InstanceHeightMultiplier;

uniform mat4 WorldMatrix;
uniform mat4 WorldViewMatrix;
uniform mat4 WorldViewProjMatrix;

out vec3 Normal;
out vec2 TexCoord;

void main()
{
	Normal = (WorldMatrix * vec4(0.0f, 1.0f, 0.0f, 0.0f)).xyz;

	TexCoord = VertexTexCoord;

	InstanceData instanceData;
	instanceData.vertexPosition = VertexPosition;
	instanceData.instanceOffset = InstanceOffset;
	instanceData.instanceRotation = InstanceRotation;
	instanceData.instanceHeightMultiplier = InstanceHeightMultiplier;

	vec3 position = calculateVertexPosition(instanceData);
	
	gl_Position = WorldViewProjMatrix * vec4(position, 1.0f);
}
