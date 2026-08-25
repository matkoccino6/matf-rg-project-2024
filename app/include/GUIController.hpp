//
// Created by mateja on 12. 8. 2026..
//

#ifndef MATF_RG_PROJECT_GUICONTROLLER_HPP
#define MATF_RG_PROJECT_GUICONTROLLER_HPP
#include <engine/core/Engine.hpp>

namespace engine::black_lodge::app {
class GUIController final : public engine::core::Controller {
public:
    std::string_view name() const override {
        return "engine::blackLodge::app::GUIController";
    }

private:
    void initialize() override;

    void poll_events() override;

    void draw() override;
};
}// namespace engine::black_lodge::app
#endif//MATF_RG_PROJECT_GUICONTROLLER_HPP
