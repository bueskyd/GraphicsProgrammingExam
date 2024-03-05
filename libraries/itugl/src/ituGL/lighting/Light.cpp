#include <ituGL/lighting/Light.h>

Light::Light() : m_color(1.0f), m_intensity(1.0f)
{
}

glm::vec3 Light::GetPosition(const glm::vec3& fallback) const
{
    return fallback;
}

void Light::SetPosition(const glm::vec3& position)
{
}

glm::vec3 Light::GetDirection(const glm::vec3& fallback) const
{
    return fallback;
}

void Light::SetDirection(const glm::vec3& direction)
{
}

glm::vec4 Light::GetAttenuation() const
{
    return glm::vec4(-1);
}

glm::vec3 Light::GetColor() const
{
    return m_color;
}

void Light::SetColor(const glm::vec3& color)
{
    m_color = color;
}

float Light::GetIntensity() const
{
    return m_intensity;
}

void Light::SetIntensity(float intensity)
{
    m_intensity = intensity;
}

const std::span<const Light::LightRenderInfo> Light::GetRenderInfo() const
{
    return std::span<LightRenderInfo>();
}
