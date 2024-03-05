#include "GrassApplication.h"
#include <ituGL/asset/Texture2DLoader.h>
#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/geometry/VertexFormat.h>
#include <glm/gtx/transform.hpp>
#include <ituGL/utils/RandomReal.h>
#include <ituGL/renderer/GBufferRenderPass.h>
#include <ituGL/renderer/DeferredRenderPass.h>
#include <ituGL/lighting/Light.h>
#include <imgui.h>
#include <ituGL/asset/ModelLoader.h>
#include <ituGL/renderer/LightRenderPass.h>
#include <iostream>

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

namespace proj
{
    GrassApplication::GrassApplication()
        : Application(1024, 1024, "Grass"),
        m_gridPoints(1000),
        m_planeSize(10),
        m_planeGridConversion(
            m_gridPoints.x / m_planeSize.x, m_gridPoints.y / m_planeSize.z),
        m_generatedGrassStraws(1'000'000),
        m_renderer(GetDevice())
    {
    }

    void GrassApplication::Initialize()
    {
        Application::Initialize();

        GetMainWindow().SetCursorMode(Window::CursorMode::Disabled);

        m_imGui.Initialize(GetMainWindow());

        m_heights = CreateHeights(m_gridPoints, glm::ivec2(0));

        InitializeCamera();

        InitializeDeferredMaterials();

        InitializeGround();

        InitializeGrass();

        m_light.SetColor(m_settings.lightColor);
        m_light.SetPosition(glm::vec3(-10.0f));
        m_light.SetIntensity(m_settings.lightIntensity);

        InitializeRenderer();

        auto& device = GetDevice();
        device.EnableFeature(GL_DEPTH_TEST);
        device.SetVSyncEnabled(false);
    }

    void GrassApplication::Update()
    {
        Application::Update();

        UpdateInput();

        m_renderer.AddModel(m_groundModel, glm::scale(static_cast<glm::vec3>(m_planeSize)));
        m_renderer.AddModel(m_grassModel, glm::mat4(1.0f));

        glm::vec3 lightDirection = m_light.GetDirection(glm::vec3(1.0f, 0.0f, 0.0f));
        float offset = 10.0f;
        glm::vec3 positionOffset = lightDirection * offset;
        glm::vec3 lightPosition = m_cameraPosition - positionOffset;
        m_light.SetPosition(lightPosition);

        m_renderer.AddLight(&m_light);

        m_renderer.SetCurrentCamera(m_camera);
    }

    void GrassApplication::Render()
    {
        Application::Render();

        GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

        m_renderer.Render();

        RenderGUI();
    }

    void GrassApplication::Cleanup()
    {
        Application::Cleanup();
    }

