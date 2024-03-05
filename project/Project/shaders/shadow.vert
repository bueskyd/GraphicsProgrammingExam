#version 330 core

layout (location = 0) in vec3 VertexPosition;

uniform mat4 LightSpaceMatrix;
uniform mat4 WorldMatrix;

void main()
{
	gl_Position = LightSpaceMatrix * WorldMatrix * vec4(VertexPosition, 1.0f);
}
