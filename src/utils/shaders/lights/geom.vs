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
uniform mat4 matLight;
uniform int instancing;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoord;
out vec4 shadowPos;
out vec4 fragColor;

void main()
{
    if (instancing == 1) 
    {
        fragPosition = vec3(matModel * instanceTransform * vec4(vertexPosition, 1.0));
    }
    else
    {
        fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    }

    
    fragNormal = normalize(vec3(matNormal *vec4(vertexNormal, 0.0)));
    shadowPos = matLight * vec4(fragPosition, 1.0);
	fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    
    // Calculate final vertex position
    if (instancing == 1)
    {
        gl_Position = mvp * instanceTransform * vec4(vertexPosition, 1.0);
    }
    else
    {
        gl_Position = mvp * vec4(vertexPosition, 1.0);    
    }

// Compute MVP for current instance
//    mat4 mvpi = mvp * instanceTransform;
//
//     Send vertex attributes to fragment shader
//    fragPosition = vec3(mvpi*vec4(vertexPosition, 1.0));
//    fragTexCoord = vertexTexCoord;
//    fragColor = vertexColor;
//    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));
//
//     Calculate final vertex position
//    gl_Position = mvpi*vec4(vertexPosition, 1.0);
}