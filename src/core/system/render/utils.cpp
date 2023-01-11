#include "utils.h"

#include "rlgl.h"
#include "raymath.h"

#include "../../extern/raylib/src/external/glad.h"

#define SHADOW_PATH SANDBOX3D_SHADER_PATH "lights/"

#define PB_ORTHOGRAPHIC_CAMERA_CULL_DISTANCE_FAR 100

ShadowMap LoadShadowMap(int width, int height) 
{
	ShadowMap shadowMap	= {};
	shadowMap.frameBufferId = rlLoadFramebuffer(width, height);

	rlEnableFramebuffer(shadowMap.frameBufferId);

	shadowMap.id = rlLoadTextureDepth(width, height, false);
	shadowMap.width = width;
	shadowMap.height = height;
	shadowMap.format = PIXELFORMAT_COMPRESSED_PVRT_RGBA;
	shadowMap.mipmaps = 1;

	rlTextureParameters(shadowMap.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_CLAMP);
	rlTextureParameters(shadowMap.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_CLAMP);
	rlFramebufferAttach(shadowMap.frameBufferId, shadowMap.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

	rlDisableFramebuffer();

	return shadowMap;
}

void UnloadShadowMap(ShadowMap shadowMap) 
{
	rlUnloadFramebuffer(shadowMap.frameBufferId);
	rlUnloadTexture(shadowMap.id);
}

void ShadowMapBegin(ShadowMap shadowMap) 
{
	rlDrawRenderBatchActive();
	rlEnableFramebuffer(shadowMap.id);
	rlDisableColorBlend();
	rlClearColor(255, 255, 255, 255);
	glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_FRONT);				// Front faces culling for solving shadow acne problem

	// Set viewport to framebuffer size
	rlViewport(0, 0, shadowMap.width, shadowMap.height);

	rlMatrixMode(RL_PROJECTION);        // Switch to projection matrix
	rlLoadIdentity();                   // Reset current matrix (projection)

	// Set orthographic projection to current framebuffer size
	// NOTE: Configured top-left corner as (0, 0)
	rlOrtho(0, shadowMap.width, shadowMap.height, 0, 0, 1);

	rlMatrixMode(RL_MODELVIEW);         // Switch back to modelview matrix
	rlLoadIdentity();                   // Reset current matrix (modelview)
}

void EndShadowCaster() 
{ 
	glCullFace(GL_BACK);				// Switch back to back faces culling
	EndMode3D(); 
}

void BeginShadowCaster(Camera3D camera)
{
	rlDrawRenderBatchActive();      // Update and draw internal render batch

	rlMatrixMode(RL_PROJECTION);    // Switch to projection matrix
	rlPushMatrix();                 // Save previous matrix, which contains the settings for the 2d ortho projection
	rlLoadIdentity();               // Reset current matrix (projection)

	float aspect = 1;

	// NOTE: zNear and zFar values are important when computing depth buffer values
	if (camera.projection == CAMERA_PERSPECTIVE)
	{
		// Setup perspective projection
		double top = RL_CULL_DISTANCE_NEAR * tan(camera.fovy * 0.5 * DEG2RAD);
		double right = top * aspect;

		rlFrustum(-right, right, -top, top, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
	}
	else if (camera.projection == CAMERA_ORTHOGRAPHIC)
	{
		// Setup orthographic projection
		double top = camera.fovy / 2.0;
		double right = top * aspect;

		rlOrtho(-right, right, -top, top, RL_CULL_DISTANCE_NEAR, PB_ORTHOGRAPHIC_CAMERA_CULL_DISTANCE_FAR);
	}

	rlMatrixMode(RL_MODELVIEW);     // Switch back to modelview matrix
	rlLoadIdentity();               // Reset current matrix (modelview)

	// Setup Camera view
	Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
	rlMultMatrixf(MatrixToFloat(matView));      // Multiply modelview matrix by view matrix (camera)

	rlEnableDepthTest();            // Enable DEPTH_TEST for 3D
}

void ShadowMapEnd() {
	rlEnableColorBlend();
	rlDisableTexture();

	rlDrawRenderBatchActive();
	rlDisableFramebuffer();

	rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());

	rlMatrixMode(RL_PROJECTION);
	rlLoadIdentity();

	rlOrtho(0, GetScreenWidth(), GetScreenHeight(), 0, 0, 1);

	rlMatrixMode(RL_MODELVIEW);
	rlLoadIdentity();
}

Matrix CameraPerspective(Camera3D const& camera) {
	if (camera.projection == CAMERA_PERSPECTIVE) {
		double const zfar = RL_CULL_DISTANCE_FAR;
		double const znear = RL_CULL_DISTANCE_NEAR;
		double const top = znear * tan(camera.fovy * 0.5 * DEG2RAD);
		double const right = top * SANDBOX3D_WINDOW_ASPECT;
		double const left = -right;
		double const bottom = -top;
		return MatrixFrustum(left, right, bottom, top, znear, zfar);
	} else {
		float aspect = 1.0f;
		double const zfar = PB_ORTHOGRAPHIC_CAMERA_CULL_DISTANCE_FAR;
		double const znear = RL_CULL_DISTANCE_NEAR;
		double top = camera.fovy / 2.0f;
		double right = top * aspect;
		return MatrixOrtho(-right, right, -top, top, znear, zfar);
	}
}
