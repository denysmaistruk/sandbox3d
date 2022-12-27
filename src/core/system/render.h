#pragma once
#include "system_base.h"

#include "raylib.h"
#include "utils/graphics/lights.h"

template<typename System>
struct SystemDebugger;

class RenderSystem : SystemBase<RenderSystem>
{
    friend SystemBase<RenderSystem>;
    friend struct SystemDebugger<RenderSystem>;
public:
    void update(float dt);

    void setShadowMap(const ShadowMap& shadowMap) { m_shadowMap = shadowMap; }
    void setShadowShader(const Shader& shader) { m_shadowShader = shader; }
    void setGeometryShader(const Shader& shader) { m_geometryShader = shader; }
    void setPreviewShader(const Shader& shader) { m_previewShader = shader; }

    static void ImGuiInit();

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
};