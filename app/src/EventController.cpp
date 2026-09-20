//
// Created by mateja on 14. 9. 2026..
//
#include <EventController.hpp>
#include <SettingsController.hpp>
#include <cmath>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/platform/PlatformController.hpp>
#include <spdlog/spdlog.h>

namespace engine::black_lodge::app {
void EventController::initialize() {
    auto *settings = engine::core::Controller::get<SettingsController>();
    m_original_dir_light_color = settings->u_dlight_color;
    m_original_dir_light_intensity = settings->u_dlight_intensity;
    m_original_point1_color = settings->u_plight_color1;
    m_original_point2_color = settings->u_plight_color2;
    m_original_point1_intensity = settings->u_plight_intensity;
    m_original_point2_intensity = settings->u_plight_intensity;
    m_original_spot_light_color = settings->u_slight_color;
    m_original_spot_light_intensity = settings->u_slight_intensity;
    m_original_ambient_strength = settings->u_ambient_strength;
}

void EventController::trigger_sequence() {
    if (m_phase != Phase::Idle && m_phase != Phase::Done)
        return;
    m_phase = Phase::WaitingForA;
    m_elapsed = 0.0f;
    m_flicker_timer = 0.0f;
    m_flicker_active = false;
    spdlog::info("Event sequence started (ACTION)");
}

void EventController::check_action() {
    /*auto* graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
        const auto& cam_pos = graphics->camera()->Position;
        const float distance_to_center = glm::length(glm::vec2(cam_pos.x, cam_pos.z));
        const bool in_zone = distance_to_center < 3.0f;*/

    auto *platform = engine::core::Controller::get<engine::platform::PlatformController>();
    const bool key_pressed =
            platform->key(engine::platform::KeyId::KEY_E).state() ==
            engine::platform::Key::State::JustPressed;

    if (key_pressed) {
        trigger_sequence();
    }
}

void EventController::trigger_event_a() {
    spdlog::info("EVENT_A: Ambient light turns red");
    auto *settings = engine::core::Controller::get<SettingsController>();
    settings->u_dlight_color = glm::vec3(0.18f, 0.01f, 0.01f);
    settings->u_dlight_intensity = 0.08f;
    settings->u_plight_color1 = glm::vec3(0.08f, 0.0f, 1.0f);
    settings->u_plight_color2 = glm::vec3(0.12f, 1.0f, 0.0f);
    settings->u_plight_intensity = 0.08f;
    settings->u_slight_color = glm::vec3(0.5f, 0.0f, 1.0f);
    settings->u_slight_intensity = 0.15f;
    settings->u_ambient_strength = 0.01f;
}

void EventController::trigger_event_b() {
    auto *settings = engine::core::Controller::get<SettingsController>();
    const float time_in_cycle = std::fmod(m_flicker_timer, 0.18f);
    const bool flash_on = time_in_cycle < 0.07f ||
                          std::sin(m_flicker_timer * 41.0f) > 0.92f;
    const float intensity = flash_on ? 35.0f : 0.02f;

    settings->u_dlight_color = glm::vec3(1.0f, 0.9f, 0.01f);
    settings->u_dlight_intensity = flash_on ? 2.5f : 0.02f;
    settings->u_plight_color1 = glm::vec3(1.0f, 0.05f, 0.02f);
    settings->u_plight_color2 = glm::vec3(0.8f, 0.9f, 1.0f);
    settings->u_plight_intensity = intensity;
    settings->u_slight_color = glm::vec3(0.3f, 0.1f, 0.85f);
    settings->u_slight_intensity = flash_on ? 18.0f : 0.0f;
    settings->u_ambient_strength = flash_on ? 0.2f : 0.005f;
    settings->u_emissive_factor = flash_on ? 0.0f : 1000.0f;
}

void EventController::reset_sequence() {
    auto *settings = engine::core::Controller::get<SettingsController>();
    settings->u_dlight_color = m_original_dir_light_color;
    settings->u_dlight_intensity = m_original_dir_light_intensity;
    settings->u_plight_color1 = m_original_point1_color;
    settings->u_plight_color2 = m_original_point2_color;
    settings->u_plight_intensity = m_original_point2_intensity;
    settings->u_slight_color = m_original_spot_light_color;
    settings->u_slight_intensity = m_original_spot_light_intensity;
    settings->u_ambient_strength = m_original_ambient_strength;
    m_flicker_active = false;
    m_flicker_timer = 0.0f;
    m_elapsed = 0.0f;
    spdlog::info("Event sequence reset");
}

void EventController::update() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
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
                m_flicker_active = true;
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
                m_phase = Phase::Done;
            }
            break;

        case Phase::Done: break;
    }
}

const char *EventController::current_phase_name() const {
    switch (m_phase) {
        case Phase::Idle: return "Idle (waiting for action)";
        case Phase::WaitingForA: return "Waiting for EVENT_A...";
        case Phase::WaitingForB: return "Waiting for EVENT_B...";
        case Phase::EventB: return "Event B triggered";
        case Phase::Done: return "Sequence complete";
    }
    return "Unknown";
}
}// namespace engine::black_lodge::app
