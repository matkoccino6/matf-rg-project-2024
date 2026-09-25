//#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 uModel;
uniform mat4 uLightSpaceMatrix;

out vec3 WorldPosition;

void main() {
    WorldPosition = vec3(uModel * vec4(aPos, 1.0));
    gl_Position = uLightSpaceMatrix * vec4(WorldPosition, 1.0);
}

//#shader fragment
#version 330 core
in vec3 WorldPosition;

uniform vec3 uLightPos;
uniform float uFarPlane;

void main() {
    gl_FragDepth = length(WorldPosition - uLightPos) / uFarPlane;
}
