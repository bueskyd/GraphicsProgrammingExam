#include <ituGL/lighting/PointLight.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

PointLight::PointLight() : m_position(0.0f), m_attenuation(0.0f)
{
    InitTextures();
    InitFramebuffers();
    UpdateLightSpaceMatrices();
}

Light::Type PointLight::GetType() const
{
    return Light::Type::Point;
}

void PointLight::InitTextures()
{
    for (auto& lightRenderInfo : m_lightRenderInfo)
    {
        lightRenderInfo.depthTextureObject.Bind();

        lightRenderInfo.depthTextureObject.SetImage(
            0, m_depthTextureResolution.x, m_depthTextureResolution.y,
            TextureObject::FormatDepth,
            TextureObject::InternalFormatDepth);
        lightRenderInfo.depthTextureObject.SetParameter(TextureObject::ParameterEnum::MinFilter, GL_NEAREST);
        lightRenderInfo.depthTextureObject.SetParameter(TextureObject::ParameterEnum::MagFilter, GL_NEAREST);

        lightRenderInfo.depthTextureObject.SetParameter(TextureObject::ParameterEnum::WrapS, GL_CLAMP_TO_BORDER);
        lightRenderInfo.depthTextureObject.SetParameter(TextureObject::ParameterEnum::WrapT, GL_CLAMP_TO_BORDER);
        float borderColor[]
        {
            1.0f, 1.0f, 1.0f, 1.0f
        };
        lightRenderInfo.depthTextureObject.SetParameter(TextureObject::ParameterColor::BorderColor, borderColor);
    }
}

void PointLight::InitFramebuffers()
{
    for (auto& lightRenderInfo : m_lightRenderInfo)
    {
        lightRenderInfo.framebufferObject.Bind();
        lightRenderInfo.framebufferObject.SetTexture(
            FramebufferObject::Target::Both,
            FramebufferObject::Attachment::Depth,
            lightRenderInfo.depthTextureObject);
        lightRenderInfo.framebufferObject.DisableColorRendering();
    }
    FramebufferObject::Unbind();
}

void PointLight::UpdateLightSpaceMatrices()
{
    static std::array<glm::vec3, 6> directions
    {
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, -1.0f, 1.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f)
    };
    for (size_t i = 0; i < m_lightRenderInfo.size(); i++)
    {
        auto& lightRenderInfo = m_lightRenderInfo[i];
        const auto& direction = directions[i];
        glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 lightView = glm::lookAt(
            m_position, m_position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
        lightRenderInfo.lightSpaceMatrix = lightProjection * lightView;
    }
}

glm::vec3 PointLight::GetPosition(const glm::vec3& fallback) const
{
    return m_position;
}

void PointLight::SetPosition(const glm::vec3& position)
{
    m_position = position;
    UpdateLightSpaceMatrices();
}

glm::vec4 PointLight::GetAttenuation() const
{
    return glm::vec4(m_attenuation, 0.0f, 0.0f);
}

glm::vec2 PointLight::GetDistanceAttenuation() const
{
    return m_attenuation;
}

void PointLight::SetDistanceAttenuation(glm::vec2 attenuation)
{
    m_attenuation = attenuation;
}

glm::ivec2 PointLight::GetDepthTextureResolution() const
{
    return m_depthTextureResolution;
}

const std::span<const Light::LightRenderInfo> PointLight::GetRenderInfo() const
{
    return m_lightRenderInfo;
}