    void GrassApplication::InitializeGround()
    {
        auto albedoTexture = Texture2DLoader::LoadTextureShared("textures/mud_forest_diff_4k.jpg", TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA, true);
        albedoTexture->Bind();
        albedoTexture->SetParameter(TextureObject::ParameterEnum::MinFilter, GL_LINEAR_MIPMAP_LINEAR);
        albedoTexture->SetParameter(TextureObject::ParameterEnum::MagFilter, GL_LINEAR);
        albedoTexture->GenerateMipmap();
        albedoTexture->Unbind();

        auto normalTexture = Texture2DLoader::LoadTextureShared("textures/mud_forest_nor_gl_4k.jpg", TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA, false);

        auto specularTexture = Texture2DLoader::LoadTextureShared("textures/mud_forest_arm_4k.jpg", TextureObject::FormatRGB, TextureObject::InternalFormatRGB, false);

        auto vertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/ground.vert");
        auto fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/ground.frag");
        auto shaderProgram = std::make_shared<ShaderProgram>();
        shaderProgram->Build(vertexShader, fragmentShader);

        auto shadowVertexShader = ShaderLoader::Load(Shader::VertexShader, "shaders/shadow.vert");
        auto shadowFragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/shadow.frag");
        auto shadowShaderProgram = std::make_shared<ShaderProgram>();
        shadowShaderProgram->Build(shadowVertexShader, shadowFragmentShader);

        ShaderUniformCollection::NameSet filteredUniforms;
        filteredUniforms.insert("WorldMatrix");
        filteredUniforms.insert("WorldViewMatrix");
        filteredUniforms.insert("WorldViewProjMatrix");
        filteredUniforms.insert("LightSpaceMatrix");
        filteredUniforms.insert("WorldMatrix");
        auto material = std::make_shared<Material>(
            shaderProgram, filteredUniforms);
        material->SetUniformValue("AlbedoTexture", albedoTexture);
        material->SetUniformValue("NormalsTexture", normalTexture);
        material->SetUniformValue("SpecularTexture", specularTexture);

        auto lightSpaceMatrixShadowLocation = shadowShaderProgram->GetUniformLocation("LightSpaceMatrix");
        auto worldMatrixShadowLocation = shadowShaderProgram->GetUniformLocation("WorldMatrix");
        
        material->SetShadowShader(shadowShaderProgram,
            [=](const ShaderProgram& shaderProgram, const Light& light, const glm::mat4& lightSpaceMatrix, const glm::mat4& worldMatrix)
            {
                shaderProgram.SetUniform(lightSpaceMatrixShadowLocation, lightSpaceMatrix);
                
                shaderProgram.SetUniform(worldMatrixShadowLocation, worldMatrix);
            });

        auto worldMatrixLocation = shaderProgram->GetUniformLocation("WorldMatrix");
        auto worldViewMatrixLocation = shaderProgram->GetUniformLocation("WorldViewMatrix");
        auto worldViewProjMatrixLocation = shaderProgram->GetUniformLocation("WorldViewProjMatrix");
        auto ambientOcclusionLocation = shaderProgram->GetUniformLocation("AmbientOcclusion");

        m_renderer.RegisterShaderProgram(shaderProgram,
            [=](const ShaderProgram& shaderProgram, const glm::mat4& worldMatrix, const Camera& camera, bool cameraChanged)
            {
                shaderProgram.SetUniform(worldMatrixLocation, worldMatrix);
                shaderProgram.SetUniform(worldViewMatrixLocation, camera.GetViewMatrix() * worldMatrix);
                shaderProgram.SetUniform(worldViewProjMatrixLocation, camera.GetViewProjectionMatrix() * worldMatrix);
                shaderProgram.SetUniform(ambientOcclusionLocation, m_settings.ambientOcclusion);
            },
            nullptr);

        auto groundMesh = std::make_shared<Mesh>();
        CreateTerrainMesh(*groundMesh, m_heights);
        m_groundModel = Model(groundMesh);
        m_groundModel.AddMaterial(material);
    }

