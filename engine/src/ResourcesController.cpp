#include <algorithm>
#include <assimp/GltfMaterial.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/resources/ShaderCompiler.hpp>
#include <engine/util/Configuration.hpp>
#include <engine/util/Errors.hpp>
#include <spdlog/spdlog.h>
#include <unordered_set>
#include <utility>

namespace engine::resources {
void ResourcesController::initialize() {
    load_shaders();
    load_models();
    load_textures();
    load_skyboxes();
}

void ResourcesController::terminate() {
    for (auto &[name, resource]: m_models) {
        resource->destroy();
    }
    for (auto &[name, resource]: m_shaders) {
        resource->destroy();
    }
    for (auto &[name, resource]: m_textures) {
        resource->destroy();
    }
    for (auto &[name, resource]: m_sky_boxes) {
        resource->destroy();
    }
}

void ResourcesController::load_shaders() {
    if (!exists(m_shaders_path)) {
        spdlog::info("[ResourcesController]: no {} found to load the shaders from", m_shaders_path.string());
        return;
    }
    for (const auto &shader_path: std::filesystem::directory_iterator(m_shaders_path)) {
        const auto name = shader_path.path().stem().string();
        shader(name, shader_path);
    }
}

void ResourcesController::load_models() {
    if (!exists(m_models_path)) {
        spdlog::info("[ResourcesController]: no {} found to load the models from", m_models_path.string());
        return;
    }
    const auto &config = util::Configuration::config();
    if (!config.contains("resources") || !config["resources"].contains("models")) {
        std::string msg =
                "No configuration for models in the config.json, please provide the resources config. See the example in the README.md";
        throw util::EngineError(util::EngineError::Type::ConfigurationError, msg);
    }
    for (const auto &model_entry: config["resources"]["models"].items()) {
        model(model_entry.key());
    }
}

void ResourcesController::load_textures() {
    if (!exists(m_textures_path)) {
        spdlog::info("[ResourcesController]: no {} found to load the textures from", m_textures_path.string());
        return;
    }
    for (const auto &texture_entry: std::filesystem::directory_iterator(m_textures_path)) {
        texture(texture_entry.path().stem().string(), texture_entry.path());
    }
}

void ResourcesController::load_skyboxes() {
    if (!exists(m_skyboxes_path)) {
        spdlog::info("[ResourcesController]: no {} found to load the skyboxes from", m_skyboxes_path.string());
        return;
    }
    for (const auto &sky_boxes_entry: std::filesystem::directory_iterator(m_skyboxes_path)) {
        skybox(sky_boxes_entry.path().stem().string(), sky_boxes_entry.path());
    }
}

/**
     * @class AssimpSceneProcessor
     * @brief Processes the meshes in an Assimp scene.
     */
class AssimpSceneProcessor {
public:
    /**
         * @brief Processes the meshes in the scene.
         * @returns The meshes in the scene.
         */
    std::vector<Mesh> process_meshes();

    explicit AssimpSceneProcessor(ResourcesController *resources_controller, const aiScene *scene,
                                  std::filesystem::path model_path)
        : m_scene(scene)
        , m_model_path(std::move(model_path))
        , m_resources_controller(resources_controller) {
    }

private:
    void process_node(const aiNode *node);

    void process_mesh(aiMesh *mesh);

    std::vector<Texture *> process_materials(const aiMaterial *material);

    void process_material_type(std::vector<Texture *> &textures, const aiMaterial *material, aiTextureType type);

    static TextureType assimp_texture_type_to_engine(aiTextureType type);

