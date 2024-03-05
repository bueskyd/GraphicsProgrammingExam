#pragma once

#include <ituGL/lighting/Light.h>

class DirectionalLight : public Light
{
public:
    DirectionalLight();

    Type GetType() const override;

    glm::vec3 GetPosition(const glm::vec3& fallback) const override;
    void SetPosition(const glm::vec3& position) override;

    glm::vec3 GetDirection(const glm::vec3& fallback) const override;
    void SetDirection(const glm::vec3& direction) override;

    [[nodiscard]]
    glm::ivec2 GetDepthTextureResolution() const override;

    [[nodiscard]]
    virtual const std::span<const LightRenderInfo> GetRenderInfo() const;

private:
    void InitTexture();
    void InitFramebuffer();
    void UpdateLightSpaceMatrix();

private:
    glm::vec3 m_position;
    glm::vec3 m_direction;
    glm::ivec2 m_depthTextureResolution;
    LightRenderInfo m_lightRenderInfo;
};
