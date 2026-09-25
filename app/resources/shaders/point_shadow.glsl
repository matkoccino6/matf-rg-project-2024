//#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 uModel;

out vec4 WorldPosition;

void main() {
    WorldPosition = uModel * vec4(aPos, 1.0);
    gl_Position = WorldPosition;
}

//#shader geometry
#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 uShadowMatrices[6];

in vec4 WorldPosition[];
out vec4 FragPosition;

void main() {
    for (int face = 0; face < 6; ++face) {
        gl_Layer = face;
        for (int vertex = 0; vertex < 3; ++vertex) {
            FragPosition = WorldPosition[vertex];
            gl_Position = uShadowMatrices[face] * FragPosition;
            EmitVertex();
        }
        EndPrimitive();
    }
}

//#shader fragment
#version 330 core
in vec4 FragPosition;

uniform vec3 uLightPos;
uniform float uFarPlane;

void main() {
    float distance = length(FragPosition.xyz - uLightPos);
    gl_FragDepth = distance / uFarPlane;
}
