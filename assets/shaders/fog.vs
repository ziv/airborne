#version 330

// from raylib's default vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

// transform from model space to clip space
uniform mat4 mvp;
uniform mat4 matModel;

// output to the fragment shader
out vec2 fragTexCoord;
out vec4 fragColor;
out float fragDistance; // distance from the camera to the vertex, used for fog calculation

void main()
{
    // final position of the vertex on the screen
    gl_Position = mvp * vec4(vertexPosition, 1.0);

    // texture and color
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    // absolute world position of the vertex
    vec4 worldPosition = matModel * vec4(vertexPosition, 1.0);


    // distance = sqrt(x^2 + z^2)
    fragDistance = length(worldPosition.xz);
}