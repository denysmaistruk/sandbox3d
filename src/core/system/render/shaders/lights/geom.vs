#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;
in mat4 instanceTransform;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;
uniform int instancing;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoord;
out vec4 fragColor;

void main()
{
    mat4 mModel = matModel;
    if(instancing == 1) {
        mModel = instanceTransform;
    }
    
    fragPosition = vec3(mModel * vec4(vertexPosition, 1.0));
    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 0.0)));
	fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

     // Calculate final vertex position
    mat4 mvpi = mvp;
    if (instancing == 1) {
        mvpi = mvp * instanceTransform;
    }
    
    gl_Position = mvpi * vec4(vertexPosition, 1.0);    
}