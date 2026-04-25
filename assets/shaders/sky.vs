#version 330

// Input vertex attributes
in vec3 vertexPosition;

// Model-View-Projection matrix
uniform mat4 mvp;

// Output the local 3D position to the Fragment Shader instead of UVs
out vec3 fragLocalPos;

void main()
{
    // Pass the raw local position of the sphere's vertex
    fragLocalPos = vertexPosition;

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}