    void GrassApplication::InitializeGrass()
    {
        Texture2DLoader textureLoader(TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA);
        textureLoader.SetFlipVertical(true);
        auto albedoTexture = textureLoader.LoadShared("textures/Grass16.jpg");
        albedoTexture->Bind();
        albedoTexture->SetParameter(TextureObject::ParameterEnum::MinFilter, GL_LINEAR_MIPMAP_LINEAR);
        albedoTexture->SetParameter(TextureObject::ParameterEnum::MagFilter, GL_LINEAR);
        albedoTexture->GenerateMipmap();
        albedoTexture->Unbind();

        auto ambientOcclusionTexture = Texture2DLoader::LoadTextureShared("textures/GrassAmbientOcclusion16.jpg", TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA, false);
        auto roughnessTexture = Texture2DLoader::LoadTextureShared("textures/GrassRoughness16.jpg", TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA, false);

        std::vector<const char*> vertexShaderPaths
        {
            "shaders/version330.glsl",
            "shaders/grass/grassVertices.glsl",
            "shaders/grass/grass.vert"
        };
        auto vertexShader = ShaderLoader(Shader::VertexShader).Load(vertexShaderPaths);
        auto fragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/grass/grass.frag");
        auto shaderProgram = std::make_shared<ShaderProgram>();
        shaderProgram->Build(vertexShader, fragmentShader);

        std::vector<const char*> shadowVertexShaderPaths
        {
            "shaders/version330.glsl",
            "shaders/grass/grassVertices.glsl",
            "shaders/grass/grassShadow.vert"
        };
        auto shadowVertexShader = ShaderLoader(Shader::VertexShader).Load(shadowVertexShaderPaths);
        auto shadowFragmentShader = ShaderLoader::Load(Shader::FragmentShader, "shaders/shadow.frag");
        auto shadowShaderProgram = std::make_shared<ShaderProgram>();
        shadowShaderProgram->Build(shadowVertexShader, shadowFragmentShader);

        ShaderUniformCollection::NameSet filteredUniforms;
        filteredUniforms.insert("WorldMatrix");
        filteredUniforms.insert("WorldViewMatrix");
        filteredUniforms.insert("WorldViewProjMatrix");
        filteredUniforms.insert("CurrentTime");
        filteredUniforms.insert("WindDirection");
        filteredUniforms.insert("WindSpeed");

        auto material = std::make_shared<Material>(
            shaderProgram, filteredUniforms);
        material->SetUniformValue("AlbedoTexture", albedoTexture);
        material->SetUniformValue("AmbientOcclusionTexture", ambientOcclusionTexture);
        material->SetUniformValue("RoughnessTexture", roughnessTexture);

        auto lightSpaceMatrixShadowLocation = shadowShaderProgram->GetUniformLocation("LightSpaceMatrix");
        auto worldMatrixShadowLocation = shadowShaderProgram->GetUniformLocation("WorldMatrix");
        auto currentTimeShadowLocation = shadowShaderProgram->GetUniformLocation("CurrentTime");
        auto windDirectionShadowLocation = shadowShaderProgram->GetUniformLocation("WindDirection");
        auto windSpeedShadowLocation = shadowShaderProgram->GetUniformLocation("WindSpeed");

        material->SetShadowShader(shadowShaderProgram,
            [=](const ShaderProgram& shaderProgram, const Light& light, const glm::mat4& lightSpaceMatrix, const glm::mat4& worldMatrix)
            {
                shaderProgram.SetUniform(lightSpaceMatrixShadowLocation, lightSpaceMatrix);
                shaderProgram.SetUniform(worldMatrixShadowLocation, worldMatrix);
                shaderProgram.SetUniform(currentTimeShadowLocation, GetCurrentTime());
                shaderProgram.SetUniform(windDirectionShadowLocation, m_settings.windDirection);
                shaderProgram.SetUniform(windSpeedShadowLocation, m_settings.windSpeed);
            });

        auto worldMatrixLocation = shaderProgram->GetUniformLocation("WorldMatrix");
        auto worldViewMatrixLocation = shaderProgram->GetUniformLocation("WorldViewMatrix");
        auto worldViewProjMatrixLocation = shaderProgram->GetUniformLocation("WorldViewProjMatrix");
        auto currentTimeLocation = shaderProgram->GetUniformLocation("CurrentTime");
        auto windDirectionLocation = shaderProgram->GetUniformLocation("WindDirection");
        auto windSpeedLocation = shaderProgram->GetUniformLocation("WindSpeed");
        auto ambientOcclusionLocation = shaderProgram->GetUniformLocation("AmbientOcclusion");

        m_renderer.RegisterShaderProgram(shaderProgram,
            [=](const ShaderProgram& shaderProgram, const glm::mat4& worldMatrix, const Camera& camera, bool cameraChanged)
            {
                shaderProgram.SetUniform(worldMatrixLocation, worldMatrix);
                shaderProgram.SetUniform(worldViewMatrixLocation, camera.GetViewMatrix() * worldMatrix);
                shaderProgram.SetUniform(worldViewProjMatrixLocation, camera.GetViewProjectionMatrix() * worldMatrix);
                shaderProgram.SetUniform(currentTimeLocation, GetCurrentTime());
                shaderProgram.SetUniform(windDirectionLocation, m_settings.windDirection);
                shaderProgram.SetUniform(windSpeedLocation, m_settings.windSpeed);
                shaderProgram.SetUniform(ambientOcclusionLocation, m_settings.ambientOcclusion);
            },
            nullptr);

        auto grassMesh = std::make_shared<Mesh>();
        CreateGrassMesh(*grassMesh, m_heights, m_grassSubmeshIndex);
        m_grassModel = Model(grassMesh);
        m_grassModel.AddMaterial(material);
    }

    void GrassApplication::InitializeCamera()
    {
        m_camera.SetViewMatrix(m_cameraPosition, glm::vec3(0.0f));

        float aspectRatio = GetMainWindow().GetAspectRatio();
        m_camera.SetPerspectiveProjectionMatrix(
            glm::radians(90.0f), aspectRatio, 0.1f, 1000.0f);
    }

