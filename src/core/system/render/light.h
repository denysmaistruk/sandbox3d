#pragma once

#include "raylib.h"
#include "utils.h"

#include "core/system/system_base.h"
#include "core/component/components.h"
#include "core/component/light.h"

class LightingSystem : public SystemBase<LightingSystem>
{
    SANDBOX3D_SYSTEM_CLASS(LightingSystem);
public:
    static constexpr uint32_t k_shadowAtlasRowSize          = 4;
    static constexpr uint32_t k_shadowAtlasResolution       = 4096;
    static constexpr uint32_t k_shadowAtlasCellCount        = (k_shadowAtlasRowSize     * k_shadowAtlasRowSize);
    static constexpr uint32_t k_shadowAtlasCellResolution   = (k_shadowAtlasResolution  / k_shadowAtlasRowSize);

    template <typename Draw>
    void prepareLightingData(Draw& drawFunction);

    Texture const& getShadowMapAtlasTexture() const { return m_shadowMapAtlas; }

protected:
    LightingSystem(size_t);
    void update(float dt) {};

private:
    enum class LightType {
        Directional,
        Spot, Point
    };

    struct LightData
    {
        LightType   type;
        uint32_t    shadowMapId;
        Vector3     position;
        Vector3     target;
        Color       color;
        float       cutoff;
        float       radius;
        float       softness;
    };

    Shader      m_shadowShader      = {};
    ShadowMap   m_shadowMapAtlas    = {};
    uint32_t    m_shadowDataBuffer  = -1;
    uint32_t    m_lightDataBuffer   = -1;
    Matrix      m_shadowDataArray   [k_shadowAtlasCellCount] = {};
    LightData   m_lightDataArray    [k_shadowAtlasCellCount] = {};
};

template<typename Draw>
inline void LightingSystem::prepareLightingData(Draw& drawFunction) {
    auto const& registry    = getRegistry();
    auto const  drawObject  = [&] (Model model) {
        model.materials[0].shader = m_shadowShader;
        DrawModel(model, Vector3Zero(), 1.f, WHITE);
    };
    auto const  renderCell  = [&](Camera const& camera, int const cellIndex)
    {
        int const x = (cellIndex / k_shadowAtlasRowSize) * k_shadowAtlasCellResolution;
        int const y = (cellIndex % k_shadowAtlasRowSize) * k_shadowAtlasCellResolution;
        rlViewport(x, y, k_shadowAtlasCellResolution, k_shadowAtlasCellResolution);
        BeginShadowCaster(camera);
        drawFunction(drawObject);
        EndShadowCaster();
    };
    auto const shadowMatrix = [](Camera const& camera) {
        auto const world    = GetCameraMatrix(camera);
        auto const proj     = CameraPerspective(camera);
        return MatrixTranspose(MatrixMultiply(world, proj));
    };

    uint32_t cellId = 0;

    ShadowMapBegin(m_shadowMapAtlas);
    {
        auto const  inactive    = entt::exclude<Inactive>;
        auto const  dirLight    = registry.view<Color, Position, LookAt, DirectionalLight, ShadowCaster>   (inactive);
        auto const  spotLight   = registry.view<Color, Position, LookAt, SpotLight, ShadowCaster>          (inactive);
        //auto const  pointLight  = registry.view<Color, Position, PointLight, ShadowCaster>                 (inactive);

        Camera camera = {};
        camera.fovy = 90.0f;
        camera.up   = Vector3{ 0.0f, 1.0f, 0.0f };
        camera.projection = CAMERA_ORTHOGRAPHIC;

        LightData light = {};
        light.type = LightType::Directional;
        for (auto [entity, color, position, lookAt] : dirLight.each())
        {
            if (!(cellId < k_shadowAtlasCellCount))
                break;
            light.shadowMapId = cellId;
            light.color     = color;
            light.position  = camera.position   = position;
            light.target    = camera.target     = lookAt;
            m_lightDataArray    [cellId] = light;
            m_shadowDataArray   [cellId] = shadowMatrix(camera);
            renderCell(camera, cellId++);
        }

        light.type = LightType::Spot;
        camera.projection = CAMERA_PERSPECTIVE;
        for (auto [entity, color, position, lookAt, spot] : spotLight.each())
        {
            if (!(cellId < k_shadowAtlasCellCount))
                break;
            light.shadowMapId = cellId;
            light.color     = color;
            light.cutoff    = spot.cutoff;
            light.radius    = spot.radius;
            light.softness  = spot.softness;
            light.position  = camera.position   = position;
            light.target    = camera.target     = lookAt;
            m_lightDataArray    [cellId] = light;
            m_shadowDataArray   [cellId] = shadowMatrix(camera);
            renderCell(camera, cellId++);
        }

        // @TODO: handle point lights
        rlUpdateShaderBufferElements(m_shadowDataBuffer, m_shadowDataArray, sizeof(Matrix) * cellId, 0);
    }
    ShadowMapEnd();

    {
        auto const  inactive    = entt::exclude<Inactive, ShadowCaster>;
        auto const  dirLight    = registry.view<Color, Position, LookAt, DirectionalLight>  (inactive);
        auto const  spotLight   = registry.view<Color, Position, LookAt, SpotLight>         (inactive);
        //auto const  pointLight  = registry.view<Color, Position, PointLight>                (inactive);

        LightData light = {};
        light.type = LightType::Directional;
        light.shadowMapId = -1;
        for (auto [entity, color, position, lookAt] : dirLight.each())
        {
            if (!(cellId < k_shadowAtlasCellCount))
                break;
            light.color     = color;
            light.position  = position;
            light.target    = lookAt;
            m_lightDataArray[cellId] = light;
        }

        light.type = LightType::Spot;
        for (auto [entity, color, position, lookAt, spot] : spotLight.each())
        {
            if (!(cellId < k_shadowAtlasCellCount))
                break;
            light.color     = color;
            light.cutoff    = spot.cutoff;
            light.radius    = spot.radius;
            light.softness  = spot.softness;
            light.position  = position;
            light.target    = lookAt;
            m_lightDataArray[cellId] = light;
        }

        // @TODO: handle point lights
        rlUpdateShaderBufferElements(m_lightDataBuffer, m_lightDataArray, sizeof(LightData) * cellId, 0);
    }
}
