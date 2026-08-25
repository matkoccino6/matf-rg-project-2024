// clang-format off
#include <glad/glad.h>
// clang-format on
#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/Texture.hpp>
#include <engine/util/Errors.hpp>

namespace engine::resources {
std::string_view texture_type_to_string(TextureType type) {
    switch (type) {
        case TextureType::Regular: return "Regular";
        case TextureType::Diffuse: return "Diffuse";
        case TextureType::Specular: return "Specular";
        case TextureType::SpecularLevel: return "SpecularLevel";
        case TextureType::Normal: return "Normal";
        case TextureType::Height: return "Height";
        case TextureType::Emissive: return "Emissive";
        case TextureType::MetallicRoughness: return "MetallicRoughness";
        case TextureType::Metallic: return "Metallic";
        case TextureType::Roughness: return "Roughness";
        case TextureType::Occlusion: return "Occlusion";
        case TextureType::Opacity: return "Opacity";
        case TextureType::Scattering: return "Scattering";
        default: RG_SHOULD_NOT_REACH_HERE("Unknown TextureType");
    }
}

void Texture::destroy() {
    CHECKED_GL_CALL(glDeleteTextures, 1, &m_id);
}

void Texture::bind(int32_t sampler) {
    RG_GUARANTEE(sampler >= GL_TEXTURE0 && sampler <= GL_TEXTURE31, "sampler out of range");
    CHECKED_GL_CALL(glActiveTexture, sampler);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, m_id);
}

std::string_view Texture::uniform_name_convention(TextureType type) {
    switch (type) {
        case TextureType::Regular: return "texture_regular";
        case TextureType::Diffuse: return "texture_diffuse";
        case TextureType::Specular: return "texture_specular";
        case TextureType::SpecularLevel: return "texture_specular_level";
        case TextureType::Normal: return "texture_normal";
        case TextureType::Height: return "texture_height";
        case TextureType::MetallicRoughness: return "texture_metallic_roughness";
        case TextureType::Metallic: return "texture_metallic";
        case TextureType::Roughness: return "texture_roughness";
        case TextureType::Occlusion: return "texture_occlusion";
        case TextureType::Emissive: return "texture_emissive";
        case TextureType::Opacity: return "texture_opacity";
        case TextureType::Scattering: return "texture_scattering";
        default: RG_SHOULD_NOT_REACH_HERE("Unhandled TextureType");
    }
}
}// namespace engine::resources
