#include <GUIController.hpp>
#include <SettingsController.hpp>
#include <engine/core/Engine.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace engine::black_lodge::app {
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
    auto camera = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();
    auto settings = Controller::get<SettingsController>();
    const auto &c = *camera;
    graphics->begin_gui();
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Camera info and Light Control", nullptr, ImGuiWindowFlags_NoSavedSettings);
    ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
    ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
    ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
    if (ImGui::BeginTabBar("LightTabs")) {
        if (ImGui::BeginTabItem("Directional")) {
            ImGui::SliderFloat3("Directional Light direction", glm::value_ptr(settings->u_dlight_dir), -100.0f,
                                100.0f);
            ImGui::SliderFloat("Directional Light Intensity", &settings->u_dlight_intensity, 0.0f, 20.0f);
            ImGui::ColorEdit3("Direction Color", glm::value_ptr(settings->u_dlight_color));
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Point Lights")) {
            ImGui::SliderFloat3("Point Light Position 1", glm::value_ptr(settings->u_plight_pos1), -100.0f, 100.0f);
            ImGui::SliderFloat3("Point Light Position 2", glm::value_ptr(settings->u_plight_pos2), -100.0f, 100.0f);

            ImGui::ColorEdit3("Point Light Color 1", glm::value_ptr(settings->u_plight_color1));
            ImGui::ColorEdit3("Point Light Color 2", glm::value_ptr(settings->u_plight_color2));
            ImGui::SliderFloat("Point Light Intensity", &settings->u_plight_intensity, 0.0f, 20.0f);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Global")) {
            ImGui::SliderFloat("Ambient Strength", &settings->m_ambient_strength, 0.0f, 1.0f);
            ImGui::SliderFloat("Far", &graphics->perspective_params().Far, 0.0f, 1000.0f);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
        if (ImGui::Button("Reset")) {
            settings->reset();
        }
    }
    ImGui::End();
    graphics->end_gui();
}
}// namespace engine::black_lodge::app
