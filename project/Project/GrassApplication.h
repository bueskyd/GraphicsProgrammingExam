#pragma once

#include <ituGL/application/Application.h>
#include <ituGL/geometry/Mesh.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/shader/Material.h>
#include <ituGL/texture/Texture2DObject.h>
#include <ituGL/geometry/Model.h>
#include <ituGL/renderer/Renderer.h>
#include <ituGL/lighting/PointLight.h>
#include <ituGL/lighting/SpotLight.h>
#include <ituGL/lighting/DirectionalLight.h>
#include <ituGL/utils/DearImGui.h>
#include <vector>

class LightRenderPass;

namespace proj
{
    class GrassApplication : public Application
    {
    private:
        struct Settings
        {
            uint32_t grassStraws = 0;

            glm::vec3 ambientColor = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
            float lightIntensity = 5.0f;

            glm::vec2 windDirection = glm::normalize(glm::vec2(1.0f, 1.0f));
            float windSpeed = 0.25f;

            float lightPitch = -52.0f, lightYaw = 0.0f;
            float ambientOcclusion = 1.0f;

            glm::vec3 skyColor = glm::vec3(0.5294f, 0.8078f, 0.9215f);

            float cameraSensitivity = 0.25f;

            bool shadowMapEnabled = true;
        };
    public:
        GrassApplication();
    protected:
        void Initialize() override;
        void Update() override;
        void Render() override;
        void Cleanup() override;
    private:
        void InitializeGround();
        void InitializeGrass();
        void InitializeCamera();
        void InitializeDeferredMaterials();
        void RenderGUI();
        Renderer::UpdateLightsFunction GetUpdateLightsFunction(
            std::shared_ptr<ShaderProgram> shaderProgram);
        void InitializeRenderer();
        void UpdateInput();
        std::vector<float> CreateHeights(
            glm::uvec2 gridPoints, glm::ivec2 coords) const;
        void CreateTerrainMesh(
            Mesh& mesh, const std::vector<float>& heights) const;
        void CreateGrassMesh(Mesh& mesh, const std::vector<float>& heights, uint32_t& grassSubmeshIndex) const;

        Camera m_camera;
        glm::vec3 m_cameraPosition = glm::vec3(0.0f, 2.5f, 0.0f);
        Model m_groundModel;
        Model m_grassModel;
        float m_cameraYaw = 45.0f;
        float m_cameraPitch = 0.0f;
        glm::vec2 m_lastMousePosition = glm::vec2(0.0f);
        bool m_cursorHidden = true;
        glm::uvec2 m_gridPoints;
        glm::uvec3 m_planeSize;
        glm::uvec2 m_planeGridConversion;
        uint32_t m_generatedGrassStraws;
        Settings m_settings;
        Settings m_defaultSettings = m_settings;
        uint32_t m_grassSubmeshIndex;
        std::vector<float> m_heights;
        std::shared_ptr<Material> m_gbufferMaterial;
        std::shared_ptr<Material> m_deferredMaterial;
        Renderer m_renderer;
        LightRenderPass* m_lightRenderPass = nullptr;

        DirectionalLight m_light;

        bool m_keyFPressed = false;
        bool m_firstMouseMove = true;

        DearImGui m_imGui;
    };
}
