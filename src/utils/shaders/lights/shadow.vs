#version 330

// Input vertex attributes
in vec3 vertexPosition;

// Input uniform values
uniform mat4 mvp;

uniform mat4 matModel;
uniform mat4 matProjection;
uniform mat4 dummyProjection;

void main()
{
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}