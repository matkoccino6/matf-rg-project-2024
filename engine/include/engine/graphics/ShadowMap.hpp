#ifndef MATF_RG_PROJECT_SHADOW_MAP_HPP
#define MATF_RG_PROJECT_SHADOW_MAP_HPP

#include <array>
#include <cstdint>
#include <memory>

#include <glm/glm.hpp>

namespace engine::graphics {
class Framebuffer;

enum class ShadowMapType {
    Basic,
    Spot,
    Point
};

struct ShadowMapDescription {
    ShadowMapType type{ShadowMapType::Basic};
    int resolution{2048};

    float left{-20.0f};
    float right{20.0f};
    float bottom{-20.0f};
    float top{20.0f};
    float near_plane{0.1f};
    float far_plane{100.0f};
    float field_of_view{90.0f};

    glm::vec3 light_position{0.0f, 10.0f, 0.0f};
    glm::vec3 target{0.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
};

class ShadowMap {
public:
    explicit ShadowMap(ShadowMapDescription description = {});
    ShadowMap(int resolution, ShadowMapType type = ShadowMapType::Basic);
    ~ShadowMap();

    ShadowMap(const ShadowMap &) = delete;
    ShadowMap &operator=(const ShadowMap &) = delete;

    void begin_render() const;
    static void end_render();
    void resize(int resolution);

    void set_light(const glm::vec3 &position, const glm::vec3 &target,
                   const glm::vec3 &up = {0.0f, 1.0f, 0.0f});
    void set_clip_planes(float near_plane, float far_plane);
    void set_orthographic_bounds(float left, float right, float bottom, float top);
    void set_field_of_view(float field_of_view);

    ShadowMapType type() const { return m_description.type; }
    int resolution() const { return m_description.resolution; }
    uint32_t depth_texture() const;
    uint32_t texture_target() const;
    void bind_texture(unsigned texture_unit) const;

    const glm::mat4 &light_view_projection() const { return m_light_view_projection; }
    const std::array<glm::mat4, 6> &point_view_projection_matrices() const {
        return m_point_view_projections;
    }

private:
    void validate_description() const;
    void update_matrices();

    ShadowMapDescription m_description;
    std::unique_ptr<Framebuffer> m_buffer;
    glm::mat4 m_light_view_projection{1.0f};
    std::array<glm::mat4, 6> m_point_view_projections{};
};
}// namespace engine::graphics

#endif
