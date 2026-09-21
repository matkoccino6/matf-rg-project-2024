#ifndef MATF_RG_PROJECT_EVENTCONTROLLER_HPP
#define MATF_RG_PROJECT_EVENTCONTROLLER_HPP
#include <engine/core/Controller.hpp>
#include <glm/glm.hpp>

namespace engine::black_lodge::app {
class EventController final : public engine::core::Controller {
public:
    void trigger_sequence();

private:
    enum class Phase {
        Idle,
        WaitingForA,
        WaitingForB,
        EventB
    };

    Phase m_phase = Phase::Idle;
    float m_elapsed = 0.0f;

    static constexpr float DELAY_A = 2.0f;
    static constexpr float DELAY_B = 3.0f;
    static constexpr float EVENT_B_DURATION = 15.0f;

    glm::vec3 m_original_dir_light_color{1.0f};
    float m_original_dir_light_intensity = 1.0f;
    glm::vec3 m_original_point1_color{1.0f};
    glm::vec3 m_original_point2_color{1.0f};
    float m_original_point_intensity = 1.0f;
    glm::vec3 m_original_spot_light_color{1.0f};
    float m_original_spot_light_intensity = 1.0f;
    float m_original_ambient_strength = 0.1f;
    float m_original_emissive_factor = 1.0f;
    float m_flicker_timer = 0.0f;
    float m_original_exposure = 0.0f;
    float m_original_bloom_intensity = 0.04f;

    void initialize() override;

    void update() override;

    void check_action();

    static void trigger_event_a();

    void trigger_event_b() const;

    void reset_sequence();
};
}// namespace engine::black_lodge::app
#endif
