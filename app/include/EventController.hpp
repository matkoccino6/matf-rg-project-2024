//
// Created by mateja on 14. 9. 2026..
//

#ifndef MATF_RG_PROJECT_EVENTCONTROLLER_HPP
#define MATF_RG_PROJECT_EVENTCONTROLLER_HPP
#include <engine/core/Controller.hpp>
#include <glm/glm.hpp>

namespace engine::black_lodge::app {
class EventController final : public engine::core::Controller {
public:
    void initialize() override;

    void update() override;

    const char *current_phase_name() const;

    bool sequence_active() const {
        return m_phase != Phase::Idle;
    }

    void trigger_sequence();

private:
    enum class Phase {
        Idle,
        WaitingForA,
        WaitingForB,
        EventB,
        Done
    };

    Phase m_phase = Phase::Idle;
    float m_elapsed = 0.0f;
    bool m_flicker_active = false;

    static constexpr float DELAY_A = 2.0f;
    static constexpr float DELAY_B = 3.0f;
    static constexpr float EVENT_B_DURATION = 10.0f;

    glm::vec3 m_original_dir_light_color{1.0f};
    float m_original_dir_light_intensity = 1.0f;
    glm::vec3 m_original_point1_color{1.0f};
    glm::vec3 m_original_point2_color{1.0f};
    float m_original_point1_intensity = 1.0f;
    float m_original_point2_intensity = 1.0f;
    glm::vec3 m_original_spot_light_color{1.0f};
    float m_original_spot_light_intensity = 1.0f;
    float m_original_ambient_strength = 0.1f;
    float m_flicker_timer = 0.0f;

    void check_action();

    void trigger_event_a();

    void trigger_event_b();

    void reset_sequence();
};
}// namespace engine::black_lodge::app
#endif//MATF_RG_PROJECT_EVENTCONTROLLER_HPP
