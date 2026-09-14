//
// Created by mateja on 26. 8. 2026..
//

#ifndef MATF_RG_PROJECT_POSTPROCESSINGCONTROLLER_HPP
#define MATF_RG_PROJECT_POSTPROCESSINGCONTROLLER_HPP
#include <engine/core/Controller.hpp>
#include <engine/graphics/Bloom.hpp>
#include <engine/platform/PlatformEventObserver.hpp>

namespace engine::black_lodge::app {
class PostProcessingController final : public engine::core::Controller {
public:
    std::string_view name() const override {
        return "PostProcessingController";
    }

private:
    class ResizeObserver final : public engine::platform::PlatformEventObserver {
    public:
        explicit ResizeObserver(PostProcessingController *controller)
            : m_controller(controller) {
        }

        void on_window_resize(int width, int height) override;

    private:
        PostProcessingController *m_controller;
    };

    void initialize() override;
    void begin_draw() override;
    void draw() override;
    void terminate() override;

    engine::graphics::Bloom m_bloom;
};
}// namespace engine::black_lodge::app
#endif//MATF_RG_PROJECT_POSTPROCESSINGCONTROLLER_HPP
