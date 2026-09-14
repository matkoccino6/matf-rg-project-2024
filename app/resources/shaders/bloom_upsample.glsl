//#shader vertex
#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;
out vec2 TexCoords;

void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 0.0, 1.0);
}

//#shader fragment
#version 330 core

in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D screenTexture;
uniform float filterRadius;
void main() {
    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float x = filterRadius;
    float y = filterRadius;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 b = texture(screenTexture, vec2(TexCoords.x, TexCoords.y + y)).rgb;
    vec3 c = texture(screenTexture, vec2(TexCoords.x + x, TexCoords.y + y)).rgb;
    vec3 a = texture(screenTexture, vec2(TexCoords.x - x, TexCoords.y + y)).rgb;

    vec3 d = texture(screenTexture, vec2(TexCoords.x - x, TexCoords.y)).rgb;
    vec3 e = texture(screenTexture, vec2(TexCoords.x, TexCoords.y)).rgb;
    vec3 f = texture(screenTexture, vec2(TexCoords.x + x, TexCoords.y)).rgb;

    vec3 g = texture(screenTexture, vec2(TexCoords.x - x, TexCoords.y - y)).rgb;
    vec3 h = texture(screenTexture, vec2(TexCoords.x, TexCoords.y - y)).rgb;
    vec3 i = texture(screenTexture, vec2(TexCoords.x + x, TexCoords.y - y)).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    vec3 upsample = vec3(0.0);
    upsample = e * 4.0;
    upsample += (b + d + f + h) * 2.0;
    upsample += (a + c + g + i);
    upsample *= 1.0 / 16.0;
    FragColor = vec4(upsample, 1.0);
    //FragColor = texture(screenTexture, TexCoords);
}