    std::vector<Mesh> m_meshes;
    const aiScene *m_scene;
    std::filesystem::path m_model_path;
    ResourcesController *m_resources_controller;
};

Model *ResourcesController::model(const std::string &name) {
    auto &result = m_models[name];
    if (!result) {
        auto &config = util::Configuration::config();
        if (!config["resources"]["models"].contains(name)) {
            std::string msg = std::format(
                    "No model ({}) specify in config.json. Please add the model to the config.json.", name);
            throw util::EngineError(util::EngineError::Type::ConfigurationError, msg);
        }
        std::filesystem::path model_path = m_models_path / std::filesystem::path(
                                                                   config["resources"]["models"][name]["path"].get<std::string>());
        Assimp::Importer importer;
        int flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace;
        if (config["resources"]["models"][name].value<bool>("flip_uvs", false)) {
            flags |= aiProcess_FlipUVs;
        }

        spdlog::info("load_model(name={}, path={})", name, model_path.string());
        const aiScene *scene = importer.ReadFile(model_path, flags);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::string msg = std::format("Assimp error while reading model: {} from path {}.", model_path.string(),
                                          name);
            throw util::EngineError(util::EngineError::Type::AssetLoadingError, msg);
        }
        AssimpSceneProcessor scene_processor(this, scene, model_path);
        std::vector<Mesh> meshes = scene_processor.process_meshes();
        result = std::make_unique<Model>(Model(std::move(meshes), model_path, name));
    }
    return result.get();
}

Texture *ResourcesController::texture(const std::string &name, const std::filesystem::path &path, TextureType type,
                                      bool flip_uvs) {
    auto &result = m_textures[name];
    if (!result) {
        spdlog::info("load_texture(path={})", path.string());
        const bool srgb = type == TextureType::Diffuse || type == TextureType::Emissive;
        auto texture = graphics::OpenGL::generate_texture(path, flip_uvs, srgb);
        result = std::make_unique<Texture>(Texture(texture, type, path, path.stem()));
    }
    return result.get();
}

Skybox *ResourcesController::skybox(const std::string &name, const std::filesystem::path &path, bool flip_uvs) {
    auto &result = m_sky_boxes[name];
    if (!result) {
        spdlog::info("load_skybox(path={})", path.string());
        auto skybox = graphics::OpenGL::init_skybox_cube();
        auto textures = graphics::OpenGL::load_skybox_textures(path, flip_uvs);
        result = std::make_unique<Skybox>(Skybox(skybox, textures, path, name));
    }
    return result.get();
}

Shader *ResourcesController::shader(const std::string &name, const std::filesystem::path &path) {
    auto &result = m_shaders[name];
    if (!result) {
        spdlog::info("load_shader(path={})", path.string());
        result = std::make_unique<Shader>(ShaderCompiler::compile_from_file(name, path));
    }
    return result.get();
}

std::vector<Mesh> AssimpSceneProcessor::process_meshes() {
    m_meshes.clear();
    process_node(m_scene->mRootNode);
    return std::move(m_meshes);
}

void AssimpSceneProcessor::process_node(const aiNode *node) {
    for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
        auto mesh = m_scene->mMeshes[node->mMeshes[i]];
        process_mesh(mesh);
    }
    for (uint32_t i = 0; i < node->mNumChildren; ++i) {
        process_node(node->mChildren[i]);
    }
}

