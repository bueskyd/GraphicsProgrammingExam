#include <ituGL/lighting/DirectionalLight.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

DirectionalLight::DirectionalLight()
    :
    m_position(0.0f, 0.0f, 0.0f),
    m_direction(1.0f, 0.0f, 0.0f)
{
    int maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    maxTextureSize = std::min(maxTextureSize, 15000);
    m_depthTextureResolution = glm::vec2(maxTextureSize);
    InitTexture();
    InitFramebuffer();
    UpdateLightSpaceMatrix();
}

Light::Type DirectionalLight::GetType() const
{
    return Light::Type::Directional;
}

void DirectionalLight::InitTexture()
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

void DirectionalLight::InitFramebuffer()
{
    m_lightRenderInfo.framebufferObject.Bind();
    m_lightRenderInfo.framebufferObject.SetTexture(
        FramebufferObject::Target::Both,
        FramebufferObject::Attachment::Depth,
        m_lightRenderInfo.depthTextureObject);
    m_lightRenderInfo.framebufferObject.DisableColorRendering();
    FramebufferObject::Unbind();
}

void DirectionalLight::UpdateLightSpaceMatrix()
{
    float size = 15.0f;
    glm::mat4 lightProjection = glm::ortho(-size, size, -size, size, 0.1f, 100.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(m_direction, up));
    glm::vec3 lightUp = glm::normalize(glm::cross(right, m_direction));
    glm::mat4 lightView = glm::lookAt(
        m_position, m_position + m_direction, lightUp);
    m_lightRenderInfo.lightSpaceMatrix = lightProjection * lightView;
}

glm::vec3 DirectionalLight::GetPosition(const glm::vec3& fallback) const
{
    return m_position;
}

void DirectionalLight::SetPosition(const glm::vec3& position)
{
    m_position = position;
}

glm::vec3 DirectionalLight::GetDirection(const glm::vec3& fallback) const
{
    return m_direction;
}

void DirectionalLight::SetDirection(const glm::vec3& direction)
{
    m_direction = direction;
    UpdateLightSpaceMatrix();
}

glm::ivec2 DirectionalLight::GetDepthTextureResolution() const
{
    return m_depthTextureResolution;
}

const std::span<const Light::LightRenderInfo> DirectionalLight::GetRenderInfo() const
{
    return std::span{ &m_lightRenderInfo, 1 };
}
