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

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;
uniform float exposure = 1.0;

void main() {
    const float gamma = 2.2;
    vec3 hdrColor = texture(sceneTexture, TexCoords).rgb;
    hdrColor += texture(bloomTexture, TexCoords).rgb;
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    mapped = pow(mapped, vec3(1.0 / gamma));
    FragColor = vec4(mapped, 1.0);
    //float gray = dot(color, vec3(0.299, 0.587, 0.114));
    //FragColor = vec4(vec3(gray), 1.0);
}