    void GrassApplication::InitializeDeferredMaterials()
    {
        {
            std::vector<const char*> vertexShaderPaths;
            vertexShaderPaths.push_back("shaders/version330.glsl");
            vertexShaderPaths.push_back("shaders/deferred.vert");
            Shader vertexShader = ShaderLoader(Shader::VertexShader).Load(vertexShaderPaths);

            std::vector<const char*> fragmentShaderPaths;
            fragmentShaderPaths.push_back("shaders/version330.glsl");
            fragmentShaderPaths.push_back("shaders/utils.glsl");
            fragmentShaderPaths.push_back("shaders/lambert-ggx.glsl");
            fragmentShaderPaths.push_back("shaders/lighting.glsl");
            fragmentShaderPaths.push_back("shaders/deferred.frag");
            Shader fragmentShader = ShaderLoader(Shader::FragmentShader).Load(fragmentShaderPaths);

            auto shaderProgramPtr = std::make_shared<ShaderProgram>();
            shaderProgramPtr->Build(vertexShader, fragmentShader);

            ShaderUniformCollection::NameSet filteredUniforms;
            filteredUniforms.insert("InvProjMatrix");
            filteredUniforms.insert("WorldViewProjMatrix");
            filteredUniforms.insert("CameraPosition");
            filteredUniforms.insert("WorldViewProjMatrix");
            filteredUniforms.insert("ShadowMapEnabled");
            filteredUniforms.insert("SkyColor");

            auto invViewMatrixLocation = shaderProgramPtr->GetUniformLocation("InvViewMatrix");
            auto invProjMatrixLocation = shaderProgramPtr->GetUniformLocation("InvProjMatrix");
            auto cameraPositionLocation = shaderProgramPtr->GetUniformLocation("CameraPosition");
            auto worldViewProjMatrixLocation = shaderProgramPtr->GetUniformLocation("WorldViewProjMatrix");
            auto shadowMapEnabledLocation = shaderProgramPtr->GetUniformLocation("ShadowMapEnabled");
            auto skyColorLocation = shaderProgramPtr->GetUniformLocation("SkyColor");

            m_renderer.RegisterShaderProgram(shaderProgramPtr,
                [=](const ShaderProgram& shaderProgram, const glm::mat4& worldMatrix, const Camera& camera, bool cameraChanged)
                {
                    if (cameraChanged)
                    {
                        shaderProgram.SetUniform(invViewMatrixLocation, glm::inverse(camera.GetViewMatrix()));
                        shaderProgram.SetUniform(invProjMatrixLocation, glm::inverse(camera.GetProjectionMatrix()));
                        shaderProgram.SetUniform(cameraPositionLocation, m_cameraPosition);
                    }
                    shaderProgram.SetUniform(worldViewProjMatrixLocation, camera.GetViewProjectionMatrix() * worldMatrix);
                    shaderProgram.SetUniform(shadowMapEnabledLocation, static_cast<int>(m_settings.shadowMapEnabled));
                    shaderProgram.SetUniform(skyColorLocation, m_settings.skyColor);
                },
                GetUpdateLightsFunction(shaderProgramPtr));
            m_deferredMaterial = std::make_shared<Material>(shaderProgramPtr, filteredUniforms);
        }
    }

    [[nodiscard]] static glm::vec3 anglesToDirection(float pitch, float yaw)
    {
        glm::vec3 direction;
        direction.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
        direction.y = std::sin(glm::radians(pitch));
        direction.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
        return glm::normalize(direction);
    }

    void GrassApplication::RenderGUI()
    {
        m_imGui.BeginFrame();

        ImGui::SliderFloat("Light pitch", &m_settings.lightPitch, -89.9f, -52.0f);
        ImGui::SliderFloat("Light yaw", &m_settings.lightYaw, -360.0f, 360.0f);
        m_light.SetDirection(anglesToDirection(m_settings.lightPitch, m_settings.lightYaw));

        float lightIntensity = m_light.GetIntensity();
        float maxLightIntensity = 10.0f;
        ImGui::SliderFloat("Light intensity", &lightIntensity, 0.0f, maxLightIntensity);
        m_light.SetIntensity(lightIntensity);

        glm::vec3 lightColor = m_light.GetColor();

        ImGui::ColorEdit3("Light color", &lightColor[0]);
        m_light.SetColor(lightColor);

        ImGui::SliderFloat("Ambient occlusion", &m_settings.ambientOcclusion, 0.0f, 1.0f);

        int grassStraws = static_cast<int>(m_settings.grassStraws);
        ImGui::SliderInt("Grass straws", &grassStraws, 0, m_generatedGrassStraws);
        m_settings.grassStraws = static_cast<uint32_t>(grassStraws);
        m_grassModel.GetMesh().SetSubmeshInstanceCount(m_grassSubmeshIndex, m_settings.grassStraws);

        ImGui::ColorEdit3("SkyColor", &m_settings.skyColor[0]);

        ImGui::SliderFloat("Camera sensitivity", &m_settings.cameraSensitivity, 0.01f, 1.0f);

        ImGui::Checkbox("Shadowmap enabled", &m_settings.shadowMapEnabled);

        if (ImGui::Button("Reset settings"))
            m_settings = m_defaultSettings;

        m_lightRenderPass->SetShadowMapEnabled(m_settings.shadowMapEnabled);

        m_imGui.EndFrame();
    }

