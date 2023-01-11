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
    static constexpr uint32_t k_lightSourceCount            = k_shadowAtlasCellCount;
    static constexpr uint32_t k_invalidShadowCellId         = -1;

    template <typename Scene>
    void prepareLightingData(Scene& drawScene);
    void renderLights(int width, int height);

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
        uint32_t    shadowId;
        Vector3     position;
        Vector3     target;
        Color       color;
        float       cutoff;
        float       radius;
        float       softness;
    };

    Shader      m_shadowShader      = {};
    ShadowMap   m_shadowMapAtlas    = {};
    uint32_t    m_lightDataBuffer   = -1;
    LightData   m_lightDataArray    [k_lightSourceCount] = {};
};

template<typename Scene>
inline void LightingSystem::prepareLightingData(Scene& drawScene) {
    auto const& registry    = getRegistry();
    auto const  drawObject  = [&] (Model model) {
        model.materials[0].shader = m_shadowShader;
        DrawModel(model, Vector3Zero(), 1.f, WHITE);
    };
    auto const  renderShadowCell = [&](Camera const& camera, uint32_t const cellIndex) {
        bool    const isShadowMasked = cellIndex < k_shadowAtlasCellCount;
        int     const x = (cellIndex / k_shadowAtlasRowSize) * k_shadowAtlasCellResolution;
        int     const y = (cellIndex % k_shadowAtlasRowSize) * k_shadowAtlasCellResolution;
        if (!isShadowMasked) return k_invalidShadowCellId;
        rlViewport(x, y, k_shadowAtlasCellResolution, k_shadowAtlasCellResolution);
        BeginShadowCaster(camera);
        drawScene(drawObject);
        EndShadowCaster();
        return cellIndex;
    };

    uint32_t cellId = 0;
    SCOPE_EXIT([&] {
        rlUpdateShaderBufferElements(m_lightDataBuffer, m_lightDataArray, sizeof(LightData) * cellId, 0);
    });

    {   // prepare data for lights with shadow mask
        ShadowMapBegin(m_shadowMapAtlas);
        SCOPE_EXIT(ShadowMapEnd);

        auto const  inactive    = entt::exclude<Inactive>;
        auto const  dirLight    = registry.view<Color, Position, LookAt, DirectionalLight, ShadowCaster>   (inactive);
        auto const  spotLight   = registry.view<Color, Position, LookAt, SpotLight, ShadowCaster>          (inactive);
        //auto const  pointLight  = registry.view<Color, Position, PointLight, ShadowCaster>                 (inactive);

        Camera camera   = {};
        camera.fovy     = 90.0f;
        camera.up       = Vector3{ 0.0f, 1.0f, 0.0f };
        camera.projection = CAMERA_ORTHOGRAPHIC;

        LightData light = {};
        light.type = LightType::Directional;
        for (auto [entity, color, position, lookAt] : dirLight.each())
        {
            if (cellId >= k_lightSourceCount) break;
            light.color     = color;
            light.target    = camera.target     = lookAt;
            light.position  = camera.position   = position;
            light.shadowId  = renderShadowCell(camera, cellId);
            m_lightDataArray[cellId++] = light;
        }

        light.type = LightType::Spot;
        camera.projection = CAMERA_PERSPECTIVE;
        for (auto [entity, color, position, lookAt, spot] : spotLight.each())
        {
            if (cellId >= k_lightSourceCount) break;
            light.color     = color;
            light.cutoff    = spot.cutoff;
            light.radius    = spot.radius;
            light.softness  = spot.softness;
            light.target    = camera.target     = lookAt;
            light.position  = camera.position   = position;
            light.shadowId  = renderShadowCell(camera, cellId);
            m_lightDataArray[cellId++] = light;
        }
    }

    {   // prepare data for rest of light sources
        auto const  inactive    = entt::exclude<Inactive, ShadowCaster>;
        auto const  dirLight    = registry.view<Color, Position, LookAt, DirectionalLight>  (inactive);
        auto const  spotLight   = registry.view<Color, Position, LookAt, SpotLight>         (inactive);
        //auto const  pointLight  = registry.view<Color, Position, PointLight>                (inactive);

        LightData light = {};
        light.type      = LightType::Directional;
        light.shadowId  = k_invalidShadowCellId;
        for (auto [entity, color, position, lookAt] : dirLight.each())
        {
            if (cellId >= k_lightSourceCount) break;
            light.color     = color;
            light.target    = lookAt;
            light.position  = position;
            m_lightDataArray[cellId++] = light;
        }

        light.type = LightType::Spot;
        for (auto [entity, color, position, lookAt, spot] : spotLight.each())
        {
            if (cellId >= k_lightSourceCount) break;
            light.cutoff    = spot.cutoff;
            light.radius    = spot.radius;
            light.softness  = spot.softness;
            light.position  = position;
            light.target    = lookAt;
            light.color     = color;
            m_lightDataArray[cellId++] = light;
        }

        // @TODO: handle point lights
    }
}
