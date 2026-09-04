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
// This shader performs downsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.
// This particular method was customly designed to eliminate
// "pulsating artifacts and temporal stability issues".

// Remember to add bilinear minification filter for this texture!
// Remember to use a floating-point texture format (for HDR)!
// Remember to use edge clamping for this texture!
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D screenTexture;
uniform int mipLevel;
vec3 PowVec3(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

const float invGamma = 1.0 / 2.2;
vec3 ToSRGB(vec3 v) { return PowVec3(v, invGamma); }

float RGBToLuminance(vec3 col)
{
    return dot(col, vec3(0.2126f, 0.7152f, 0.0722f));
}

float KarisAverage(vec3 col)
{
    // Formula is 1 / (1 + luma)
    float luma = RGBToLuminance(ToSRGB(col));
    return 1.0f / (1.0f + luma);
}
void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(screenTexture, 0));
    float x = texelSize.x;
    float y = texelSize.y;
    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(screenTexture, vec2(TexCoords.x - 2 * x, TexCoords.y + 2 * y)).rgb;
    vec3 b = texture(screenTexture, vec2(TexCoords.x, TexCoords.y + 2 * y)).rgb;
    vec3 c = texture(screenTexture, vec2(TexCoords.x + 2 * x, TexCoords.y + 2 * y)).rgb;

    vec3 d = texture(screenTexture, vec2(TexCoords.x - 2 * x, TexCoords.y)).rgb;
    vec3 e = texture(screenTexture, vec2(TexCoords.x, TexCoords.y)).rgb;
    vec3 f = texture(screenTexture, vec2(TexCoords.x + 2 * x, TexCoords.y)).rgb;

    vec3 g = texture(screenTexture, vec2(TexCoords.x - 2 * x, TexCoords.y - 2 * y)).rgb;
    vec3 h = texture(screenTexture, vec2(TexCoords.x, TexCoords.y - 2 * y)).rgb;
    vec3 i = texture(screenTexture, vec2(TexCoords.x + 2 * x, TexCoords.y - 2 * y)).rgb;

    vec3 j = texture(screenTexture, vec2(TexCoords.x - x, TexCoords.y + y)).rgb;
    vec3 k = texture(screenTexture, vec2(TexCoords.x + x, TexCoords.y + y)).rgb;
    vec3 l = texture(screenTexture, vec2(TexCoords.x - x, TexCoords.y - y)).rgb;
    vec3 m = texture(screenTexture, vec2(TexCoords.x + x, TexCoords.y - y)).rgb;
    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1
    // Check if we need to perform Karis average on each block of 4 samples
    vec3 downsample = vec3(0.0);
    vec3 groups[5];
    float kw0, kw1, kw2, kw3, kw4;
    switch (mipLevel)
    {
        case 0:
    // We are writing to mip 0, so we need to apply Karis average to each block
    // of 4 samples to prevent fireflies (very bright subpixels, leads to pulsating
    // artifacts).
            groups[0] = (a + b + d + e) / 4.0f;
            groups[1] = (b + c + e + f) / 4.0f;
            groups[2] = (d + e + g + h) / 4.0f;
            groups[3] = (e + f + h + i) / 4.0f;
            groups[4] = (j + k + l + m) / 4.0f;
            kw0 = KarisAverage(groups[0]);
            kw1 = KarisAverage(groups[1]);
            kw2 = KarisAverage(groups[2]);
            kw3 = KarisAverage(groups[3]);
            kw4 = KarisAverage(groups[4]);
            downsample = (kw0 * groups[0] + kw1 * groups[1] + kw2 * groups[2] + kw3 * groups[3] + kw4 * groups[4])
            / (kw0 + kw1 + kw2 + kw3 + kw4);
            break;
        default:
            downsample = e * 0.125;
            downsample += (a + c + g + i) * 0.03125;
            downsample += (b + d + f + h) * 0.0625;
            downsample += (j + k + l + m) * 0.125;
            break;
    }
    FragColor = vec4(downsample, 1.0);
}