    Renderer::UpdateLightsFunction GrassApplication::GetUpdateLightsFunction(std::shared_ptr<ShaderProgram> shaderProgram)
    {
        ShaderProgram::Location lightIndirectLocation = shaderProgram->GetUniformLocation("LightIndirect");
        ShaderProgram::Location lightColorLocation = shaderProgram->GetUniformLocation("LightColor");
        ShaderProgram::Location lightPositionLocation = shaderProgram->GetUniformLocation("LightPosition");
        ShaderProgram::Location lightDirectionLocation = shaderProgram->GetUniformLocation("LightDirection");
        ShaderProgram::Location lightAttenuationLocation = shaderProgram->GetUniformLocation("LightAttenuation");

        return [=](const ShaderProgram& shaderProgram, std::span<const Light* const> lights, unsigned int& lightIndex) -> bool
        {
            bool needsRender = lightIndex == 0;

            shaderProgram.SetUniform(lightIndirectLocation, lightIndex == 0 ? 1 : 0);

            if (lightIndex < lights.size())
            {
                const Light& light = *lights[lightIndex];
                shaderProgram.SetUniform(lightColorLocation, light.GetColor() * light.GetIntensity());
                shaderProgram.SetUniform(lightPositionLocation, light.GetPosition(glm::vec3(0.0f)));
                shaderProgram.SetUniform(lightDirectionLocation, light.GetDirection(glm::vec3(0.0f, 0.0f, 0.0f)));
                shaderProgram.SetUniform(lightAttenuationLocation, light.GetAttenuation());
                needsRender = true;
            }
            else
            {
                // Disable light
                shaderProgram.SetUniform(lightColorLocation, glm::vec3(0.0f));
            }

            lightIndex++;

            return needsRender;
        };
    }

    void GrassApplication::InitializeRenderer()
    {
        int width, height;
        GetMainWindow().GetDimensions(width, height);
        auto gbufferRenderpass = std::make_unique<GBufferRenderPass>(width, height);

        m_deferredMaterial->SetUniformValue("DepthTexture", gbufferRenderpass->GetDepthTexture());
        m_deferredMaterial->SetUniformValue("AlbedoTexture", gbufferRenderpass->GetAlbedoTexture());
        m_deferredMaterial->SetUniformValue("NormalTexture", gbufferRenderpass->GetNormalTexture());
        m_deferredMaterial->SetUniformValue("SpecularTexture", gbufferRenderpass->GetSpecularTexture());
        auto lightRenderPass = std::make_unique<LightRenderPass>();
        m_lightRenderPass = lightRenderPass.get();
        m_renderer.AddRenderPass(std::move(lightRenderPass));
        m_renderer.AddRenderPass(std::move(gbufferRenderpass));
        m_renderer.AddRenderPass(std::make_unique<DeferredRenderPass>(m_deferredMaterial));
    }

