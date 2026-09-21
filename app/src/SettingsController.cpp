#include <SettingsController.hpp>
#include <spdlog/spdlog.h>

namespace engine::black_lodge::app {
void SettingsController::initialize() {
    spdlog::info("SettingsController::initialize");
}

void SettingsController::reset() {
    u_ambient_strength = 0.1f;
    far = 200.0f;
    u_plight_pos1 = glm::vec3(10.9f, 17.4f, 52.9f);
    u_plight_pos2 = glm::vec3(39.9f, 17.4f, 52.9f);
    u_dlight_dir = glm::vec3(-10.0f);

    u_plight_color1 = glm::vec3(1.0f);
    u_plight_color2 = glm::vec3(1.0f);
    u_dlight_color = glm::vec3(1.0f);
    u_slight_pos = glm::vec3(0.0f, 5.0f, 0.0f);
    u_slight_dir = glm::vec3(0.0f, -1.0f, 0.0f);
    u_slight_cut_off = 12.5f;
    u_slight_outer_cut_off = 17.5f;
    u_slight_color = glm::vec3(1.0f);

    u_plight_intensity = 1.0f;
    u_lamp_emission_factor = 1.0f;
    u_dlight_intensity = 1.0f;
    u_slight_intensity = 3.0f;
    u_exposure = 1.0f;
    u_r_factor = 0.15f;
    u_bloom_mip_levels = 6;
    u_bloom_filter_radius = 0.005f;
    u_emissive_factor = 1.0f;
    u_bloom_intensity = 0.04f;
}
}// namespace engine::black_lodge::app
