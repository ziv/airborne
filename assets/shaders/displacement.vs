#version 330

// Attributes that raylib provides automatically
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Values passed to the Fragment Shader (for coloring)
out vec2 fragTexCoord;
out vec4 fragColor;

// Uniforms we need to provide
uniform mat4 mvp;               // Model-View-Projection matrix (raylib sets this automatically)
uniform sampler2D heightmap;    // Our heightmap texture
uniform float heightScale;      // Maximum mountain height multiplier

void main()
{
    // Pass coordinates forward
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    // 1. Read the height from the texture via the red channel (r),
    // since in a grayscale heightmap all RGB channels are identical.
    float height = texture(heightmap, vertexTexCoord).r;

    // 2. Displace the vertex along the Y axis
    vec3 displacedPos = vertexPosition;
    displacedPos.y += height * heightScale;

    // 3. Compute the final position on screen
    gl_Position = mvp * vec4(displacedPos, 1.0);
}