#version 430

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;

#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_SPOT              1
#define     LIGHT_POINT             2

struct LightData {
    uint        type;
    uint        shadowId;
    uint        color;
    float       cutoff;
    float       radius;
    float       softness;
    vec3        position;
    vec3        target;
};

#define lightDataCount          lightingInfo.r
#define shadowMapAtlasRowLen    lightingInfo.g
#define shadowMapAtlasCellRes   lightingInfo.b

uniform ivec3       lightingInfo;
uniform sampler2D   shadowMapAtlas; 
layout(std430, binding=0)
readonly buffer lightDataBlock {
    LightData lights[];
};

// Output fragment color
out vec4 finalColor;

// Fog 
const vec4  fogColor    = vec4(0.8, 0.8, 0.8, 0.5);
const vec4  ambient     = vec4(0.4, 0.4, 0.4, 1.0) / 10.0;
const float shadowBias  = -0.00003;
const float fogDensity  = 0.005;

// Shadows
const bool usePoisondDisk = true;
const vec2 poissonDisk[16] = vec2[] (
    vec2(-0.94201624, -0.39906216),  vec2(0.94558609, -0.76890725),
    vec2(-0.094184101, -0.92938870), vec2(0.34495938, 0.29387760),
    vec2(-0.91588581, 0.45771432),   vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543, 0.27676845),   vec2(0.97484398, 0.75648379),
    vec2(0.44323325, -0.97511554),   vec2(0.53742981, -0.47373420),
    vec2(-0.26496911, -0.41893023),  vec2(0.79197514, 0.19090188),
    vec2(-0.24188840, 0.99706507),   vec2(-0.81409955, 0.91437590),
    vec2(0.19984126, 0.78641367),    vec2(0.14383161, -0.14100790)
);

float random(vec3 seed, int i)
{
    vec4    seed4   = vec4(seed, i);
    float   dotSeed4= dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
    return  fract(sin(dotSeed4) * 43758.5453);
}

// Get camera look-at matrix (view matrix)
mat4 MatrixLookAt(vec3 eye, vec3 target, vec3 up)
{
    vec3 vz = normalize(eye - target);
    vec3 vx = normalize(cross(up, vz));
    vec3 vy = cross(vz, vx);
    return mat4(
        vec4(vx.x, vy.x, vz.x, 0.0),
        vec4(vx.y, vy.y, vz.y, 0.0),
        vec4(vx.z, vy.z, vz.z, 0.0),
        vec4(-dot(vx, eye), -dot(vy, eye), -dot(vz, eye), 1.0)
    );
}

mat4 MatrixFrustum(float left, float right, float bottom, float top, float near, float far)
{
    float rl = right - left;
    float tb = top - bottom;
    float fn = far - near;
    return mat4(
        vec4((near*2.0)/rl, 0.0, 0.0, 0.0),
        vec4(0.0, (near*2.0)/tb, 0.0, 0.0),
        vec4((right + left)/rl, (top + bottom)/tb, -(far + near)/fn, -1.0),
        vec4(0.0, 0.0, -(far*near*2.0)/fn, 0.0)
    );
}

// Get orthographic projection matrix
mat4 MatrixOrtho(float left, float right, float bottom, float top, float near, float far)
{
    float rl = right - left;
    float tb = top - bottom;
    float fn = far - near;
    return mat4(
        vec4(2.0/rl, 0.0, 0.0, 0.0),
        vec4(0.0, 2.0/tb, 0.0, 0.0),
        vec4(0.0, 0.0, -2.0/fn, 0.0),
        vec4(-(left + right)/rl, -(top + bottom)/tb, -(far + near)/fn, 1.0)
    );
}

#define RL_CULL_DISTANCE_FAR 1000.0
#define RL_CULL_DISTANCE_NEAR 0.01
#define PB_ORTHOGRAPHIC_CAMERA_CULL_DISTANCE_FAR 100.0
#define CASTER_FOV 90.0

mat4 CasterPerspective(uint type) {
    float aspect = 1.0;
	if (type == LIGHT_DIRECTIONAL) {
		float zfar  = PB_ORTHOGRAPHIC_CAMERA_CULL_DISTANCE_FAR;
		float znear = RL_CULL_DISTANCE_NEAR;
		float top   = CASTER_FOV / 2.0;
		float right = top * aspect;
		return MatrixOrtho(-right, right, -top, top, znear, zfar);
	} else {
        float zfar  = RL_CULL_DISTANCE_FAR;
		float znear = RL_CULL_DISTANCE_NEAR;
		float top   = znear * tan(radians(CASTER_FOV * 0.5));
		float right = top * aspect;
		float left  = -right;
		float bottom= -top;
		return MatrixFrustum(left, right, bottom, top, znear, zfar);
    }
}

