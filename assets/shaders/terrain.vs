#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;

uniform mat4 mvp;
uniform sampler2D heightMap;
uniform float heightScale;

out vec2 fragTexCoord;

void main()
{
    fragTexCoord = vertexTexCoord;

    vec3 color = texture(heightMap, vertexTexCoord).rgb;
    vec3 c = color * 255.0;
    float heightValue = -10000.0 + ((c.r * 65536.0 + c.g * 256.0 + c.b) * 0.1);

    vec3 displacedPosition = vertexPosition;
    displacedPosition.y += heightValue * heightScale;

    // add skirt to the edges of the terrain to hide gaps between tiles
    float epsilon = 0.001;

    bool isEdge = (vertexTexCoord.x < epsilon) ||
                  (vertexTexCoord.x > 1.0 - epsilon) ||
                  (vertexTexCoord.y < epsilon) ||
                  (vertexTexCoord.y > 1.0 - epsilon);

    if (isEdge)
    {
        displacedPosition.y -= 150.0;
    }

    gl_Position = mvp * vec4(displacedPosition, 1.0);
}