// clang-format off
#include <glad/glad.h>
// clang-format on
#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/Mesh.hpp>
#include <engine/resources/Shader.hpp>
#include <engine/util/Errors.hpp>
#include <engine/util/Utils.hpp>
#include <unordered_map>

namespace engine::resources {
    Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices,
               std::vector<Texture *> textures, Material material)
    : m_textures(std::move(textures))
  , m_material(material) {
        // NOLINTBEGIN
        static_assert(std::is_trivial_v<Vertex>);
        uint32_t VAO, VBO, EBO;
        CHECKED_GL_CALL(glGenVertexArrays, 1, &VAO);
        CHECKED_GL_CALL(glGenBuffers, 1, &VBO);
        CHECKED_GL_CALL(glGenBuffers, 1, &EBO);

        CHECKED_GL_CALL(glBindVertexArray, VAO);
        CHECKED_GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, VBO);
        CHECKED_GL_CALL(glBufferData, GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(),
                        GL_STATIC_DRAW);

        CHECKED_GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, EBO);
        CHECKED_GL_CALL(glBufferData, GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(),
                        GL_STATIC_DRAW);

        CHECKED_GL_CALL(glEnableVertexAttribArray, 0);
        CHECKED_GL_CALL(glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *) offsetof(Vertex, Position));

        CHECKED_GL_CALL(glEnableVertexAttribArray, 1);
        CHECKED_GL_CALL(glVertexAttribPointer, 1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *) offsetof(Vertex, Normal));

        CHECKED_GL_CALL(glEnableVertexAttribArray, 2);
        CHECKED_GL_CALL(glVertexAttribPointer, 2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *) offsetof(Vertex, TexCoords));

        CHECKED_GL_CALL(glEnableVertexAttribArray, 3);
        CHECKED_GL_CALL(glVertexAttribPointer, 3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *) offsetof(Vertex, Tangent));

        CHECKED_GL_CALL(glEnableVertexAttribArray, 4);
        CHECKED_GL_CALL(glVertexAttribPointer, 4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *) offsetof(Vertex, Bitangent));

        CHECKED_GL_CALL(glBindVertexArray, 0);
        // NOLINTEND
        m_vao         = VAO;
        m_num_indices = indices.size();
    }

    void Mesh::draw(const Shader *shader) {
        shader->set_vec4("uBaseColor", m_material.base_color);
        shader->set_float("uMetallicFactor", m_material.metallic);
        shader->set_float("uRoughnessFactor", m_material.roughness);
        shader->set_float("uOpacity", m_material.opacity);
        shader->set_int("uAlphaMode", static_cast<int>(m_material.alpha_mode));
        shader->set_float("uAlphaCutoff", m_material.alpha_cutoff);

        std::unordered_map<std::string_view, uint32_t> counts;
        const auto texture_unit_base = [](TextureType type) {
            switch (type) {
            case TextureType::Diffuse: return 0;
            case TextureType::Specular: return 4;
            case TextureType::SpecularLevel: return 5;
            case TextureType::Normal: return 8;
            case TextureType::Height: return 12;
            case TextureType::MetallicRoughness: return 16;
            case TextureType::Metallic: return 20;
            case TextureType::Roughness: return 21;
            case TextureType::Scattering: return 22;
            case TextureType::Occlusion: return 24;
            case TextureType::Emissive: return 28;
            case TextureType::Opacity: return 30;
            case TextureType::Regular: return 31;
            }
            RG_SHOULD_NOT_REACH_HERE("Unhandled texture type");
        };
        //Unbind previously bound textures if there are any
        for (const auto type: {
                 TextureType::Diffuse, TextureType::Specular, TextureType::SpecularLevel, TextureType::Normal,
                 TextureType::Height, TextureType::MetallicRoughness, TextureType::Occlusion,
                 TextureType::Metallic, TextureType::Roughness, TextureType::Emissive,
                 TextureType::Opacity, TextureType::Scattering, TextureType::Regular
             }) {
            const auto unit = texture_unit_base(type);
            CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0 + unit);
            CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, 0);
            shader->set_int(std::string(Texture::uniform_name_convention(type)) + "1", unit);
            shader->set_bool("has_" + std::string(Texture::uniform_name_convention(type)) + "1", false);
        }

        std::string uniform_name;
        uniform_name.reserve(32);
        for (const auto *texture: m_textures) {
            const auto &texture_type = Texture::uniform_name_convention(texture->type());
            const auto unit          = texture_unit_base(texture->type()) + counts[texture_type]++;
            CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0 + unit);
            uniform_name.append(texture_type);
            uniform_name.append(std::to_string(counts[texture_type]));
            shader->set_int(uniform_name, unit);
            shader->set_bool("has_" + uniform_name, true);
            CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, texture->id());
            uniform_name.clear();
        }
        if (m_material.alpha_mode == AlphaMode::Blend) {
            CHECKED_GL_CALL(glEnable, GL_BLEND);
            CHECKED_GL_CALL(glBlendFunc, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        CHECKED_GL_CALL(glBindVertexArray, m_vao);
        CHECKED_GL_CALL(glDrawElements, GL_TRIANGLES, m_num_indices, GL_UNSIGNED_INT, (void *) 0);
        CHECKED_GL_CALL(glBindVertexArray, 0);
        if (m_material.alpha_mode == AlphaMode::Blend) {
            CHECKED_GL_CALL(glDisable, GL_BLEND);
        }
    }

    void Mesh::destroy() {
        CHECKED_GL_CALL(glDeleteVertexArrays, 1, &m_vao);
    }
} // namespace engine::resources
