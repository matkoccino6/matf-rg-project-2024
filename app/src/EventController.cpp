#include <EventController.hpp>
#include <SettingsController.hpp>
#include <cmath>
#include <engine/platform/PlatformController.hpp>
#include <spdlog/spdlog.h>

namespace engine::black_lodge::app {
void EventController::initialize() {
    const auto settings = get<SettingsController>();
    m_original_dir_light_color = settings->u_dlight_color;
    m_original_dir_light_intensity = settings->u_dlight_intensity;
    m_original_point1_color = settings->u_plight_color1;
    m_original_point2_color = settings->u_plight_color2;
    m_original_point_intensity = settings->u_plight_intensity;
    m_original_spot_light_color = settings->u_slight_color;
    m_original_spot_light_intensity = settings->u_slight_intensity;
    m_original_ambient_strength = settings->u_ambient_strength;
    m_original_emissive_factor = settings->u_emissive_factor;
    m_original_exposure = settings->u_exposure;
}

void EventController::trigger_sequence() {
    if (m_phase != Phase::Idle) {
        return;
    }
    m_phase = Phase::WaitingForA;
    m_elapsed = 0.0f;
    m_flicker_timer = 0.0f;
    spdlog::info("Event sequence started (ACTION)");
}

void EventController::check_action() {
    const auto platform = get<platform::PlatformController>();
    const bool key_pressed =
            platform->key(platform::KeyId::KEY_E).state() ==
            platform::Key::State::JustPressed;

    if (key_pressed) {
        trigger_sequence();
    }
}

void EventController::trigger_event_a() {
    spdlog::info("EVENT_A: Ambient light turns red");
    const auto settings = get<SettingsController>();
    settings->u_plight_color1 = glm::vec3(0.08f, 0.0f, 1.0f);
    settings->u_plight_color2 = glm::vec3(0.12f, 1.0f, 0.0f);
    settings->u_plight_intensity = 4.0f;
    settings->u_slight_color = glm::vec3(0.5f, 0.0f, 1.0f);
    settings->u_slight_intensity = 0.25f;
    settings->u_ambient_strength = 0.0f;
    settings->u_emissive_factor = 1000.0f;
}

void EventController::trigger_event_b() const {
    const auto settings = get<SettingsController>();
    const float time_in_cycle = std::fmod(m_flicker_timer, 0.18f);
    const bool flash_on = time_in_cycle < 0.07f ||
                          std::sin(m_flicker_timer * 35.0f) > 0.92f;
    const float intensity = flash_on ? 35.0f : 0.02f;

    //settings->u_dlight_color = glm::vec3(1.0f, 0.9f, 0.01f);
    //settings->u_dlight_intensity = flash_on ? 2.5f : 0.02f;
    settings->u_bloom_intensity = flash_on ? 0.2f : 0.1f;
    settings->u_plight_color1 = glm::vec3(1.0f, 0.05f, 0.02f);
    settings->u_plight_color2 = glm::vec3(0.2f, 0.03f, 1.0f);
    settings->u_plight_intensity = intensity;
    settings->u_slight_color = glm::vec3(0.3f, 0.1f, 0.85f);
    settings->u_slight_intensity = flash_on ? 30.0f : 0.0f;
    settings->u_ambient_strength = flash_on ? 0.0005f : 0.0f;
    settings->u_emissive_factor = flash_on ? 0.0f : 2500.0f;
    settings->u_exposure = flash_on ? 2.0f : 0.1f;
    settings->u_lamp_emission_factor = flash_on ? 20.0f : 10.0f;
}

void EventController::reset_sequence() {
    const auto settings = get<SettingsController>();
    settings->u_dlight_color = m_original_dir_light_color;
    settings->u_dlight_intensity = m_original_dir_light_intensity;
    settings->u_plight_color1 = m_original_point1_color;
    settings->u_plight_color2 = m_original_point2_color;
    settings->u_plight_intensity = m_original_point_intensity;
    settings->u_slight_color = m_original_spot_light_color;
    settings->u_slight_intensity = m_original_spot_light_intensity;
    settings->u_ambient_strength = m_original_ambient_strength;
    settings->u_emissive_factor = m_original_emissive_factor;
    settings->u_exposure = m_original_exposure;
    settings->u_bloom_intensity = m_original_bloom_intensity;
    settings->u_lamp_emission_factor = m_original_lamp_factor;
    m_flicker_timer = 0.0f;
    m_elapsed = 0.0f;
    m_phase = Phase::Idle;
    spdlog::info("Event sequence reset");
}

void EventController::update() {
    const auto platform = get<platform::PlatformController>();
    const float dt = platform->dt();
    switch (m_phase) {
        case Phase::Idle:
            check_action();
            break;

        case Phase::WaitingForA:
            m_elapsed += dt;
            if (m_elapsed >= DELAY_A) {
                trigger_event_a();
                m_phase = Phase::WaitingForB;
                m_elapsed = 0.0f;
            }
            break;

        case Phase::WaitingForB:
            m_elapsed += dt;
            if (m_elapsed >= DELAY_B) {
                m_elapsed = 0.0f;
                m_phase = Phase::EventB;
                spdlog::info("EVENT_B: Flickering");
            }
            break;

        case Phase::EventB:
            m_elapsed += dt;
            m_flicker_timer += dt;
            trigger_event_b();
            if (m_elapsed >= EVENT_B_DURATION) {
                reset_sequence();
            }
            break;
    }
}
}// namespace engine::black_lodge::app
