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
uniform bool horizontal;

const float weights[5] = float[](
    0.227027,
    0.1945946,
    0.1216216,
    0.054054,
    0.016216
);

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(screenTexture, 0));
    vec3 result = texture(screenTexture, TexCoords).rgb * weights[0];

    for (int i = 1; i < 5; ++i) {
        vec2 offset = horizontal
            ? vec2(texelSize.x * i, 0.0)
            : vec2(0.0, texelSize.y * i);

        result += texture(screenTexture, TexCoords + offset).rgb * weights[i];
        result += texture(screenTexture, TexCoords - offset).rgb * weights[i];
    }

    FragColor = vec4(result, 1.0);
}