void AssimpSceneProcessor::process_mesh(aiMesh *mesh) {
    std::vector<Vertex> vertices;
    vertices.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex{};
        vertex.Position.x = mesh->mVertices[i].x;
        vertex.Position.y = mesh->mVertices[i].y;
        vertex.Position.z = mesh->mVertices[i].z;

        if (mesh->HasNormals()) {
            vertex.Normal.x = mesh->mNormals[i].x;
            vertex.Normal.y = mesh->mNormals[i].y;
            vertex.Normal.z = mesh->mNormals[i].z;
        }

        if (mesh->mTextureCoords[0]) {
            vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
            vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;

            vertex.Tangent.x = mesh->mTangents[i].x;
            vertex.Tangent.y = mesh->mTangents[i].y;
            vertex.Tangent.z = mesh->mTangents[i].z;

            vertex.Bitangent.x = mesh->mBitangents[i].x;
            vertex.Bitangent.y = mesh->mBitangents[i].y;
            vertex.Bitangent.z = mesh->mBitangents[i].z;
        }
        vertices.push_back(vertex);
    }

    std::vector<uint32_t> indices;
    for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];

        for (uint32_t j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    auto material = m_scene->mMaterials[mesh->mMaterialIndex];
    std::vector<Texture *> textures = process_materials(material);
    Material mesh_material;
    aiColor4D color;
    if (material->Get(AI_MATKEY_BASE_COLOR, color) != AI_SUCCESS &&
        material->Get(AI_MATKEY_COLOR_DIFFUSE, color) != AI_SUCCESS) {
        color = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
    }
    mesh_material.base_color = {color.r, color.g, color.b, color.a};
    material->Get(AI_MATKEY_METALLIC_FACTOR, mesh_material.metallic);
    material->Get(AI_MATKEY_ROUGHNESS_FACTOR, mesh_material.roughness);
    material->Get(AI_MATKEY_OPACITY, mesh_material.opacity);
    aiString alpha_mode;
    if (material->Get(AI_MATKEY_GLTF_ALPHAMODE, alpha_mode) == AI_SUCCESS) {
        const std::string_view mode = alpha_mode.C_Str();
        if (mode == "BLEND") {
            mesh_material.alpha_mode = AlphaMode::Blend;
        } else if (mode == "MASK") {
            mesh_material.alpha_mode = AlphaMode::Mask;
        }
    }
    if (mesh_material.opacity < 1.0f && mesh_material.alpha_mode == AlphaMode::Opaque) {
        mesh_material.alpha_mode = AlphaMode::Blend;
    }
    material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, mesh_material.alpha_cutoff);
    aiString material_name;
    material->Get(AI_MATKEY_NAME, material_name);
    spdlog::info(
            "material(name={}, base_color=({}, {}, {}, {}), metallic={}, roughness={}, opacity={}, transparent={}, "
            "alpha_cutoff={}, textures={})",
            material_name.C_Str(), mesh_material.base_color.r, mesh_material.base_color.g,
            mesh_material.base_color.b, mesh_material.base_color.a, mesh_material.metallic,
            mesh_material.roughness, mesh_material.opacity,
            mesh_material.alpha_mode != AlphaMode::Opaque, mesh_material.alpha_cutoff, textures.size());
    m_meshes.emplace_back(Mesh(vertices, indices, std::move(textures), mesh_material));
}

std::vector<Texture *> AssimpSceneProcessor::process_materials(const aiMaterial *material) {
    std::vector<Texture *> textures;
    auto ai_texture_types = {
            aiTextureType_DIFFUSE,
            aiTextureType_BASE_COLOR,
            aiTextureType_SPECULAR,
            aiTextureType_SHEEN,
            aiTextureType_NORMALS,
            aiTextureType_NORMAL_CAMERA,
            aiTextureType_HEIGHT,
            aiTextureType_GLTF_METALLIC_ROUGHNESS,
            aiTextureType_METALNESS,
            aiTextureType_DIFFUSE_ROUGHNESS,
            aiTextureType_AMBIENT_OCCLUSION,
            aiTextureType_EMISSIVE,
            aiTextureType_EMISSION_COLOR,
            aiTextureType_OPACITY,
            aiTextureType_TRANSMISSION};

    for (auto ai_texture_type: ai_texture_types) {
        process_material_type(textures, material, ai_texture_type);
    }
    return textures;
}

