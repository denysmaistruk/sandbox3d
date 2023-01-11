#include "core/system/render/render.h"

template<>
struct SystemDebugger<RenderSystem>
{
    void setDrawText(bool drawText)
    {
        RenderSystem::getSystem().m_drawText = drawText;
    }

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