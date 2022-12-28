#pragma once
#include "system_base.h"

#include "raylib.h"
#include "utils/graphics/lights.h"

struct ImGuiIO;

template<typename System>
struct SystemDebugger;

class RenderSystem : public SystemBase<RenderSystem>
{
    DECLARE_PHYSBOX_SYSTEM(RenderSystem);
public:
    void update(float dt);

    const ShadowMap& getShadowMap() { return m_shadowMap; }
    const Shader& getShadowShader() { return m_shadowShader; }
    const Shader& getGeometryShader() { return m_geometryShader; }
    const Shader& getPreviewShader() { return m_previewShader; }

    void unloadAllModels();
    void unloadAllShaders();

    static ImGuiIO* ImGuiInit();

protected:
    RenderSystem();

private:
    void drawShadow(const Model& model);
    void drawGeometry(const Model& model, const float shadowFactor);

    void ImGuiBegin();
    void ImGuiEnd();
    void ImGuiWidgets();

    ShadowMap m_shadowMap;
    Shader m_shadowShader;
    Shader m_geometryShader;
    Shader m_previewShader;

    bool m_isWiresMode;
    bool m_drawShadowMap;
};