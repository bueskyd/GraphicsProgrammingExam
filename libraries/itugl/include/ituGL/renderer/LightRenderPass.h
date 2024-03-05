#pragma once

#include <ituGL/renderer/RenderPass.h>
#include <ituGL/texture/Texture2DObject.h>
#include <ituGL/texture/FramebufferObject.h>
#include <ituGL/shader/ShaderProgram.h>
#include <vector>

class LightRenderPass : public RenderPass
{
public:
    LightRenderPass(int drawcallCollectionIndex = 0);

    void Render() override;

    void SetShadowMapEnabled(bool enabled);

private:
    int m_drawcallCollectionIndex;
    bool m_shadowMapEnabled = true;
};
