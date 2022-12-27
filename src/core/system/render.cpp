#include "render.h"

#include "raymath.h"
#include "core/component/components.h"

RenderSystem::RenderSystem()
    : m_isWiresMode(false)
{
    m_shadowMap = LoadShadowMap(PHYSBOX_SHADOW_MAP_WIDTH, PHYSBOX_SHADOW_MAP_WIDTH);
    m_shadowShader = LoadShadowShader();
    m_geometryShader = LoadShadedGeometryShader();
    m_previewShader = LoadDepthPreviewShader();
}

void RenderSystem::update(float dt)
{
    auto entityView = getRegistry().view<TransformComponent, RenderComponent>();
    
    // Synchronize transform
    for (auto [entity, transformComponent, renderComponent] : entityView.each())
    {
        renderComponent.model.transform = transformComponent.transform;
    }

    // Find shadow caster (directional light only for now)
    auto lightsView = getRegistry().view<LightComponent>();
    bool casterFound = false;
    Camera caster = { 0 };
    for (auto [entity, lightComponent] : lightsView.each())
    {
        if (lightComponent.light.type = LIGHT_DIRECTIONAL)
        {
            caster = lightComponent.caster;
            casterFound = true;
        }
            
    }
    assert(casterFound);

    // Draw to shadow map (z-buffer)
    ShadowMapBegin(m_shadowMap);
        BeginShadowCaster(caster);
            for (auto [entity, transformComponent, renderComponent] : entityView.each())
            {
                updateShadow(renderComponent.model);
            }
        EndShadowCaster();
    ShadowMapEnd();

    // Draw scene
    //BeginDrawing();
    //ClearBackground(RAYWHITE);

}

void RenderSystem::updateShadow(const Model& model)
{
    model.materials[0].shader = m_shadowShader;
    DrawModel(model, Vector3Zero(), 1.f, WHITE);
}

void RenderSystem::updateGeometry(const Model& model, const float shadowFactor)
{
    model.materials[0].shader = m_geometryShader;
    model.materials[0].maps[MATERIAL_MAP_SHADOW].texture = m_shadowMap.depth;
    if (m_isWiresMode)
    {
        DrawModelWires(model, Vector3Zero(), 1.f, RED);
    }
    else
    {
        SetShaderValue(m_geometryShader, m_geometryShader.locs[SHADER_LOC_SHADOW_FACTOR], &shadowFactor, SHADER_UNIFORM_FLOAT);
        DrawModel(model, Vector3Zero(), 1.f, WHITE);
    }
}