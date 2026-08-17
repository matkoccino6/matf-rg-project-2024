#include <MainController.hpp>
#include <GUIController.hpp>
#include <engine/core/Engine.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <memory>
#include <spdlog/spdlog.h>

namespace engine::blackLodge::app {
    void MainController::initialize() {
        spdlog::info("MainController::initialize");
        engine::graphics::OpenGL::enable_depth_testing();
        auto observer = std::make_unique<MainPlatformEventObserver>();
        engine::core::Controller::get<engine::platform::PlatformController>()->register_platform_event_observer(
            std::move(observer));
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
    }

    void MainController::begin_draw() {
        spdlog::debug("MainController::begin_draw");
        engine::graphics::OpenGL::clear_buffers();
    }

    void MainController::draw() {
        spdlog::debug("MainController::draw");
        auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
        auto shader   = engine::core::Controller::get<engine::resources::ResourcesController>()->shader("lighting");
        auto floor    = engine::core::Controller::get<engine::resources::ResourcesController>()->model("black_lodge");
        shader->use();
        shader->set_mat4("uProjection", graphics->projection_matrix());
        shader->set_mat4("uView", graphics->camera()->view_matrix());
        shader->set_mat4("uModel", glm::scale(glm::mat4(1.0f), glm::vec3(m_scale)));
        shader->set_vec3("uLightPos", glm::vec3(40.0f, 18.0f, 53.0f));
        shader->set_vec3("uLightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        shader->set_vec3("uViewPos", graphics->camera()->Position);
        shader->set_float("uShininess", 32.0f);
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
        auto camera   = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();
        float dt      = platform->dt();
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
        camera->zoom(mouse.scroll);
    }

    void MainPlatformEventObserver::on_key(engine::platform::Key key) {
        spdlog::debug("MainPlatformEventObserver::on_key");
    }

    void MainPlatformEventObserver::on_mouse_move(engine::platform::MousePosition position) {
        spdlog::debug("MainPlatformEventObserver::on_mouse_move");
    }
}