mat4 RebuildShadowCasterMatrix(in LightData light) {
    mat4 proj   = CasterPerspective(light.type);
    mat4 view   = MatrixLookAt(light.position, light.target, vec3(0.0,1.0,0.0));
    return (proj * view);
}

float ShadowCalc(vec4 p, float bias, vec2 atlasOffset)
{
    vec2    texelSize   = 1.0 / textureSize(shadowMapAtlas, 0);
    vec3    projCoords3D= p.xyz / p.w;
    projCoords3D        = projCoords3D * 0.5 + 0.5;
    float   depth       = projCoords3D.z;
    vec2    projCoords  = projCoords3D.xy / shadowMapAtlasRowLen + atlasOffset * texelSize;
    float   texDepth    = texture(shadowMapAtlas, projCoords.xy).r;
    float   shadow      = 0.0;
    if (usePoisondDisk) {
        for (int x = -1 ; x <= 1; ++x)
        for (int y = -1 ; y <= 1; ++y)
        for (int i = 0  ; i < 4 ; ++i) {
            int     index       = int(16.0 * random(fragPosition, i)) % 16;
            vec2    pointOffset = vec2(x, y) * texelSize;
            vec2    poisson     = poissonDisk[index] / 5000.0;
            vec2    pointUV     = projCoords.xy + pointOffset + poisson; 
            float   pcfDepth    = texture(shadowMapAtlas, pointUV).r; 
            shadow  += float(depth - bias > pcfDepth) * 0.25;
        }
    } else {
        for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y) {
            vec2    pointOffset = vec2(x, y) * texelSize;
            vec2    pointUV     = projCoords.xy + pointOffset; 
            float   pcfDepth    = texture(shadowMapAtlas, pointUV).r; 
            shadow  += float(depth - bias > pcfDepth);
        }
    }
    
    return shadow / 9.0;
}

void main()
{
    // Texel color fetching from texture sampler
    vec3 normal     = normalize(fragNormal);
    vec3 viewD      = normalize(viewPos - fragPosition);
    vec4 texelColor = texture(texture0, fragTexCoord);

    for (int i = 0; i < lightDataCount; ++i)
    {
        float   spot        = 1.0;
        float   attenuation = 1.0;
        float   specular    = 0.0;
        vec3    lightDir    = normalize(lights[i].position - lights[i].target);
        vec3    lightRaw    = lights[i].position - fragPosition;
        float   radiusSqr   = lights[i].radius * lights[i].radius;
        float   distSqr     = dot(lightRaw, lightRaw);
        vec3    light       = lights[i].type == LIGHT_DIRECTIONAL
                            ? lightDir : normalize(lightRaw);
        float   NdotL       = max(dot(normal, light), 0.0);

        if (lights[i].type != LIGHT_DIRECTIONAL)
            attenuation = pow(clamp(1.0 - distSqr / radiusSqr, 0, 1), 2);

        if (lights[i].type != LIGHT_SPOT && NdotL > 0.0)
            specular = pow(max(0.0, dot(viewD, reflect(-(light), normal))), 16.0); // 16 refers to shine

        if (lights[i].type == LIGHT_SPOT)
        {
            float theta     = dot(-light, -lightDir);
            float epsilon   = lights[i].cutoff - lights[i].softness;
            spot = clamp((theta - lights[i].cutoff) / epsilon, 0.0, 1.0);
        }

        float shadow = float(NdotL == 0.0);
        if (NdotL > 0.0 && lights[i].shadowId != -1) {
            uint x = (lights[i].shadowId / shadowMapAtlasRowLen);
            uint y = (lights[i].shadowId % shadowMapAtlasRowLen);
            vec2 offset     = vec2(x,y) * shadowMapAtlasCellRes;
            mat4 lightMat   = RebuildShadowCasterMatrix(lights[i]);
            vec4 shadowPos  = lightMat * vec4(fragPosition, 1.0);
            shadow = ShadowCalc(shadowPos, shadowBias, offset);
        }

        specular *= (1.0 - shadow * 0.5);
        vec4 specularMasked = vec4(vec3(specular), 1.0);
        vec4 diffuseMasked  = colDiffuse * (1.0 - shadow);
        vec3 lightDot       = unpackUnorm4x8(lights[i].color).rgb * NdotL * spot * attenuation;
        vec4 colorMasked    = (diffuseMasked + specularMasked) * vec4(lightDot, 1.0);

        finalColor += colorMasked;
    }

    finalColor  *= texelColor;
    finalColor  += texelColor * (ambient / 10.0) * colDiffuse;
    finalColor  = pow(finalColor, vec4(1.0 / 2.2));                     // Gamma correction
    float dist  = length(viewPos - fragPosition);                       // Fog calculation
    float factor= 1.0 / exp((dist * fogDensity) * (dist * fogDensity)); // Exponential fog
    factor      = clamp(factor, 0.0, 1.0);
    finalColor  = mix(fogColor, finalColor, factor);
}
