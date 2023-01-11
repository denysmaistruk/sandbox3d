#pragma once

#include "raylib.h"
#include "utils.h"

#include "core/system/system_base.h"
#include "core/component/components.h"
#include "core/component/light.h"

class LightingSystem : public SystemBase<LightingSystem>
{
    SANDBOX3D_DECLARE_SYSTEM(LightingSystem);
public:
    static constexpr uint32_t k_shadowAtlasRowSize          = 4;
    static constexpr uint32_t k_shadowAtlasResolution       = 4096;
    static constexpr uint32_t k_shadowAtlasCellCount        = (k_shadowAtlasRowSize     * k_shadowAtlasRowSize);
    static constexpr uint32_t k_shadowAtlasCellResolution   = (k_shadowAtlasResolution  / k_shadowAtlasRowSize);
    static constexpr uint32_t k_lightSourceCount            = k_shadowAtlasCellCount;
    static constexpr uint32_t k_invalidShadowCellId         = -1;

    template <typename Scene>
    void prepareLightingData(Scene& drawScene);
    void bindLightingData(Shader const& shader, Vector3 const& cameraPos);

    Texture const& getShadowMapAtlasTexture() const { return m_shadowMapAtlas; }

protected:
    LightingSystem(size_t);
    void update(float dt) {};

private:
    enum class LightType : uint32_t {
        Directional,
        Spot, Point
    };

    struct LightData {
        LightType   type;
        uint32_t    shadowId;
        Color       color;
        float       cutoff;
        float       radius;
        float       softness;
        DEFINE_PADDING(2);
        Vector3     position;   DEFINE_PADDING(1);
        Vector3     target;     DEFINE_PADDING(1);
    };

    Shader      m_shadowShader      = {};
    ShadowMap   m_shadowMapAtlas    = {};
    uint32_t    m_lightDataBuffer   = -1;
    uint32_t    m_lightDataCount    = 0;
    LightData   m_lightDataArray    [k_lightSourceCount] = {};
};

template<typename Scene>
inline void LightingSystem::prepareLightingData(Scene& drawScene) {
    auto const& registry    = getRegistry();
    auto const  drawObject  = [&] (Model model) {
        model.materials[0].shader = m_shadowShader;
        DrawModel(model, Vector3Zero(), 1.f, WHITE);
    };
    uint32_t    shadowCellId = 0;
    auto const  renderShadowCell = [&](entt::entity entity, LightData const& light) {
        bool const isCaster = registry.any_of<ShadowCaster>(entity);
        bool const isValidId= shadowCellId < k_shadowAtlasCellCount;
        bool const isValid  = isCaster && isValidId;
        if (!isValid)       return k_invalidShadowCellId;

        Camera  camera      = {};
        camera.fovy         = 90.0f;
        camera.target       = light.target;
        camera.position     = light.position;
        camera.up           = Vector3{ 0.0f, 1.0f, 0.0f };
        camera.projection   = LightType::Directional == light.type
                            ? CAMERA_ORTHOGRAPHIC : CAMERA_PERSPECTIVE;
        int const offsetX   = (shadowCellId / k_shadowAtlasRowSize) * k_shadowAtlasCellResolution;
        int const offsetY   = (shadowCellId % k_shadowAtlasRowSize) * k_shadowAtlasCellResolution;
        rlViewport(offsetX, offsetY, k_shadowAtlasCellResolution, k_shadowAtlasCellResolution);
        BeginShadowCaster(camera);
        drawScene(drawObject);
        EndShadowCaster();
        return shadowCellId++;
    };

    uint32_t lightCellId = 0;
    memset(m_lightDataArray, 0, sizeof(m_lightDataArray));
    SCOPE_EXIT([&] {
        m_lightDataCount = lightCellId;
        rlUpdateShaderBufferElements
            ( m_lightDataBuffer
            , m_lightDataArray
            , m_lightDataCount
            * sizeof(LightData), 0 );
    });

    {   // prepare data for lights with shadow mask
        ShadowMapBegin(m_shadowMapAtlas);
        SCOPE_EXIT(ShadowMapEnd);

        auto const  inactive    = entt::exclude<Inactive>;
        auto const  dirLight    = registry.view<Color, Position, LookAt, DirectionalLight>   (inactive);
        auto const  spotLight   = registry.view<Color, Position, LookAt, SpotLight>          (inactive);
        auto const  pointLight  = registry.view<Color, Position, PointLight>                 (inactive);

        LightData light = {};
        light.type = LightType::Directional;
        for (auto [entity, color, position, lookAt] : dirLight.each())
        {
            if (lightCellId >= k_lightSourceCount) break;
            light.color     = color;
            light.target    = lookAt;
            light.position  = position;
            light.shadowId  = renderShadowCell(entity, light);
            m_lightDataArray[lightCellId++] = light;
        }

        light.type = LightType::Spot;
        for (auto [entity, color, position, lookAt, spot] : spotLight.each())
        {
            if (lightCellId >= k_lightSourceCount) break;
            light.color     = color;
            light.target    = lookAt;
            light.position  = position;
            light.cutoff    = spot.cutoff;
            light.radius    = spot.radius;
            light.softness  = spot.softness;
            light.shadowId  = renderShadowCell(entity, light);
            m_lightDataArray[lightCellId++] = light;
        }

        // @TODO: no shadows for point lights for now...
        light.type = LightType::Point;
        for (auto [entity, color, position, point] : pointLight.each())
        {
            if (lightCellId >= k_lightSourceCount) break;
            light.color     = color;
            light.position  = position;
            light.radius    = point.radius;
            light.softness  = point.softness;
            light.shadowId  = k_invalidShadowCellId;
            m_lightDataArray[lightCellId++] = light;
        }
    }
}