    void GrassApplication::UpdateInput()
    {
        Window& window = GetMainWindow();

        glm::vec2 mousePosition = window.GetMousePosition(false);

        if (m_firstMouseMove)
        {
            m_lastMousePosition = mousePosition;
            m_firstMouseMove = false;
        }

        float xOffset = mousePosition.x - m_lastMousePosition.x;
        float yOffset = m_lastMousePosition.y - mousePosition.y;
        m_lastMousePosition = mousePosition;

        if (m_cursorHidden)
        {
            xOffset *= m_settings.cameraSensitivity;
            yOffset *= m_settings.cameraSensitivity;

            m_cameraYaw += xOffset;
            m_cameraPitch += yOffset;
        }

        if (m_cameraPitch > 89.0f)
            m_cameraPitch = 89.0f;
        if (m_cameraPitch < -89.0f)
            m_cameraPitch = -89.0f;

        glm::vec3 cameraDirection = anglesToDirection(m_cameraPitch, m_cameraYaw);

        float movementSpeed = 1.5f;
        movementSpeed *= GetDeltaTime();

        glm::vec3 up(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(cameraDirection, up));
        glm::vec3 cameraUp = glm::normalize(glm::cross(right, cameraDirection));

        if (window.IsKeyPressed(GLFW_KEY_W))
            m_cameraPosition +=
            movementSpeed * glm::normalize(glm::vec3(cameraDirection.x, 0.0f, cameraDirection.z));
        if (window.IsKeyPressed(GLFW_KEY_S))
            m_cameraPosition -=
            movementSpeed * glm::normalize(glm::vec3(cameraDirection.x, 0.0f, cameraDirection.z));
        if (window.IsKeyPressed(GLFW_KEY_D))
            m_cameraPosition +=
            movementSpeed * glm::normalize(glm::cross(cameraDirection, cameraUp));
        if (window.IsKeyPressed(GLFW_KEY_A))
            m_cameraPosition -=
            movementSpeed * glm::normalize(glm::cross(cameraDirection, cameraUp));

        int x = static_cast<int>(m_cameraPosition.x * m_planeGridConversion.x);
        int z = static_cast<int>(m_cameraPosition.z * m_planeGridConversion.y);

        if (m_cameraPosition.x < 0.0f)
            x = m_cameraPosition.x = 0.0f;
        if (m_cameraPosition.x >= m_planeSize.x)
        {
            m_cameraPosition.x = m_planeSize.x;
            x = m_gridPoints.x - 1;
        }

        if (m_cameraPosition.z < 0.0f)
            z = m_cameraPosition.z = 0.0f;
        if (m_cameraPosition.z >= m_planeSize.z)
        {
            m_cameraPosition.z = m_planeSize.z;
            z = m_gridPoints.y - 1;
        }

        float heightSample = m_heights[x + z * m_gridPoints.x] * m_planeSize.y;
        float cameraTerrainDistance = 1.0f;
        m_cameraPosition.y = heightSample + cameraTerrainDistance;

        m_camera.SetViewMatrix(m_cameraPosition, m_cameraPosition + cameraDirection, cameraUp);

        int width, height;
        window.GetDimensions(width, height);
        float aspectRatio = static_cast<float>(width) / height;
        m_camera.SetPerspectiveProjectionMatrix(1.0f, aspectRatio, 0.1f, 100.0f);

        if (!m_keyFPressed && window.IsKeyPressed(GLFW_KEY_F))
        {
            m_keyFPressed = true;
            m_cursorHidden = !m_cursorHidden;
            if (m_cursorHidden)
                window.SetCursorMode(Window::CursorMode::Disabled);
            else
                window.SetCursorMode(Window::CursorMode::Normal);
        }
        else if (m_keyFPressed && window.IsKeyReleased(GLFW_KEY_F))
            m_keyFPressed = false;
    }

    std::vector<float> GrassApplication::CreateHeights(glm::uvec2 gridPoints, glm::ivec2 coords) const
    {
        std::vector<float> heights(gridPoints.x * gridPoints.y);
        for (unsigned int j = 0; j < gridPoints.y; ++j)
        {
            for (unsigned int i = 0; i < gridPoints.x; ++i)
            {
                float x = static_cast<float>(i) / (gridPoints.x - 1) + coords.x;
                float y = static_cast<float>(j) / (gridPoints.y - 1) + coords.y;
                heights[j * gridPoints.x + i] = stb_perlin_fbm_noise3(x, y, 0.0f, 1.9f, 0.5f, 8) * 0.5f;
            }
        }
        return heights;
    }

    struct Tangents
    {
        glm::vec3 tangent;
        glm::vec3 bitangent;
    };

    [[nodiscard]]
    static Tangents tangents(
        const glm::vec3& deltaPos0,
        const glm::vec3& deltaPos1,
        const glm::vec2& deltaUV0,
        const glm::vec2& deltaUV1)
    {
        //Source:
        //https://learnopengl.com/Advanced-Lighting/Normal-Mapping

        float f = 1.0f / (deltaUV0.x * deltaUV1.y - deltaUV1.x * deltaUV0.y);
        glm::vec3 tangent;
        tangent.x = f * (deltaUV1.y * deltaPos0.x - deltaUV0.y * deltaPos1.x);
        tangent.y = f * (deltaUV1.y * deltaPos0.y - deltaUV0.y * deltaPos1.y);
        tangent.z = f * (deltaUV1.y * deltaPos0.z - deltaUV0.y * deltaPos1.z);

        glm::vec3 bitangent;
        bitangent.x = f * (-deltaUV1.x * deltaPos0.x + deltaUV0.x * deltaPos1.x);
        bitangent.y = f * (-deltaUV1.x * deltaPos0.y + deltaUV0.x * deltaPos1.y);
        bitangent.z = f * (-deltaUV1.x * deltaPos0.z + deltaUV0.x * deltaPos1.z);
        return { tangent, bitangent };
    }

