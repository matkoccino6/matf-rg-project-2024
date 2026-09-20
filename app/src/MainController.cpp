#include <GUIController.hpp>
#include <MainController.hpp>
#include <SettingsController.hpp>
#include <ShadowController.hpp>
#include <engine/core/Engine.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <memory>
#include <spdlog/spdlog.h>

namespace engine::black_lodge::app {
void MainController::initialize() {
    spdlog::info("MainController::initialize");
    engine::graphics::OpenGL::enable_depth_testing();
    auto observer = std::make_unique<MainPlatformEventObserver>();
    engine::core::Controller::get<engine::platform::PlatformController>()->register_platform_event_observer(
            std::move(observer));
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto settings = Controller::get<SettingsController>();
    graphics->perspective_params().Far = settings->m_far;
    auto camera = get<engine::graphics::GraphicsController>()->camera();
    camera->Position = settings->m_camera_position;
}

bool MainController::loop() {
    const auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KeyId::KEY_ESCAPE).state() == engine::platform::Key::State::JustPressed) {
        return false;
    }
    return true;
}

void MainController::poll_events() {
    spdlog::debug("MainController::poll_events");
    const auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KEY_F1).state() == engine::platform::Key::State::JustPressed) {
        m_cursor_enabled = !m_cursor_enabled;
        platform->set_enable_cursor(m_cursor_enabled);
    }
}

void MainController::update() {
    spdlog::debug("MainController::update");
    update_camera();
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    graphics->perspective_params().Far = Controller::get<SettingsController>()->m_far;
}

void MainController::begin_draw() {
    spdlog::debug("MainController::begin_draw");
    engine::graphics::OpenGL::clear_buffers();
}

void MainController::draw() {
    spdlog::debug("MainController::draw");
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto shader = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("lighting");
    auto floor = engine::core::Controller::get<engine::resources::ResourcesController>()->model("black_lodge");
    auto settings = Controller::get<SettingsController>();
    const auto model = glm::scale(glm::mat4(1.0f), glm::vec3(m_scale));

    engine::graphics::OpenGL::set_viewport(
            graphics->perspective_params().Width, graphics->perspective_params().Height);
    shader->use();
    shader->set_mat4("uProjection", graphics->projection_matrix());
    shader->set_mat4("uView", graphics->camera()->view_matrix());
    shader->set_mat4("uModel", model);

    shader->set_vec3("uViewPos", graphics->camera()->Position);
    shader->set_float("uAmbientStrength", settings->u_ambient_strength);

    shader->set_vec3("uDirLight.color", settings->u_dlight_color);
    shader->set_vec3("uDirLight.direction", settings->u_dlight_dir);
    shader->set_float("uDirLight.intensity", settings->u_dlight_intensity);

    shader->set_vec3("uPointLights[0].color", settings->u_plight_color1);
    shader->set_vec3("uPointLights[0].position", settings->u_plight_pos1);
    shader->set_float("uPointLights[0].intensity", settings->u_plight_intensity);
    shader->set_float("uPointLights[0].constant", 1.0f);
    shader->set_float("uPointLights[0].linear", 0.07f);
    shader->set_float("uPointLights[0].quadratic", 0.017f);

    shader->set_vec3("uPointLights[1].color", settings->u_plight_color2);
    shader->set_vec3("uPointLights[1].position", settings->u_plight_pos2);
    shader->set_float("uPointLights[1].intensity", settings->u_plight_intensity);
    shader->set_float("uPointLights[1].constant", 1.0f);
    shader->set_float("uPointLights[1].linear", 0.07f);
    shader->set_float("uPointLights[1].quadratic", 0.017f);

    shader->set_vec3("uSpotLight.position", settings->u_slight_pos);
    shader->set_vec3("uSpotLight.color", settings->u_slight_color);
    shader->set_vec3("uSpotLight.direction", settings->u_slight_dir);
    shader->set_float("uSpotLight.intensity", settings->u_slight_intensity);
    shader->set_float("uSpotLight.constant", 1.0f);
    shader->set_float("uSpotLight.linear", 0.027f);
    shader->set_float("uSpotLight.quadratic", 0.0028f);
    shader->set_float("uSpotLight.cutOff", glm::cos(glm::radians(settings->u_slight_cut_off)));
    shader->set_float("uSpotLight.outerCutOff", glm::cos(glm::radians(settings->u_slight_outer_cut_off)));

    shader->set_float("rFactor", settings->u_r_factor);
    shader->set_float("uEmissiveFactor", settings->u_emissive_factor);
    const auto shadows = Controller::get<ShadowController>();
    shader->set_mat4("uLightSpaceMatrix", shadows->directional_shadow_map().light_view_projection());
    shader->set_vec3("uDirectionalShadowLightDir", settings->u_dlight_dir);
    shader->set_int("uShadowMap", ShadowController::directional_texture_unit());
    shader->set_bool("uUseShadowMap", true);
    shader->set_int("uPointShadowMaps[0]", ShadowController::point_texture_unit(0));
    shader->set_int("uPointShadowMaps[1]", ShadowController::point_texture_unit(1));
    shader->set_vec3("uPointShadowLightPos1", settings->u_plight_pos1);
    shader->set_vec3("uPointShadowLightPos2", settings->u_plight_pos2);
    shader->set_float("uPointShadowFarPlane", shadows->point_shadow_far_plane());
    shader->set_bool("uUsePointShadowMap", true);
    shadows->directional_shadow_map().bind_texture(ShadowController::directional_texture_unit());
    shadows->point_shadow_map(0).bind_texture(ShadowController::point_texture_unit(0));
    shadows->point_shadow_map(1).bind_texture(ShadowController::point_texture_unit(1));

    floor->draw(shader);
}

void MainController::end_draw() {
    spdlog::debug("MainController::end_draw");
    engine::core::Controller::get<engine::platform::PlatformController>()->swap_buffers();
}

void MainController::update_camera() {
    auto gui = engine::core::Controller::get<GUIController>();
    if (gui->is_enabled()) {
        return;
    }
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto camera = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();
    float dt = platform->dt();
    if (platform->key(platform::KEY_W).state() == platform::Key::State::Pressed) {
        camera->move_camera(graphics::Camera::Movement::FORWARD, dt);
    }
    if (platform->key(platform::KEY_S).state() == platform::Key::State::Pressed) {
        camera->move_camera(graphics::Camera::Movement::BACKWARD, dt);
    }
    if (platform->key(platform::KEY_A).state() == platform::Key::State::Pressed) {
        camera->move_camera(graphics::Camera::Movement::LEFT, dt);
    }
    if (platform->key(platform::KEY_D).state() == platform::Key::State::Pressed) {
        camera->move_camera(graphics::Camera::Movement::RIGHT, dt);
    }
    auto mouse = platform->mouse();
    camera->rotate_camera(mouse.dx, mouse.dy);
}

void MainPlatformEventObserver::on_key(engine::platform::Key key) {
    spdlog::debug("MainPlatformEventObserver::on_key");
}

void MainPlatformEventObserver::on_mouse_move(engine::platform::MousePosition position) {
    spdlog::debug("MainPlatformEventObserver::on_mouse_move");
}

void MainPlatformEventObserver::on_scroll(engine::platform::MousePosition position) {
    auto gui = engine::core::Controller::get<GUIController>();
    if (gui->is_enabled()) {
        return;
    }

    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto camera = graphics->camera();
    camera->zoom(position.scroll);
    graphics->perspective_params().FOV = glm::radians(camera->Zoom);
}
}// namespace engine::black_lodge::app
