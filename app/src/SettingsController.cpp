//
// Created by mateja on 17. 8. 2026..
//
#include <SettingsController.hpp>
#include <spdlog/spdlog.h>

namespace engine::blackLodge::app {
    void SettingsController::initialize() {
        spdlog::info("SettingsController::initialize");
    }

    void SettingsController::reset() {
        m_ambientStrength = 0.1f;
        m_shininess       = 32.0f;
        uPLightPos1       = glm::vec3(10.9f, 17.4f, 52.9f);
        uPLightPos2       = glm::vec3(39.9f, 17.4f, 52.9f);
        uDLightDir        = glm::vec3(-10.0f);

        uPLightColor1 = glm::vec3(1.0f);
        uPLightColor2 = glm::vec3(1.0f);
        uDLightColor  = glm::vec3(1.0f);

        uPLightIntensity = 1.0f;
        uDLightIntensity = 1.0f;
    }
}
