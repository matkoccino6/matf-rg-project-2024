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
constexpr float light_distance = 100.0f;
constexpr int shadow_resolution = 4096;
constexpr int point_resolution = 2048;
constexpr int spot_resolution = 4096;
constexpr float shadow_size = 100.0f;

glm::vec3 normalized_direction(const glm::vec3 &direction) {
    const auto length = glm::length(direction);
    return length > 0.0f ? direction / length : glm::vec3(0.0f, -1.0f, 0.0f);
}

glm::vec3 shadow_up(const glm::vec3 &direction) {
    const auto normalized = normalized_direction(direction);
    return std::abs(glm::dot(normalized, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.95f
                   ? glm::vec3(1.0f, 0.0f, 0.0f)
                   : glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::mat4 lodge_model(const float scale) {
    auto model = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
    return glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 statue_model(const float scale) {
    auto model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.5f, 0.0f, -6.2f));
    return glm::scale(model, glm::vec3(scale / 100.0f));
}

float scaled_scene_extent(const SettingsController *settings) {
    return shadow_size * settings->m_scale;
}

float directional_shadow_size(const SettingsController *settings) {
    return glm::max(scaled_scene_extent(settings), 1.0f);
}

float directional_far_plane(const SettingsController *settings) {
    return glm::max(settings->far, light_distance + scaled_scene_extent(settings));
}

float point_far_plane(const SettingsController *settings) {
    const auto scene_extent = scaled_scene_extent(settings);
    const auto position1_distance = glm::length(settings->u_plight_pos1);
    const auto position2_distance = glm::length(settings->u_plight_pos2);
    const auto point_light_distance = glm::max(position1_distance, position2_distance);
    return glm::max(settings->far, point_light_distance + scene_extent);
}
}// namespace

void ShadowController::initialize() {
    const auto settings = get<SettingsController>();
    const auto directional_size = directional_shadow_size(settings);
    m_point_shadow_far_plane = point_far_plane(settings);
    m_spot_shadow_far_plane = settings->far;

    m_directional_shadow_map = std::make_unique<graphics::ShadowMap>(
            engine::graphics::ShadowMapDescription{
                    .type = engine::graphics::ShadowMapType::Basic,
                    .resolution = shadow_resolution,
                    .left = -directional_size,
                    .right = directional_size,
                    .bottom = -directional_size,
                    .top = directional_size,
                    .near_plane = 1.0f,
                    .far_plane = directional_far_plane(settings),
                    .light_position = -normalized_direction(settings->u_dlight_dir) * light_distance,
                    .target = glm::vec3(0.0f)});

    for (std::size_t index = 0; index < m_point_shadow_maps.size(); ++index) {
        m_point_shadow_maps[index] = std::make_unique<graphics::ShadowMap>(
                graphics::ShadowMapDescription{
                        .type = graphics::ShadowMapType::Point,
                        .resolution = point_resolution,
                        .near_plane = 0.1f,
                        .far_plane = m_point_shadow_far_plane,
                        .light_position = index == 0 ? settings->u_plight_pos1 : settings->u_plight_pos2});
    }

    m_spot_shadow_map = std::make_unique<graphics::ShadowMap>(
            graphics::ShadowMapDescription{
                    .type = graphics::ShadowMapType::Spot,
                    .resolution = spot_resolution,
                    .near_plane = 0.1f,
                    .far_plane = m_spot_shadow_far_plane,
                    .field_of_view = settings->u_slight_outer_cut_off * 2.0f,
                    .light_position = settings->u_slight_pos,
                    .target = settings->u_slight_pos + normalized_direction(settings->u_slight_dir),
                    .up = shadow_up(settings->u_slight_dir)});
}

void ShadowController::begin_draw() {
    const auto settings = Controller::get<SettingsController>();
    const auto directional_size = directional_shadow_size(settings);
    const auto point_far = glm::max(point_far_plane(settings), 0.2f);
    const auto spot_far = glm::max(settings->far, 0.2f);
    const auto direction_far = glm::max(directional_far_plane(settings), 0.2f);
    m_point_shadow_far_plane = point_far;
    m_spot_shadow_far_plane = spot_far;

    m_directional_shadow_map->set_clip_planes(1.0f, direction_far);
    m_directional_shadow_map->set_orthographic_bounds(-directional_size, directional_size,
                                                      -directional_size, directional_size);
    m_directional_shadow_map->set_light(-normalized_direction(settings->u_dlight_dir) * light_distance,
                                        glm::vec3(0.0f));
    for (std::size_t index = 0; index < m_point_shadow_maps.size(); ++index) {
        const auto position = index == 0 ? settings->u_plight_pos1 : settings->u_plight_pos2;
        m_point_shadow_maps[index]->set_clip_planes(0.1f, point_far);
        m_point_shadow_maps[index]->set_light(position, position + glm::vec3(1.0f, 0.0f, 0.0f));
    }
    m_spot_shadow_map->set_clip_planes(0.1f, spot_far);
    m_spot_shadow_map->set_field_of_view(settings->u_slight_outer_cut_off * 2.0f);
    m_spot_shadow_map->set_light(settings->u_slight_pos,
                                 settings->u_slight_pos + normalized_direction(settings->u_slight_dir),
                                 shadow_up(settings->u_slight_dir));

    render_directional_shadow();
    for (std::size_t index = 0; index < m_point_shadow_maps.size(); ++index) {
        render_point_shadow(index);
    }
    render_spot_shadow();
}

void ShadowController::render_directional_shadow() const {
    const auto resources = get<resources::ResourcesController>();
    const auto shader = resources->shader("shadow");
    const auto floor = resources->model("black_lodge");
    const auto statue = resources->model("statue");
    const auto settings = get<SettingsController>();

    m_directional_shadow_map->begin_render();
    shader->use();
    shader->set_mat4("uLightSpaceMatrix", m_directional_shadow_map->light_view_projection());
    shader->set_mat4("uModel", lodge_model(settings->m_scale));
    floor->draw(shader);
    shader->set_mat4("uModel", statue_model(settings->m_scale));
    statue->draw(shader);
    graphics::ShadowMap::end_render();
}

void ShadowController::render_point_shadow(const std::size_t index) const {
    const auto resources = get<resources::ResourcesController>();
    const auto shader = resources->shader("point_shadow");
    const auto floor = resources->model("black_lodge");
    const auto statue = resources->model("statue");
    const auto settings = get<SettingsController>();
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
    shader->set_mat4("uModel", lodge_model(settings->m_scale));
    floor->draw(shader);
    shader->set_mat4("uModel", statue_model(settings->m_scale));
    statue->draw(shader);
    graphics::ShadowMap::end_render();
}

void ShadowController::render_spot_shadow() const {
    const auto resources = get<resources::ResourcesController>();
    const auto shader = resources->shader("spot_shadow");
    const auto floor = resources->model("black_lodge");
    const auto statue = resources->model("statue");
    const auto settings = get<SettingsController>();

    m_spot_shadow_map->begin_render();
    shader->use();
    shader->set_vec3("uLightPos", settings->u_slight_pos);
    shader->set_float("uFarPlane", m_spot_shadow_far_plane);
    shader->set_mat4("uLightSpaceMatrix", m_spot_shadow_map->light_view_projection());
    shader->set_mat4("uModel", lodge_model(settings->m_scale));
    floor->draw(shader);
    shader->set_mat4("uModel", statue_model(settings->m_scale));
    statue->draw(shader);
    graphics::ShadowMap::end_render();
}

void ShadowController::terminate() {
    for (auto &shadow_map: m_point_shadow_maps) {
        shadow_map.reset();
    }
    m_directional_shadow_map.reset();
    m_spot_shadow_map.reset();
}
}// namespace engine::black_lodge::app
