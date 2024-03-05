layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 VertexTexCoord;
layout (location = 2) in vec3 InstanceOffset;
layout (location = 3) in float InstanceRotation;
layout (location = 4) in float InstanceHeightMultiplier;

uniform mat4 LightSpaceMatrix;
uniform mat4 WorldMatrix;

void main()
{
	InstanceData instanceData;
	instanceData.vertexPosition = VertexPosition;
	instanceData.instanceOffset = InstanceOffset;
	instanceData.instanceRotation = InstanceRotation;
	instanceData.instanceHeightMultiplier = InstanceHeightMultiplier;

	vec3 position = calculateVertexPosition(instanceData);
	
	gl_Position = LightSpaceMatrix * WorldMatrix * vec4(position, 1.0f);
}
