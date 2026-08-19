#include <GUIController.hpp>
#include <engine/core/Engine.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <imgui.h>
#include <SettingsController.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace engine::blackLodge::app {
    void GUIController::initialize() {
        set_enable(false);
    }

    void GUIController::poll_events() {
        const auto platform = engine::core::Controller::get<platform::PlatformController>();
        if (platform->key(platform::KeyId::KEY_F2).state() == platform::Key::State::JustPressed) {
            set_enable(!is_enabled());
        }
    }

    void GUIController::draw() {
        auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
        auto camera   = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();
        auto settings = Controller::get<SettingsController>();
        graphics->begin_gui();
        ImGui::Begin("Camera info");
        const auto &c = *camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::SliderFloat("Ambient Strength", &settings->m_ambientStrength, 0.0f, 1.0f);

        ImGui::SliderFloat3("Point Light Position 1", glm::value_ptr(settings->uPLightPos1), -100.0f, 100.0f);
        ImGui::SliderFloat3("Point Light Position 2", glm::value_ptr(settings->uPLightPos2), -100.0f, 100.0f);
        ImGui::SliderFloat3("Direction Light", glm::value_ptr(settings->uDLightDir), -100.0f, 100.0f);

        ImGui::ColorEdit3("Point Light Color 1", glm::value_ptr(settings->uPLightColor1));
        ImGui::ColorEdit3("Point Light Color 2", glm::value_ptr(settings->uPLightColor2));
        ImGui::SliderFloat("Point Light Intensity", &settings->uPLightIntensity, 0.0f, 20.0f);
        ImGui::SliderFloat("Direct Light Intensity", &settings->uDLightIntensity, 0.0f, 20.0f);
        ImGui::ColorEdit3("Direction Color", glm::value_ptr(settings->uDLightColor));
        ImGui::SliderFloat("Far", &graphics->perspective_params().Far, 0.0f, 1000.0f);
        ImGui::End();
        graphics->end_gui();
    }
}
