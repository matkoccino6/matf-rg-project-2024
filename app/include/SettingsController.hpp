//
// Created by mateja on 17. 8. 2026..
//

#ifndef MATF_RG_PROJECT_SETTINGS_HPP
#define MATF_RG_PROJECT_SETTINGS_HPP
#include <engine/core/Controller.hpp>
#include <glm/vec3.hpp>

namespace engine::blackLodge::app {
    class SettingsController final : public engine::core::Controller {
    public:
        float m_ambientStrength = 0.1f;
        float m_shininess       = 32.0f;
        glm::vec3 uPLightPos1   = glm::vec3(10.9f, 17.4f, 52.9f);
        glm::vec3 uPLightPos2   = glm::vec3(39.9f, 17.4f, 52.9f);
        glm::vec3 uDLightDir    = glm::vec3(-10.0f);

        glm::vec3 uPLightColor1 = glm::vec3(1.0f);
        glm::vec3 uPLightColor2 = glm::vec3(1.0f);
        glm::vec3 uDLightColor  = glm::vec3(1.0f);

        float uPLightIntensity = 1.0f;
        float uDLightIntensity = 1.0f;

        std::string_view name() const override {
            return "engine:blackLodge::app::SettingsController";
        };

    private:
        void initialize() override;
    };
}
#endif //MATF_RG_PROJECT_SETTINGS_HPP
