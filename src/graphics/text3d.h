#pragma once
#include "raylib.h"

Shader LoadText3DShader();

void DrawTextCodepoint3D(Font font, int codepoint, Vector3 position, float fontSize, bool backface, Color tint, bool drawBoundary);
void DrawText3D(Font font, const char* text, Vector3 position, float fontSize, float fontSpacing, float lineSpacing, bool backface, Color tint, bool drawBoundary);
Vector3 MeasureText3D(Font font, const char* text, float fontSize, float fontSpacing, float lineSpacing);