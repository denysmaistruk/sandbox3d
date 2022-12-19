#include "lights.h"

#include "rlgl.h"
#include "raymath.h"

#include "../extern/raylib/src/external/glad.h"

#define SHADER_PATH "../src/utils/shaders/"
#define SHADOW_PATH SHADER_PATH "lights/"

#define PB_ORTHOGRAPHIC_CAMERA_CULL_DISTANCE_FAR 100

ShadowMap LoadShadowMap(int width, int height) {
	ShadowMap shadowMap	= {};
	shadowMap.id		= rlLoadFramebuffer(width, height);
	shadowMap.width		= width;
	shadowMap.height	= height;

	rlEnableFramebuffer(shadowMap.id);

	shadowMap.depth.id = rlLoadTextureDepth(width, height, false);
	shadowMap.depth.width = width;
	shadowMap.depth.height = height;
	shadowMap.depth.format = PIXELFORMAT_COMPRESSED_PVRT_RGBA;
	shadowMap.depth.mipmaps = 1;

	rlTextureParameters(shadowMap.depth.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_CLAMP);
	rlTextureParameters(shadowMap.depth.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_CLAMP);
	rlFramebufferAttach(shadowMap.id, shadowMap.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

	rlDisableFramebuffer();

	return shadowMap;
}

void UnloadShadowMap(ShadowMap shadowMap) {
	rlUnloadFramebuffer(shadowMap.id);
	rlUnloadTexture(shadowMap.depth.id);
}

void ShadowMapBegin(ShadowMap shadowMap) {
	rlDrawRenderBatchActive();
	rlEnableFramebuffer(shadowMap.id);

	rlClearScreenBuffers();

	// Set viewport to framebuffer size
	rlViewport(0, 0, shadowMap.width, shadowMap.height);

	rlMatrixMode(RL_PROJECTION);        // Switch to projection matrix
	rlLoadIdentity();                   // Reset current matrix (projection)

	// Set orthographic projection to current framebuffer size
	// NOTE: Configured top-left corner as (0, 0)
	rlOrtho(0, shadowMap.width, shadowMap.height, 0, 0, 1);

	rlMatrixMode(RL_MODELVIEW);         // Switch back to modelview matrix
	rlLoadIdentity();                   // Reset current matrix (modelview)

	rlClearColor(255, 255, 255, 255);
	rlDisableColorBlend();

	glCullFace(GL_FRONT);				// Front faces culling for solving shadow acne problem
}

void EndShadowCaster() { 
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

static int lightsCount = 0;

Light CreateLight(Shader shader, int type, Vector3 position, Vector3 target, Color color, float cutoff /*= 0.7f*/, float lightRadius /*= 10.f*/, float spotSoftness /*= 0.65f*/)
{
	Light light = { 0 };

	if (lightsCount < MAX_LIGHTS)
	{
		light.enabled = true;
		light.type = type;
		light.position = position;
		light.target = target;
		light.color = color;
		light.cutoff = cutoff;
		light.lightRadius = lightRadius;
		light.spotSoftness = spotSoftness;

		char enabledName[32] = "lights[x].enabled\0";
		char typeName[32] = "lights[x].type\0";
		char posName[32] = "lights[x].position\0";
		char targetName[32] = "lights[x].target\0";
		char colorName[32] = "lights[x].color\0";
		char cutoffName[32] = "lights[x].cutoff\0";
		char lightRadiusName[32] = "lights[x].lightRadius\0";
		char spotSoftnessName[32] = "lights[x].spotSoftness\0";

		// Set location name [x] depending on lights count
		enabledName[7] = '0' + lightsCount;
		typeName[7] = '0' + lightsCount;
		posName[7] = '0' + lightsCount;
		targetName[7] = '0' + lightsCount;
		colorName[7] = '0' + lightsCount;
		cutoffName[7] = '0' + lightsCount;
		lightRadiusName[7] = '0' + lightsCount;
		spotSoftnessName[7] = '0' + lightsCount;

		light.enabledLoc = GetShaderLocation(shader, enabledName);
		light.typeLoc = GetShaderLocation(shader, typeName);
		light.posLoc = GetShaderLocation(shader, posName);
		light.targetLoc = GetShaderLocation(shader, targetName);
		light.colorLoc = GetShaderLocation(shader, colorName);
		light.cutoffLoc = GetShaderLocation(shader, cutoffName);
		light.lightRadiusLoc = GetShaderLocation(shader, lightRadiusName);
		light.spotSoftnessLoc = GetShaderLocation(shader, spotSoftnessName);

		UpdateLightValues(shader, light);
		++lightsCount;
	}
	return light;
}

void UpdateLightValues(Shader shader, Light light)
{
    // Send to shader light enabled state and type
    SetShaderValue(shader, light.enabledLoc, &light.enabled, SHADER_UNIFORM_INT);
    SetShaderValue(shader, light.typeLoc, &light.type, SHADER_UNIFORM_INT);

    // Send to shader light position values
    float position[3] = { light.position.x, light.position.y, light.position.z };
    SetShaderValue(shader, light.posLoc, position, SHADER_UNIFORM_VEC3);

    // Send to shader light target position values
    float target[3] = { light.target.x, light.target.y, light.target.z };
    SetShaderValue(shader, light.targetLoc, target, SHADER_UNIFORM_VEC3);

    // Send to shader light color values
    float color[4] = { (float)light.color.r / (float)255, (float)light.color.g / (float)255,
                       (float)light.color.b / (float)255, (float)light.color.a / (float)255 };
	SetShaderValue(shader, light.colorLoc, color, SHADER_UNIFORM_VEC4);
	
	SetShaderValue(shader, light.cutoffLoc, &light.cutoff, SHADER_UNIFORM_FLOAT);

	SetShaderValue(shader, light.lightRadiusLoc, &light.lightRadius, SHADER_UNIFORM_FLOAT);

	SetShaderValue(shader, light.spotSoftnessLoc, &light.spotSoftness, SHADER_UNIFORM_FLOAT);
}

Shader  LoadDepthPreviewShader() { 

	auto shader = LoadShader(0, SHADOW_PATH"depth.fs");
	shader.locs[SHADER_LOC_CASTER_PERSPECTIVE] = GetShaderLocation(shader, "casterPerspective");
	return shader;
}
Shader	LoadShadowShader			() { return LoadShader(SHADOW_PATH"shadow.vs", SHADOW_PATH"shadow.fs"); }
Shader	LoadShadedGeometryShader	() {
	auto shader = LoadShader(SHADOW_PATH"geom.vs", SHADOW_PATH"geom.fs");
	shader.locs[SHADER_LOC_VECTOR_VIEW]	  = GetShaderLocation(shader, "viewPos");
	shader.locs[SHADER_LOC_MAP_SHADOW]	  = GetShaderLocation(shader, "shadowMap");
	shader.locs[SHADER_LOC_SHADOW_FACTOR] = GetShaderLocation(shader, "shadowFactor");
	shader.locs[SHADER_LOC_MAT_LIGHT]	  = GetShaderLocation(shader, "matLight");
	shader.locs[SHADER_LOC_AMBIENT]		  = GetShaderLocation(shader, "ambient");

	return shader;
}

Matrix CameraFrustum(Camera3D const& camera) {
	double const zfar	= RL_CULL_DISTANCE_FAR;
	double const znear	= RL_CULL_DISTANCE_NEAR;
	double const top	= znear * tan(camera.fovy * 0.5 * DEG2RAD);
	double const right	= top;
	double const left	= -right;
	double const bottom	= -top;

	return MatrixFrustum(left, right, bottom, top, znear, zfar);
}

Matrix CameraOrtho(Camera3D const& camera) {
	float aspect		= 1.0f;
    double const zfar	= PB_ORTHOGRAPHIC_CAMERA_CULL_DISTANCE_FAR;
    double const znear	= RL_CULL_DISTANCE_NEAR;
	double top			= camera.fovy / 2.0f;
	double right		= top * aspect;

	return MatrixOrtho(-right, right, -top, top, znear, zfar);
}

static Shader shaderCached;
void BeginInstacing(Shader shader)
{
	int enable = 1;
	SetShaderValue(shader, shader.locs[SHADER_LOC_INSTANCING], &enable, SHADER_UNIFORM_INT);
    shaderCached = shader;
}

void EndInstancing()
{
    int enable = 0;
    SetShaderValue(shaderCached, shaderCached.locs[SHADER_LOC_INSTANCING], &enable, SHADER_UNIFORM_INT);
}