#pragma once

#include <ituGL/lighting/Light.h>
#include <glm/vec2.hpp>
#include <array>

class PointLight : public Light
{
public:
    PointLight();

    Type GetType() const override;

    using Light::GetPosition;
    glm::vec3 GetPosition(const glm::vec3& fallback) const override;
    void SetPosition(const glm::vec3& position) override;

    glm::vec4 GetAttenuation() const override;

    glm::vec2 GetDistanceAttenuation() const;
    void SetDistanceAttenuation(glm::vec2 attenuation);

    [[nodiscard]]
    glm::ivec2 GetDepthTextureResolution() const override;

    [[nodiscard]]
    const std::span<const LightRenderInfo> GetRenderInfo() const override;

private:
    void InitTextures();
    void InitFramebuffers();
    void UpdateLightSpaceMatrices();

private:
    glm::vec3 m_position;
    glm::vec2 m_attenuation;
    glm::ivec2 m_depthTextureResolution = glm::ivec2(10000);
    std::array<LightRenderInfo, 6> m_lightRenderInfo;
};
