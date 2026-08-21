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

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in vec3 TangentLightPos[NUM_LIGHTS - 1];
in vec3 TangentViewPos;
in vec3 TangentFragPos;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_metallic_roughness1;
uniform sampler2D texture_emissive1;

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
uniform DirLight uDirLight;
uniform PointLight uPointLights[NUM_LIGHTS - 1];

const float PI = 3.14159265359;
uniform float uAmbientStrength = 0.1f;
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
vec3 CalcPointLight(PointLight light, vec3 tangentLightPos, vec3 fragPosTangent, vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness, vec3 F0) {
    vec3 lightDir = normalize(tangentLightPos - fragPosTangent);
    vec3 halfVec = normalize(viewDir + lightDir);

    float distance = length(tangentLightPos - fragPosTangent);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 radiance = light.color * light.intensity * attenuation;

    float NDF = DistributionGGX(normal, halfVec, roughness);
    float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    vec3 F = fresnelSchlick(max(dot(halfVec, viewDir), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    float NdotL = max(dot(normal, lightDir), 0.0);

    return (kD * albedo / PI + specular) * radiance * NdotL;
}
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness, vec3 F0) {
    vec3 lightDir = normalize(-light.direction);
    vec3 halfVec = normalize(viewDir + lightDir);
    vec3 radiance = light.color * light.intensity;

    float NDF = DistributionGGX(normal, halfVec, roughness);
    float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    vec3 F = fresnelSchlick(max(dot(halfVec, viewDir), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    float NdotL = max(dot(normal, lightDir), 0.0);

    return (kD * albedo / PI + specular) * radiance * NdotL;
}
void main() {
    //Albedos are done in sRGB color space
    vec3 albedo = pow(texture(texture_diffuse1, TexCoords).rgb, vec3(2.2));
    vec3 normal = Normal;
    normal = texture(texture_normal1, TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    float metallic = texture(texture_metallic_roughness1, TexCoords).b;
    float roughness = texture(texture_metallic_roughness1, TexCoords).g;

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 Lo = vec3(0.0);
    vec3 ambient = uAmbientStrength * albedo;
    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
    Lo += CalcDirLight(uDirLight, normal, viewDir, albedo, metallic, roughness, F0);
    for (int i = 0; i < NUM_LIGHTS - 1; i++) {
        Lo += CalcPointLight(uPointLights[i], TangentLightPos[i], TangentFragPos, normal, viewDir, albedo, metallic, roughness, F0);
    }
    vec3 emission = vec3(0.0);
    //emission = texture(texture_emissive1, TexCoords).rgb;
    vec3 color = Lo + ambient + emission;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));


    FragColor = vec4(color, 1.0);
}