#include "core/system/render.h"

template<>
struct SystemDebugger<RenderSystem>
{
    void setWiresMode(bool wiresMode)
    {
        RenderSystem::getSystem().m_isWiresMode = wiresMode;
    }

    void setDrawShadowMap(bool drawShadowMap)
    {
        RenderSystem::getSystem().m_drawShadowMap = drawShadowMap;
    }

    void setDrawLightSource(bool drawLights)
    {
        RenderSystem::getSystem().m_drawLightSource = drawLights;
    }
};