#pragma once

#include <ituGL/texture/FramebufferObject.h>
#include <ituGL/texture/Texture2DObject.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <memory>
#include <span>

class FramebufferObject;
class Texture2DObject;

class Light
{
public:
    enum class Type
    {
        Directional,
        Point,
        Spot,
    };

    struct LightRenderInfo
    {
        FramebufferObject framebufferObject;
        Texture2DObject depthTextureObject;
        glm::mat4 lightSpaceMatrix;
    };

public:
    Light();

    virtual Type GetType() const = 0;

    virtual glm::vec3 GetPosition(const glm::vec3& fallback) const;
    virtual void SetPosition(const glm::vec3& position);

    virtual glm::vec3 GetDirection(const glm::vec3& fallback) const;
    virtual void SetDirection(const glm::vec3& direction);

    virtual glm::vec4 GetAttenuation() const;

    glm::vec3 GetColor() const;
    void SetColor(const glm::vec3& color);

    float GetIntensity() const;
    void SetIntensity(float intensity);

    [[nodiscard]]
    virtual glm::ivec2 GetDepthTextureResolution() const = 0;

    [[nodiscard]]
    virtual const std::span<const LightRenderInfo> GetRenderInfo() const;

private:
    glm::vec3 m_color;
    float m_intensity;
};
