#pragma once
#include "system_base.h"

#include "raylib.h"
#include "utils/graphics/lights.h"

struct ImGuiIO;

template<typename System>
struct SystemDebugger;

class RenderSystem : public SystemBase<RenderSystem>
{
    SANDBOX3D_SYSTEM_CLASS(RenderSystem);
    friend struct SystemDebugger<RenderSystem>;
public:
    void update(float dt);

    const ShadowMap& getShadowMap() { return m_shadowMap; }
    const Shader& getShadowShader() { return m_shadowShader; }
    const Shader& getGeometryShader() { return m_geometryShader; }
    const Shader& getPreviewShader() { return m_previewShader; }

    void unloadAllModels();
    void unloadAllShaders();

    int addDebugDrawCallback(const std::function<void()>& callBack);
    void removeDebugDrawCallback(int index);

    static ImGuiIO* ImGuiInit();

protected:
    RenderSystem(size_t id);

private:
    void drawShadow(const Model& model);
    void drawGeometry(const Model& model, const float shadowFactor);
    void drawLightSource(const Light& light);
    void drawDebugGeometry();

    void ImGuiBegin();
    void ImGuiEnd();
    void ImGuiWidgets();

    ShadowMap m_shadowMap;
    Shader m_shadowShader;
    Shader m_geometryShader;
    Shader m_previewShader;

    bool m_isWiresMode;
    bool m_drawShadowMap;
    bool m_drawLightSource;

    std::vector<std::function<void()>> m_debugDrawCallbacks;
};