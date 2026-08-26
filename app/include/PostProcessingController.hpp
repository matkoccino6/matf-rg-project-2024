//
// Created by mateja on 26. 8. 2026..
//

#ifndef MATF_RG_PROJECT_POSTPROCESSINGCONTROLLER_HPP
#define MATF_RG_PROJECT_POSTPROCESSINGCONTROLLER_HPP
#include <engine/core/Controller.hpp>
#include <engine/graphics/Framebuffer.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/platform/PlatformEventObserver.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <memory>

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

    std::unique_ptr<engine::graphics::Framebuffer> m_scene_buffer;
    std::array<std::unique_ptr<engine::graphics::Framebuffer>, 2> m_blur_buffers;
    engine::resources::Shader *m_shader{};
    engine::resources::Shader *m_blur_shader{};
};
}// namespace engine::black_lodge::app
#endif//MATF_RG_PROJECT_POSTPROCESSINGCONTROLLER_HPP
