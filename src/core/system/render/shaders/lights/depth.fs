#version 330

in vec2 fragTexCoord;

// Output fragment color
out vec4 finalColor;

uniform sampler2D texture0;
uniform int casterPerspective;

const float nearClip = 0.01;
const float farClip = 1000.0;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * nearClip * farClip) / (farClip + nearClip - z * (farClip - nearClip));
}

void main()
{
	vec2 uv = fragTexCoord;
	uv.y = 1.0 - uv.y;

    if (bool(casterPerspective)) 
    {
        float depth = LinearizeDepth(texture(texture0, uv).r);
        finalColor.rgb = vec3(depth) / farClip;
    }

    finalColor.rgb = vec3(texture(texture0, uv).r);

    finalColor.a = 1.0;
}