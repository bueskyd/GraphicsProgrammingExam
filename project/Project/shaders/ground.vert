#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;
layout (location = 3) in vec3 VertexTangent;
layout (location = 4) in vec3 VertexBitangent;

out vec2 TexCoord;
out mat3 TBN;

uniform mat4 WorldMatrix;
uniform mat4 WorldViewMatrix;
uniform mat4 WorldViewProjMatrix;

void main()
{
	vec3 t = normalize(vec3(WorldMatrix * vec4(VertexTangent, 0.0f)));
	vec3 b = normalize(vec3(WorldMatrix * vec4(VertexBitangent, 0.0f)));
	vec3 n = normalize(vec3(WorldMatrix * vec4(VertexNormal, 0.0f)));
	TBN = mat3(t, b, n);

	TexCoord = VertexTexCoord;
	gl_Position = WorldViewProjMatrix * vec4(VertexPosition, 1.0f);
}