    void GrassApplication::CreateTerrainMesh(Mesh& mesh, const std::vector<float>& heights) const
    {
        // Define the vertex structure
        struct Vertex
        {
            Vertex() = default;
            Vertex(
                const glm::vec3& position,
                const glm::vec3& normal,
                const glm::vec2& texCoord,
                const glm::vec3& tangent,
                const glm::vec3& bitangent)
                :
                position(position),
                normal(normal),
                texCoord(texCoord),
                tangent(tangent), 
                bitangent(bitangent) {}
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 texCoord;
            glm::vec3 tangent;
            glm::vec3 bitangent;
        };

        // Define the vertex format (should match the vertex structure)
        VertexFormat vertexFormat;
        vertexFormat.AddVertexAttribute<float>(3);
        vertexFormat.AddVertexAttribute<float>(3);
        vertexFormat.AddVertexAttribute<float>(2);
        vertexFormat.AddVertexAttribute<float>(3);
        vertexFormat.AddVertexAttribute<float>(3);

        // List of vertices (VBO)
        std::vector<Vertex> vertices;

        // List of indices (EBO)
        std::vector<unsigned int> indices;

        // Grid scale to convert the entire grid to size 1x1
        glm::vec2 scale(1.0f / (m_gridPoints.x - 1), 1.0f / (m_gridPoints.y - 1));

        glm::vec2 texCoordScale = scale * 4.0f;

        // Number of columns and rows

        // Iterate over each VERTEX
        for (unsigned int j = 0; j < m_gridPoints.y; ++j)
        {
            for (unsigned int i = 0; i < m_gridPoints.x; ++i)
            {
                // Vertex data for this vertex only
                float height = heights[i + j * m_gridPoints.x];
                glm::vec3 position(i * scale.x, height, j * scale.y);
                glm::vec3 normal(0.0f, 1.0f, 0.0f);
                glm::vec2 texCoord = glm::vec2(i, j) * texCoordScale;
                glm::vec3 tangent(0.0f);
                glm::vec3 bitangent(0.0f);
                vertices.emplace_back(position, normal, texCoord, tangent, bitangent);

                // Index data for quad formed by previous vertices and current
                if (i > 0 && j > 0)
                {
                    unsigned int top_right = j * m_gridPoints.x + i; // Current vertex
                    unsigned int top_left = top_right - 1;
                    unsigned int bottom_right = top_right - m_gridPoints.x;
                    unsigned int bottom_left = bottom_right - 1;

                    //Triangle 1
                    indices.push_back(bottom_left);
                    indices.push_back(bottom_right);
                    indices.push_back(top_left);

                    //Triangle 2
                    indices.push_back(bottom_right);
                    indices.push_back(top_left);
                    indices.push_back(top_right);
                }
            }
        }

        for (uint32_t j = 0; j < m_gridPoints.y - 1; ++j)
        {
            for (uint32_t i = 0; i < m_gridPoints.x - 1; ++i)
            {
                int index = j * m_gridPoints.x + i;
                Vertex& vertex = vertices[index];

                unsigned int prevX = i > 0 ? index - 1 : index;
                unsigned int nextX = i < m_gridPoints.x ? index + 1 : index;

                const auto& prevXVertex = vertices[prevX];
                const auto& nextXVertex = vertices[nextX];

                glm::vec3 prevNormalX = vertex.position - prevXVertex.position;
                prevNormalX = glm::vec3(-prevNormalX.y, prevNormalX.x, 0.0f);

                glm::vec3 nextNormalX = nextXVertex.position - vertex.position;
                nextNormalX = glm::vec3(-nextNormalX.y, nextNormalX.x, 0.0f);

                int prevZ = j > 0 ? index - m_gridPoints.x : index;
                int nextZ = j < m_gridPoints.y ? index + m_gridPoints.x : index;

                const auto& prevZVertex = vertices[prevZ];
                const auto& nextZVertex = vertices[nextZ];

                glm::vec3 prevNormalZ = vertex.position - prevZVertex.position;
                prevNormalZ = glm::vec3(0.0f, prevNormalZ.z, -prevNormalZ.y);

                glm::vec3 nextNormalZ = nextZVertex.position - vertex.position;
                nextNormalZ = glm::vec3(0.0f, nextNormalZ.z, -nextNormalZ.y);

                glm::vec3 normal = prevNormalX + nextNormalX + prevNormalZ + nextNormalZ;
                normal = glm::normalize(normal);
                vertex.normal = normal;

                glm::vec3 deltaPos0 = nextXVertex.position - vertex.position;
                glm::vec3 deltaPos1 = nextZVertex.position - vertex.position;

                glm::vec2 deltaUV0 = nextXVertex.texCoord - vertex.texCoord;

                glm::vec2 deltaUV1 = nextZVertex.texCoord - vertex.texCoord;

                Tangents t = tangents(deltaPos0, deltaPos1, deltaUV0, deltaUV1);
                vertex.tangent = t.tangent;
                vertex.bitangent = t.bitangent;
            }
        }

        mesh.AddSubmesh<Vertex, unsigned int, VertexFormat::LayoutIterator>(Drawcall::Primitive::Triangles, vertices, indices,
            vertexFormat.LayoutBegin(static_cast<int>(vertices.size()), true /* interleaved */), vertexFormat.LayoutEnd());
    }

