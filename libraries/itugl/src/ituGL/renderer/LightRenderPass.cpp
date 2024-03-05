#include <ituGL/renderer/LightRenderPass.h>
#include <ituGL/renderer/Renderer.h>
#include <ituGL/shader/Material.h>
#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/geometry/VertexArrayObject.h>
#include <ituGL/lighting/Light.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <ituGL/application/Window.h>
#include <cassert>
#include <iostream>

LightRenderPass::LightRenderPass(int drawcallCollectionIndex)
	: m_drawcallCollectionIndex(drawcallCollectionIndex)
{
}

void LightRenderPass::Render()
{
	if (!m_shadowMapEnabled)
		return;
	Renderer& renderer = GetRenderer();
	const auto& drawcallCollection = renderer.GetDrawcalls(m_drawcallCollectionIndex);
	
	auto lights = renderer.GetLights();

	auto& device = renderer.GetDevice();

	auto& window = device.GetCurrentWindow();
	int windowWidth, windowHeight;
	window.GetDimensions(windowWidth, windowHeight);

	for (const auto& light : lights)
	{
		if (!light)
			continue;

		auto renderInfo = light->GetRenderInfo();

		for (const auto& ri : renderInfo)
		{
			ri.framebufferObject.Bind();

			glm::vec2 depthTextureResolution = light->GetDepthTextureResolution();
			device.SetViewport(
				0, 0, depthTextureResolution.x, depthTextureResolution.y);

			device.Clear(false, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

			for (const auto& drawcallInfo : drawcallCollection)
			{
				if (!drawcallInfo.material.CastsShadows())
					continue;

				assert(drawcallInfo.material.GetBlendEquationColor() == Material::BlendEquation::None);
				assert(drawcallInfo.material.GetBlendEquationAlpha() == Material::BlendEquation::None);
				assert(drawcallInfo.material.GetDepthWrite());

				const auto& worldMatrix = renderer.GetWorldMatrix(drawcallInfo.worldMatrixIndex);

				drawcallInfo.material.UseShadowShader(*light, ri.lightSpaceMatrix, worldMatrix);

				drawcallInfo.vao.Bind();

				drawcallInfo.drawcall.Draw();
			}
		}
	}

	FramebufferObject::Unbind();

	device.SetViewport(0, 0, windowWidth, windowHeight);
}

void LightRenderPass::SetShadowMapEnabled(bool enabled)
{
	m_shadowMapEnabled = enabled;
}
