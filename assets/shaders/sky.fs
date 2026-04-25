#version 330

// Input 3D position from Vertex Shader
in vec3 fragLocalPos;

// Uniforms from C++ (Sky colors)
uniform vec3 zenithColor;
uniform vec3 horizonColor;

out vec4 finalColor;

void main()
{
    // 1. Convert the local position into a normalized direction vector.
    // This gives us a vector where x, y, and z are strictly between -1.0 and 1.0.
    vec3 dir = normalize(fragLocalPos);

    // 2. dir.y represents the vertical direction:
    // 0.0 is exactly the horizon.
    // 1.0 is exactly straight up (zenith).
    // -1.0 is exactly straight down (nadir).

    // Take the absolute value so both looking up and looking down are positive.
    // float upFactor = abs(dir.y);
    float upFactor = max(dir.y, 0.0);

    // 3. Apply a mathematical curve to soften the transition.
    // A smaller power (like 0.5 or 0.6) pulls the horizon color higher up into the sky.
    // A larger power (like 2.0) keeps the horizon color tightly at the bottom.
    float blendFactor = pow(upFactor, 0.6);

    // Clamp for safety
    blendFactor = clamp(blendFactor, 0.0, 1.0);

    // 4. Mix the colors based on our perfect mathematical direction
    vec3 skyColor = mix(horizonColor, zenithColor, blendFactor);

    finalColor = vec4(skyColor, 1.0);
}