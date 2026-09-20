#include <ShadowController.hpp>

#include <SettingsController.hpp>
#include <engine/core/Engine.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace engine::black_lodge::app {
namespace {
const float light_distance = 100.0f;
const int shadow_resolution = 2048;
const int point_resolution = 1024;
const float shadow_size = 100.0f;

glm::vec3 normalized_direction(const glm::vec3 &direction) {
    const auto length = glm::length(direction);
    return length > 0.0f ? direction / length : glm::vec3(0.0f, -1.0f, 0.0f);
}
}// namespace

void ShadowController::initialize() {
    const auto settings = Controller::get<SettingsController>();
    m_point_shadow_far_plane = settings->m_far;

    m_directional_shadow_map = std::make_unique<engine::graphics::ShadowMap>(
            engine::graphics::ShadowMapDescription{
                    .type = engine::graphics::ShadowMapType::Basic,
                    .resolution = shadow_resolution,
                    .left = -shadow_size,
                    .right = shadow_size,
                    .bottom = -shadow_size,
                    .top = shadow_size,
                    .near_plane = 1.0f,
                    .far_plane = settings->m_far,
                    .light_position = -normalized_direction(settings->u_dlight_dir) * light_distance,
                    .target = glm::vec3(0.0f)});

    for (std::size_t index = 0; index < m_point_shadow_maps.size(); ++index) {
        m_point_shadow_maps[index] = std::make_unique<engine::graphics::ShadowMap>(
                engine::graphics::ShadowMapDescription{
                        .type = engine::graphics::ShadowMapType::Point,
                        .resolution = point_resolution,
                        .near_plane = 0.1f,
                        .far_plane = settings->m_far,
                        .light_position = index == 0 ? settings->u_plight_pos1 : settings->u_plight_pos2});
    }
}

void ShadowController::begin_draw() {
    const auto settings = Controller::get<SettingsController>();
    const auto far_plane = glm::max(settings->m_far, 0.2f);
    m_point_shadow_far_plane = far_plane;

    m_directional_shadow_map->set_clip_planes(1.0f, far_plane);
    m_directional_shadow_map->set_light(-normalized_direction(settings->u_dlight_dir) * light_distance,
                                        glm::vec3(0.0f));
    for (std::size_t index = 0; index < m_point_shadow_maps.size(); ++index) {
        const auto position = index == 0 ? settings->u_plight_pos1 : settings->u_plight_pos2;
        m_point_shadow_maps[index]->set_clip_planes(0.1f, far_plane);
        m_point_shadow_maps[index]->set_light(position, position + glm::vec3(1.0f, 0.0f, 0.0f));
    }

    render_directional_shadow();
    for (std::size_t index = 0; index < m_point_shadow_maps.size(); ++index) {
        render_point_shadow(index);
    }
}

void ShadowController::render_directional_shadow() {
    const auto resources = Controller::get<engine::resources::ResourcesController>();
    const auto shader = resources->shader("shadow");
    const auto floor = resources->model("black_lodge");

    m_directional_shadow_map->begin_render();
    shader->use();
    shader->set_mat4("uLightSpaceMatrix", m_directional_shadow_map->light_view_projection());
    shader->set_mat4("uModel", glm::mat4(1.0f));
    floor->draw(shader);
    engine::graphics::ShadowMap::end_render();
}

void ShadowController::render_point_shadow(std::size_t index) {
    const auto resources = Controller::get<engine::resources::ResourcesController>();
    const auto shader = resources->shader("point_shadow");
    const auto floor = resources->model("black_lodge");
    const auto settings = Controller::get<SettingsController>();
    const auto position = index == 0 ? settings->u_plight_pos1 : settings->u_plight_pos2;
    const auto &shadow_map = *m_point_shadow_maps[index];

    shadow_map.begin_render();
    shader->use();
    shader->set_vec3("uLightPos", position);
    shader->set_float("uFarPlane", m_point_shadow_far_plane);
    for (std::size_t face = 0; face < 6; ++face) {
        shader->set_mat4("uShadowMatrices[" + std::to_string(face) + "]",
                         shadow_map.point_view_projection_matrices()[face]);
    }
    shader->set_mat4("uModel", glm::mat4(1.0f));
    floor->draw(shader);
    engine::graphics::ShadowMap::end_render();
}

void ShadowController::terminate() {
    for (auto &shadow_map: m_point_shadow_maps) {
        shadow_map.reset();
    }
    m_directional_shadow_map.reset();
}
}// namespace engine::black_lodge::app