    void GrassApplication::CreateGrassMesh(Mesh& mesh, const std::vector<float>& heights, uint32_t& grassSubmeshIndex) const
    {
        struct Vertex
        {
            const glm::vec3 position;
            const glm::vec2 texCoord;

            Vertex() = default;
            Vertex(
                const glm::vec3& position,
                const glm::vec2& texCoord)
                : position(position), texCoord(texCoord) {}
        };

        VertexFormat vertexFormat;
        vertexFormat.AddVertexAttribute<float>(3); // position
        vertexFormat.AddVertexAttribute<float>(2); // texCoord

        float strawRadius = 0.01f;
        float strawBottomHeight = 0.25f;
        float strawTopHeight = 0.25f;
        float strawTotalHeight = strawBottomHeight + strawTopHeight;

        std::vector<Vertex> vertices
        {
            Vertex(glm::vec3(strawRadius, strawBottomHeight, 0.0f), glm::vec2(1.0f, strawBottomHeight / strawTotalHeight)),
            Vertex(glm::vec3(strawRadius, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
            Vertex(glm::vec3(-strawRadius, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
            Vertex(glm::vec3(-strawRadius, strawBottomHeight, 0.0f), glm::vec2(0.0f, strawBottomHeight / strawTotalHeight)),
            Vertex(glm::vec3(0.0f, strawTotalHeight, 0.0f), glm::vec2(0.5f, 1.0f))
        };

        std::vector<uint32_t> indices
        {
            0, 1, 3,
            3, 2, 1,
            3, 4, 0
        };

        struct Instance
        {
            glm::vec3 position;
            float rotation;
            float heightMultiplier;

            Instance(glm::vec3 position, float rotation, float heightMultiplier)
                :
                position(position),
                rotation(rotation),
                heightMultiplier(heightMultiplier) { }
        };

        VertexFormat instanceFormat;
        instanceFormat.AddVertexAttribute<float>(3); // position
        instanceFormat.AddVertexAttribute<float>(1); // rotation
        instanceFormat.AddVertexAttribute<float>(1); // heightMultiplier

        std::vector<Instance> instances;

        RandomReal randomX(0.0f, m_gridPoints.x);
        RandomReal randomZ(0.0f, m_gridPoints.y);
        RandomReal randomHeightMultiplier(0.2f, 1.5f);
        RandomReal randomRotation(0.0f, glm::two_pi<float>());
        for (size_t i = 0; i < m_generatedGrassStraws; i++)
        {
            float x = randomX.Get();
            float z = randomZ.Get();
            float rotation = randomRotation.Get();
            int xi = static_cast<float>(x);
            int zi = static_cast<float>(z);
            if (xi >= m_gridPoints.x)
                xi = m_gridPoints.x - 1;
            if (zi >= m_gridPoints.y)
                zi = m_gridPoints.y - 1;
            float y = heights[xi + zi * m_gridPoints.x] * m_planeSize.y;
            x /= m_planeGridConversion.x;
            z /= m_planeGridConversion.y;
            float heightMultiplier = randomHeightMultiplier.Get();
            instances.emplace_back(glm::vec3(x, y, z), rotation, heightMultiplier);
        }

        grassSubmeshIndex = mesh.AddSubmesh<Vertex, uint32_t, VertexFormat::LayoutIterator, Instance>(
            Drawcall::Primitive::Triangles, vertices, indices, instances,
            vertexFormat.LayoutBegin(static_cast<int>(vertices.size()), true),
            vertexFormat.LayoutEnd(),
            instanceFormat.LayoutBegin(static_cast<int>(instances.size()), true),
            instanceFormat.LayoutEnd());
    }
}
