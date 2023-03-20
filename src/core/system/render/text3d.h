#pragma once
#include "raylib.h"

/**
 * List of functions useful in raylib context
 */

Shader loadText3DShader();
Font getFontText3D();

void drawTextCodepoint3D(Font font, int codepoint, Vector3 position, float fontSize, bool backface, Color tint, bool drawBoundary);
void drawText3DImpl(Font font, const char* text, Vector3 position, float fontSize, float fontSpacing, float lineSpacing, bool backface, Color tint, bool drawBoundary);

Vector3 measureText3D(Font font, const char* text, float fontSize, float fontSpacing, float lineSpacing);