#ifndef MATF_RG_PROJECT_SHADOW_CONTROLLER_HPP
#define MATF_RG_PROJECT_SHADOW_CONTROLLER_HPP

#include <array>
#include <memory>

#include <engine/core/Controller.hpp>
#include <engine/graphics/ShadowMap.hpp>

namespace engine::black_lodge::app {
class ShadowController final : public engine::core::Controller {
public:
    std::string_view name() const override {
        return "engine::blackLodge::app::ShadowController";
    }

    const engine::graphics::ShadowMap &directional_shadow_map() const {
        return *m_directional_shadow_map;
    }

    const engine::graphics::ShadowMap &point_shadow_map(std::size_t index) const {
        return *m_point_shadow_maps.at(index);
    }

    float point_shadow_far_plane() const {
        return m_point_shadow_far_plane;
    }

    static constexpr unsigned directional_texture_unit() {
        return 32;
    }

    static constexpr unsigned point_texture_unit(std::size_t index) {
        return 33 + static_cast<unsigned>(index);
    }

private:
    void initialize() override;
    void begin_draw() override;
    void terminate() override;

    void render_directional_shadow();
    void render_point_shadow(std::size_t index);

    std::unique_ptr<engine::graphics::ShadowMap> m_directional_shadow_map;
    std::array<std::unique_ptr<engine::graphics::ShadowMap>, 2> m_point_shadow_maps;
    float m_point_shadow_far_plane{};
};
}// namespace engine::black_lodge::app

#endif