void AssimpSceneProcessor::process_material_type(std::vector<Texture *> &textures, const aiMaterial *material,
                                                 aiTextureType type) {
    auto material_count = material->GetTextureCount(type);
    aiString material_name_string;
    material->Get(AI_MATKEY_NAME, material_name_string);
    const std::string material_name = material_name_string.C_Str();
    const bool is_fbx = m_model_path.extension() == ".fbx";
    const auto texture_search_root =
            is_fbx ? m_model_path.parent_path().parent_path() : m_model_path.parent_path();
    bool loaded_for_type = false;

    auto load_texture = [&](const std::filesystem::path &texture_path) {
        if (!exists(texture_path)) {
            return false;
        }
        Texture *texture = m_resources_controller->texture(texture_path.string(), texture_path,
                                                           assimp_texture_type_to_engine(type));
        textures.emplace_back(texture);
        loaded_for_type = true;
        return true;
    };

    auto find_fbx_texture = [&](std::string_view suffix) {
        if (!is_fbx || material_name.empty()) {
            return;
        }
        const std::string prefix = material_name + std::string(suffix);
        for (const auto &entry: std::filesystem::recursive_directory_iterator(texture_search_root)) {
            if (entry.is_regular_file() && entry.path().stem().string().starts_with(prefix)) {
                load_texture(entry.path());
                return;
            }
        }
    };

    for (uint32_t i = 0; i < material_count; ++i) {
        aiString ai_texture_path_string;
        material->GetTexture(type, i, &ai_texture_path_string);
        std::string texture_reference = ai_texture_path_string.C_Str();
        std::replace(texture_reference.begin(), texture_reference.end(), '\\', '/');
        std::filesystem::path texture_path = texture_reference;
        if (texture_path.is_relative()) {
            texture_path = m_model_path.parent_path() / texture_path;
        }
        if (!exists(texture_path)) {
            std::string texture_name = texture_path.filename().string();
            for (const auto &entry: std::filesystem::recursive_directory_iterator(texture_search_root)) {
                if (entry.is_regular_file() && entry.path().filename() == texture_name) {
                    texture_path = entry.path();
                    break;
                }
            }
        }
        if (!exists(texture_path)) {
            spdlog::warn("Texture referenced by model {} was not found: {}", m_model_path.string(),
                         texture_reference);
            continue;
        }
        load_texture(texture_path);
    }

    if (!loaded_for_type) {
        switch (type) {
            case aiTextureType_DIFFUSE:
                find_fbx_texture("_Base_Color");
                break;
            case aiTextureType_NORMALS:
                find_fbx_texture("_Normal_OpenGL");
                break;
            case aiTextureType_METALNESS:
                find_fbx_texture("_Metallic");
                break;
            case aiTextureType_DIFFUSE_ROUGHNESS:
                find_fbx_texture("_Roughness");
                break;
            case aiTextureType_AMBIENT_OCCLUSION:
                find_fbx_texture("_Mixed_AO");
                break;
            case aiTextureType_EMISSIVE:
                find_fbx_texture("_Emissive");
                break;
            case aiTextureType_OPACITY:
                find_fbx_texture("_Opacity");
                break;
            case aiTextureType_SPECULAR:
                find_fbx_texture("_Specular_level");
                break;
            case aiTextureType_TRANSMISSION:
                find_fbx_texture("_Scattering");
                break;
            default: break;
        }
    }
}

TextureType AssimpSceneProcessor::assimp_texture_type_to_engine(aiTextureType type) {
    switch (type) {
        case aiTextureType_DIFFUSE: return TextureType::Diffuse;
        case aiTextureType_BASE_COLOR: return TextureType::Diffuse;
        case aiTextureType_SPECULAR: return TextureType::SpecularLevel;
        case aiTextureType_SHEEN: return TextureType::SpecularLevel;
        case aiTextureType_HEIGHT: return TextureType::Height;
        case aiTextureType_NORMALS: return TextureType::Normal;
        case aiTextureType_NORMAL_CAMERA: return TextureType::Normal;
        case aiTextureType_GLTF_METALLIC_ROUGHNESS: return TextureType::MetallicRoughness;
        case aiTextureType_METALNESS: return TextureType::Metallic;
        case aiTextureType_DIFFUSE_ROUGHNESS: return TextureType::Roughness;
        case aiTextureType_AMBIENT_OCCLUSION: return TextureType::Occlusion;
        case aiTextureType_EMISSIVE: return TextureType::Emissive;
        case aiTextureType_EMISSION_COLOR: return TextureType::Emissive;
        case aiTextureType_OPACITY: return TextureType::Opacity;
        case aiTextureType_TRANSMISSION: return TextureType::Scattering;
        default: RG_SHOULD_NOT_REACH_HERE("Engine currently doesn't support the aiTextureType: {}",
                                          static_cast<int>(type));
    }
}
}// namespace engine::resources
