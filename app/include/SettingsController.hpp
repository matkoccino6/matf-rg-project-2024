//
// Created by mateja on 17. 8. 2026..
//

#ifndef MATF_RG_PROJECT_SETTINGS_HPP
#define MATF_RG_PROJECT_SETTINGS_HPP
#include <engine/core/Controller.hpp>
#include <glm/vec3.hpp>

namespace engine::black_lodge::app {
    class SettingsController final : public engine::core::Controller {
    public:
        float m_ambient_strength    = 0.1f;
        float m_shininess           = 32.0f;
        float m_far                 = 200.0f;
        glm::vec3 m_camera_position = glm::vec3(0.0f, 10.0f, 0.0f);
        glm::vec3 u_plight_pos1     = glm::vec3(10.9f, 17.4f, 52.9f);
        glm::vec3 u_plight_pos2     = glm::vec3(39.9f, 17.4f, 52.9f);
        glm::vec3 u_dlight_dir      = glm::vec3(-10.0f);

        glm::vec3 u_plight_color1 = glm::vec3(1.0f);
        glm::vec3 u_plight_color2 = glm::vec3(1.0f);
        glm::vec3 u_dlight_color  = glm::vec3(1.0f);

        float u_plight_intensity = 1.0f;
        float u_dlight_intensity = 1.0f;

        std::string_view name() const override {
            return "engine:black_lodge::app::SettingsController";
        };

        void reset();

    private:
        void initialize() override;
    };
} // namespace engine::blackLodge::app
#endif//MATF_RG_PROJECT_SETTINGS_HPP
