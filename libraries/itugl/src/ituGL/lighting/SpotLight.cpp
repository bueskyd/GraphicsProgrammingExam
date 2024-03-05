#include <ituGL/lighting/SpotLight.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

SpotLight::SpotLight() : m_position(0.0f), m_direction(1.0f, 0.0f, 0.0f), m_attenuation(0.0f)
{
    InitTexture();
    InitFramebuffer();
    UpdateLightSpaceMatrix();
}

Light::Type SpotLight::GetType() const
{
    return Light::Type::Spot;
}

void SpotLight::InitTexture()
{
    m_lightRenderInfo.depthTextureObject.Bind();

    m_lightRenderInfo.depthTextureObject.SetImage(
        0, m_depthTextureResolution.x, m_depthTextureResolution.y,
        TextureObject::FormatDepth,
        TextureObject::InternalFormatDepth);
    m_lightRenderInfo.depthTextureObject.SetParameter(TextureObject::ParameterEnum::MinFilter, GL_NEAREST);
    m_lightRenderInfo.depthTextureObject.SetParameter(TextureObject::ParameterEnum::MagFilter, GL_NEAREST);

    m_lightRenderInfo.depthTextureObject.SetParameter(TextureObject::ParameterEnum::WrapS, GL_CLAMP_TO_BORDER);
    m_lightRenderInfo.depthTextureObject.SetParameter(TextureObject::ParameterEnum::WrapT, GL_CLAMP_TO_BORDER);
    float borderColor[]
    {
        1.0f, 1.0f, 1.0f, 1.0f
    };
    m_lightRenderInfo.depthTextureObject.SetParameter(TextureObject::ParameterColor::BorderColor, borderColor);
}

void SpotLight::InitFramebuffer()
{
    m_lightRenderInfo.framebufferObject.Bind();
    m_lightRenderInfo.framebufferObject.SetTexture(
        FramebufferObject::Target::Both,
        FramebufferObject::Attachment::Depth,
        m_lightRenderInfo.depthTextureObject);
    m_lightRenderInfo.framebufferObject.DisableColorRendering();
    FramebufferObject::Unbind();
}

void SpotLight::UpdateLightSpaceMatrix()
{
    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 lightView = glm::lookAt(
        m_position, m_position + m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
    m_lightRenderInfo.lightSpaceMatrix = lightProjection * lightView;
}

glm::vec3 SpotLight::GetPosition(const glm::vec3& fallback) const
{
    return m_position;
}

void SpotLight::SetPosition(const glm::vec3& position)
{
    m_position = position;
    UpdateLightSpaceMatrix();
}

glm::vec3 SpotLight::GetDirection(const glm::vec3& fallback) const
{
    return m_direction;
}

void SpotLight::SetDirection(const glm::vec3& direction)
{
    m_direction = direction;
    UpdateLightSpaceMatrix();
}

glm::vec4 SpotLight::GetAttenuation() const
{
    return m_attenuation;
}

float SpotLight::GetAngle() const
{
    return m_attenuation.w;
}

void SpotLight::SetAngle(float angle)
{
    m_attenuation.w = angle;
}

glm::vec2 SpotLight::GetDistanceAttenuation() const
{
    return glm::vec2(m_attenuation.x, m_attenuation.y);
}

void SpotLight::SetDistanceAttenuation(glm::vec2 attenuation)
{
    m_attenuation.x = attenuation.x;
    m_attenuation.y = attenuation.y;
}

glm::vec2 SpotLight::GetAngleAttenuation() const
{
    return glm::vec2(m_attenuation.z, m_attenuation.w);
}

void SpotLight::SetAngleAttenuation(glm::vec2 attenuation)
{
    m_attenuation.z = attenuation.x;
    m_attenuation.w = attenuation.y;
}

glm::ivec2 SpotLight::GetDepthTextureResolution() const
{
    return m_depthTextureResolution;
}

const std::span<const Light::LightRenderInfo> SpotLight::GetRenderInfo() const
{
    return std::span{&m_lightRenderInfo, 1};
}
