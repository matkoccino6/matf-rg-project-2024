#include <SettingsController.hpp>
#include <spdlog/spdlog.h>

namespace engine::black_lodge::app {
void SettingsController::initialize() {
    spdlog::info("SettingsController::initialize");
}

void SettingsController::reset() {
    m_scale = 0.1f;
    u_ambient_strength = 0.01f;
    far = 250.0f * m_scale;
    camera_position = glm::vec3(0.0f, 2.0f, 0.0f);
    u_plight_pos1 = glm::vec3(-3.99f, 1.9f, -5.29f);
    u_plight_pos2 = glm::vec3(-1.1f, 1.9f, -5.29f);
    u_dlight_dir = glm::vec3(-10.0f) * m_scale;
    u_slight_pos = glm::vec3(1.08f, 6.39f, 1.68f);
    u_slight_dir = glm::vec3(-6.93f, -7.65f, -9.46f);

    u_plight_color1 = glm::vec3(1.0f);
    u_plight_color2 = glm::vec3(1.0f);
    u_dlight_color = glm::vec3(1.0f);
    u_slight_color = glm::vec3(1.0f, 0.7f, 0.4f);

    u_plight_intensity = 3.0f * m_scale;
    u_lamp_emission_factor = 3.0f;
    u_dlight_intensity = 0.0f;
    u_slight_intensity = 3.0f;
    u_slight_cut_off = 15.0f;
    u_slight_outer_cut_off = 30.0f;
    u_exposure = 1.0f;
    u_r_factor = 0.15f;
    u_bloom_mip_levels = 6;
    u_bloom_filter_radius = 0.005f;
    u_emissive_factor = 1.0f;
    u_bloom_intensity = 0.04f;
}
}// namespace engine::black_lodge::app
