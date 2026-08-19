//#shader vertex
#version 330
#define NUM_LIGHTS 3
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
out vec3 TangentLightPos[NUM_LIGHTS - 1];
out vec3 TangentViewPos;
out vec3 TangentFragPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform vec3 uLightPos[NUM_LIGHTS - 1];
uniform vec3 uViewPos;
void main() {
    FragPos = vec3(uModel * vec4(aPos, 1.0));
    TexCoords = aTexCoords;
    //translacija Normali u world space normal matrix
    Normal = mat3(transpose(inverse(uModel))) * aNormal;

    vec3 T = normalize(mat3(uModel) * aTangent);
    vec3 N = normalize(mat3(uModel) * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    //transpose function instead of the inverse function here.
    //A great property of orthogonal matrices (each axis is a perpendicular unit vector) is that the transpose of an orthogonal matrix equals its inverse.
    //This is a great property as inverse is expensive and a transpose isn't.
    mat3 TBN = transpose(mat3(T, B, N));
    for (int i = 0; i < NUM_LIGHTS - 1; i++) {
        TangentLightPos[i] = TBN * uLightPos[i];
    }
    TangentViewPos = TBN * uViewPos;
    TangentFragPos = TBN * FragPos;
    gl_Position = uProjection * uView * vec4(FragPos, 1.0);
}

//#shader fragment
#version 330
#define NUM_LIGHTS 3
out vec4 FragColor;
struct DirLight {
    vec3 direction;
    vec3 color;
    float intensity;
};
struct PointLight {
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in vec3 TangentLightPos[NUM_LIGHTS - 1];
in vec3 TangentViewPos;
in vec3 TangentFragPos;

uniform sampler2D uDiffuseTexture;
uniform sampler2D uSpecularTexture;
uniform sampler2D uNormalTexture;
uniform sampler2D uEmissiveTexture;

uniform DirLight uDirLight;
uniform PointLight uPointLights[NUM_LIGHTS - 1];
//uniform vec3 uLightPos;
//uniform vec3 uViewPos;
//uniform vec3 uLightColor;
//uniform float uShininess = 32.0f;
uniform float uAmbientStrength = 0.1f;
vec3 CalcPointLight(PointLight light, vec3 tangentLightPos, vec3 normal, vec3 fragPosTangent, vec3 viewDir, vec3 diffuseColor) {
    vec3 lightDir = normalize(tangentLightPos - fragPosTangent);
    float distance = length(tangentLightPos - fragPosTangent);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * light.intensity * diff * diffuseColor * attenuation;
    return diffuse;
}
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffuseColor) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * light.intensity * diff * diffuseColor;
    return diffuse;
}
void main() {
    vec3 normal = Normal;
    normal = texture(uNormalTexture, TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    vec3 diffuseColor = texture(uDiffuseTexture, TexCoords).rgb;
    //float specularStrength = texture(uSpecularTexture, TexCoords).r;

    vec3 ambient = uAmbientStrength * diffuseColor;

    //vec3 lightDir = normalize(uLightPos - FragPos);
/*vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = uLightColor * (diff * diffuseColor);*/

    //vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
    //vec3 reflectDir = reflect(-lightDir, normal);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), uShininess);
    //vec3 specular = uLightColor * (spec * specularStrength);
    vec3 result = ambient;
    result += CalcDirLight(uDirLight, normal, viewDir, diffuseColor);
    for (int i = 0; i < NUM_LIGHTS - 1; i++) {
        result += CalcPointLight(uPointLights[i], TangentLightPos[i], normal, TangentFragPos, viewDir, diffuseColor);
    }
    vec3 emission = vec3(0.0);
/*if (textureSize(uEmissiveTexture, 0).x > 0) {
        emission = texture(uEmissiveTexture, TexCoords).rgb;
    }*/
    FragColor = vec4(result, 1.0);
}