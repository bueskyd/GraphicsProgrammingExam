uniform float CurrentTime;
uniform vec2 WindDirection;
uniform float WindSpeed;

struct InstanceData
{
	vec3 vertexPosition;
	vec3 instanceOffset;
	float instanceRotation;
	float instanceHeightMultiplier;
};

vec3 calculateVertexPosition(InstanceData instanceData)
{
	instanceData.vertexPosition.y *= instanceData.instanceHeightMultiplier;

	//Rotate around y-axis:
	float c = cos(instanceData.instanceRotation);
	float s = sin(instanceData.instanceRotation);
	float rotatedX = instanceData.vertexPosition.x * c - instanceData.vertexPosition.z * s;
	float rotatedZ = instanceData.vertexPosition.x * s + instanceData.vertexPosition.z * c;

	float instanceDot = dot(vec3(WindDirection, 0.0f), instanceData.instanceOffset);

	//Rotate around z-axis depending on wind:
	float a = sin(instanceDot + CurrentTime) * WindDirection.x;
	c = cos(a * WindSpeed);
	s = sin(a * WindSpeed);
	float rotatedY = instanceData.vertexPosition.y;
	rotatedX = rotatedX * c - rotatedY * s;
	rotatedY = rotatedX * s + rotatedY * c;
	
	//Rotate around x-axis depending on wind:
	a = sin(instanceDot + CurrentTime) * WindDirection.y;
	c = cos(a * WindSpeed);
	s = sin(a * WindSpeed);
	rotatedZ = rotatedZ * c - rotatedY * s;
	rotatedY = rotatedZ * s + rotatedY * c;

	//Determine world position:
	float offsetRotatedX = rotatedX + instanceData.instanceOffset.x;
	float offsetRotatedZ = rotatedZ + instanceData.instanceOffset.z;
	float offsetY = rotatedY + instanceData.instanceOffset.y;
	vec3 position = vec3(offsetRotatedX, offsetY, offsetRotatedZ);
	return position;
}
