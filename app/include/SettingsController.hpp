#ifndef MATF_RG_PROJECT_SETTINGS_HPP
#define MATF_RG_PROJECT_SETTINGS_HPP
#include <engine/core/Controller.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

namespace engine::black_lodge::app {
class SettingsController final : public core::Controller {
public:
    float u_ambient_strength = 0.05f;
    float far = 200.0f;
    glm::vec3 camera_position = glm::vec3(0.0f, 10.0f, 0.0f);

    glm::vec3 u_plight_pos1 = glm::vec3(10.9f, 17.4f, 52.9f);
    glm::vec3 u_plight_pos2 = glm::vec3(39.9f, 17.4f, 52.9f);
    glm::vec3 u_dlight_dir = glm::vec3(-10.0f);
    glm::vec3 u_slight_pos = glm::vec3(0.0f, 45.0f, 0.0f);
    glm::vec3 u_slight_dir = glm::vec3(20.0f, -33.0f, 42.0f);

    glm::vec3 u_plight_color1 = glm::vec3(1.0f);
    glm::vec3 u_plight_color2 = glm::vec3(1.0f);
    glm::vec3 u_dlight_color = glm::vec3(1.0f);
    glm::vec3 u_slight_color = glm::vec3(1.0f);

    float u_plight_intensity = 1.0f;
    float u_dlight_intensity = 0.0f;
    float u_slight_intensity = 20.0f;

    float u_slight_cut_off = 30.0f;
    float u_slight_outer_cut_off = 50.0f;

    float u_exposure = 1.0f;
    float u_r_factor = 0.15f;
    float u_emissive_factor = 1.0f;
    int u_bloom_mip_levels = 6;
    float u_bloom_filter_radius = 0.005f;
    float u_bloom_intensity = 0.04f;

    std::string_view name() const override {
        return "engine:black_lodge::app::SettingsController";
    };

    void reset();

private:
    void initialize() override;
};
}// namespace engine::black_lodge::app
#endif
