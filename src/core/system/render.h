#pragma once
#include "system_base.h"

#include "raylib.h"

#include "graphics/lights.h"

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
    void removeDebugDrawCallback(int key);

    int addText3dMessage(const char* message, const Vector3& pos);
    void updateText3dMessage(int key, const char* newMessage, const Vector3& newPos);
    void removeText3dMessage(int key);

    static ImGuiIO* ImGuiInit();

protected:
    RenderSystem(size_t id);

private:
    void drawShadow(const Model& model);
    void drawGeometry(const Model& model, const float shadowFactor);
    void drawLightSource(const Light& light);
    void drawText3D();
    void drawDebugGeometry();
    
    void ImGuiBegin();
    void ImGuiEnd();
    void ImGuiWidgets();

    void ImGuizmoBegin();
    void ImGuizmoWidgets();

    ShadowMap m_shadowMap;
    Shader m_shadowShader;
    Shader m_geometryShader;
    Shader m_previewShader;
    Shader m_text3dShader;

    bool m_isWiresMode;
    bool m_drawShadowMap;
    bool m_drawLightSource;
    bool m_drawText3d;

    std::map<int, std::pair<const char*, Vector3>> m_text3dMessages;
    std::map<int, std::function<void()>> m_debugDrawCallbacks;
};