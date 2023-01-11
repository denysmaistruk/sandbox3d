#pragma once

#include <raylib.h>
#include <stdint.h>

struct ShadowMap : Texture2D {
    uint32_t    frameBufferId;
};

ShadowMap LoadShadowMap(int width, int height);

void UnloadShadowMap(ShadowMap shadowMap);
void ShadowMapBegin(ShadowMap shadowMap);
void ShadowMapEnd();

void BeginShadowCaster(Camera3D camera);
void EndShadowCaster();

Matrix CusterPerspective(Camera3D const& camera);

#define _CONCAT(x, y) x ## y
#define CONCAT(x, y) _CONCAT(x, y)
#define SCOPE_EXIT(TODO) auto const CONCAT(__scopeExit_,__LINE__) = makeScopeExit(TODO)

template <typename ToDo>
auto makeScopeExit(ToDo&& todo) {
    struct ScopeExit {
        ToDo todo; ~ScopeExit() { todo(); }
    };
    return ScopeExit{ todo };
};

#define DEFINE_PADDING(SIZE) uint32_t CONCAT(__pad_,__LINE__)[SIZE]
