#version 430



// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;
uniform float shadowFactor;

#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1
#define     LIGHT_SPOT              2
#define SANDBOX3D_MAX_LIGHTS 8
#define SANDBOX3D_SHADOW_MAP_RESOLUTION 4096
#define SANDBOX3D_SHADOW_MAP_WIDTH  SANDBOX3D_SHADOW_MAP_RESOLUTION
#define SANDBOX3D_SHADOW_MAP_HEIGTH SANDBOX3D_SHADOW_MAP_RESOLUTION
#define SANDBOX3D_SHADOW_MAP_CELL_SIZE 1024
#define SANDBOX3D_SHADOW_MAP_CELL_COUNT ((SANDBOX3D_SHADOW_MAP_RESOLUTION * 2) / SANDBOX3D_SHADOW_MAP_CELL_SIZE)
#define SANDBOX3D_SHADOW_MAP_ROW_SIZE (SANDBOX3D_SHADOW_MAP_RESOLUTION / SANDBOX3D_SHADOW_MAP_CELL_SIZE)

struct Light 
{
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
    float cutoff;
    float lightRadius;
    float spotSoftness;
};

uniform vec4 ambient;
layout(std430, binding=0) buffer lightsBlock {
    Light lights[];
};


uniform sampler2D shadowMapAtlas; // Shadowmap
layout(std430, binding=1) buffer shadowMatBlock {
    mat4 shadowMat[];
};

// Output fragment color
out vec4 finalColor;

// Fog 
const vec4 fogColor = vec4(0.8, 0.8, 0.8, 0.5);
const float fogDensity = 0.005;
const vec4 shadowPos = vec4(0.0);


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
    vec4 seed4 = vec4(seed, i);
    float dotSeed4 = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
    return fract(sin(dotSeed4) * 43758.5453);
}

float ShadowCalc(vec4 p, float bias, vec2 atlasOffset)
{
    vec2 texelSize = 1.0 / textureSize(shadowMapAtlas, 0);
    vec3 projCoords3D = p.xyz / p.w;
    projCoords3D = projCoords3D * 0.5 + 0.5;
    float depth = projCoords3D.z;
    vec2 projCoords = projCoords3D.xy / SANDBOX3D_SHADOW_MAP_ROW_SIZE + atlasOffset * texelSize;
    float texDepth = texture(shadowMapAtlas, projCoords.xy).r;

    float shadow = 0.0;
    if (usePoisondDisk) 
    {
        for (int x = -1; x <= 1; ++x) 
        {
            for (int y = -1; y <= 1; ++y) 
            {
                for (int i = 0; i < 4; ++i) 
                {
                    int index = int(16.0 * random(fragPosition, i)) % 16;
                    float pcfDepth = texture(shadowMapAtlas, projCoords.xy + vec2(x, y) * texelSize + poissonDisk[index] / 5000.0).r; 
                    shadow += depth - bias < pcfDepth ? 0.0 : 0.25;        
                }
            }    
        }
    }
    else 
    {
        for (int x = -1; x <= 1; ++x) 
        {
            for (int y = -1; y <= 1; ++y) 
            {
                float pcfDepth = texture(shadowMapAtlas, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += depth - bias < pcfDepth ? 0.0 : 1.0;        
            }    
        }
    }
    
    return shadow / 9.0;
}

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    
    float spot = 1.0;
    float attenuation = 1.0;
    vec3 lightDot = vec3(0.0);
    vec3 specular = vec3(0.0);
    
    for (int i = 0; i < SANDBOX3D_MAX_LIGHTS; ++i)
    {
        vec3 light = vec3(0.0);

        if (lights[i].type == LIGHT_DIRECTIONAL)
        {
            light = -normalize(lights[i].target - lights[i].position);
        }

        if (lights[i].type == LIGHT_POINT)
        {
            light = normalize(lights[i].position - fragPosition);
        }

        if (lights[i].type == LIGHT_SPOT)
        {
            vec3 lightRaw = (lights[i].position - fragPosition);
            float lightDistSqr = dot(lightRaw, lightRaw);
            light = normalize(lightRaw);

            attenuation = clamp(1.0 - lightDistSqr / (lights[i].lightRadius * lights[i].lightRadius), 0, 1);
            attenuation *= attenuation;

            float theta = dot(-light, normalize(lights[i].target - lights[i].position));
            float epsilon = lights[i].cutoff - lights[i].spotSoftness;
            spot = clamp((theta - lights[i].cutoff) / epsilon, 0.0, 1.0);
        }

        float NdotL = max(dot(normal, light), 0.1);
        lightDot += lights[i].color.rgb * NdotL * spot * attenuation;

        float specCo = 0.0;
        if (lights[i].type != LIGHT_SPOT && NdotL > 0.0)
        {
            specCo = pow(max(0.0, dot(viewD, reflect(-(light), normal))), 16.0); // 16 refers to shine
        }
        specular += specCo;
    }

    // Note: alternative bias calculations
    // vec3 lightSum = normalize(lightDot - fragPosition);
    // float NdotLSum = max(dot(normal, lightSum), 0.0);
    // float bias = max(0.0005 * (1.0 - NdotLSum), 0.00005);
    // float bias = 0.00005f * tan(acos(NdotLSum));
    
    float shadow = 0.0;
    const float bias = -0.0005;
    for (int i = 0; i < SANDBOX3D_MAX_LIGHTS; ++i)
    {
        int x = (i / SANDBOX3D_SHADOW_MAP_ROW_SIZE);
        int y = (i % SANDBOX3D_SHADOW_MAP_ROW_SIZE);
        vec2 offset = vec2(x,y) * SANDBOX3D_SHADOW_MAP_CELL_SIZE;
        vec4 shadowPos = shadowMat[i] * vec4(fragPosition, 1.0);
        shadow = ShadowCalc(shadowPos, bias, offset);
    }
    finalColor = vec4(vec3(shadow), 1);
    return;
    // HACK: currently shadow casts by one source of directional light, should be changed to account any source
    // Turn off shadows for fragments facing from the light source 
    // for (int i = 0; i < SANDBOX3D_MAX_LIGHTS; i++)
    // {    
    //     if (lights[i].type == LIGHT_DIRECTIONAL)
    //     {
    //         vec3 light = -normalize(lights[i].target - lights[i].position);
    //         if (dot(light, fragNormal) < 0.0)
    //             shadow = 0.0; 
    //     }
    //     continue;
    // }

    finalColor = (texelColor * ((colDiffuse * (1.0 - shadow * shadowFactor) 
        + vec4(specular * (1.0 - shadow * shadowFactor * 0.5), 1.0)) * vec4(lightDot, 1.0)));
    finalColor += texelColor * (ambient / 10.0) * colDiffuse;

    // Gamma correction
    finalColor = pow(finalColor, vec4(1.0 / 2.2));

    // Fog calculation
    float dist = length(viewPos - fragPosition);

    // Exponential fog
    float fogFactor = 1.0 / exp((dist * fogDensity) * (dist * fogDensity));

    fogFactor = clamp(fogFactor, 0.0, 1.0);

    finalColor = mix(fogColor, finalColor, fogFactor);
}