//
// Created by mateja on 17. 8. 2026..
//
#include <SettingsController.hpp>
#include <spdlog/spdlog.h>

namespace engine::black_lodge::app {
void SettingsController::initialize() {
    spdlog::info("SettingsController::initialize");
}

void SettingsController::reset() {
    m_ambient_strength = 0.1f;
    m_shininess = 32.0f;
    u_plight_pos1 = glm::vec3(10.9f, 17.4f, 52.9f);
    u_plight_pos2 = glm::vec3(39.9f, 17.4f, 52.9f);
    u_dlight_dir = glm::vec3(-10.0f);

    u_plight_color1 = glm::vec3(1.0f);
    u_plight_color2 = glm::vec3(1.0f);
    u_dlight_color = glm::vec3(1.0f);

    u_plight_intensity = 1.0f;
    u_dlight_intensity = 1.0f;
}
}// namespace engine::black_lodge::app
