#include <PostProcessingController.hpp>
#include <SettingsController.hpp>
#include <algorithm>
#include <engine/core/Engine.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/ResourcesController.hpp>

namespace engine::black_lodge::app {
void PostProcessingController::initialize() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();

    m_scene_buffer = std::make_unique<engine::graphics::Framebuffer>(
            engine::graphics::FramebufferDescription{
                    .width = platform->window()->width(),
                    .height = platform->window()->height(),
                    .color_formats = {graphics::FramebufferFormat::RGBA16F, graphics::FramebufferFormat::RGBA16F},
                    .depth_format = graphics::FramebufferFormat::Depth24Stencil8});

    for (auto &buffer: m_blur_buffers) {
        buffer = std::make_unique<engine::graphics::Framebuffer>(
                engine::graphics::FramebufferDescription{
                        .width = platform->window()->width(),
                        .height = platform->window()->height(),
                        .color_formats = {graphics::FramebufferFormat::RGBA16F},
                        .depth_format = std::nullopt});
    }

    m_shader = resources->shader("post_processing");
    m_blur_shader = resources->shader("blur");
    platform->register_platform_event_observer(std::make_unique<ResizeObserver>(this));
}

void PostProcessingController::begin_draw() {
    m_scene_buffer->bind();
    engine::graphics::OpenGL::set_viewport(m_scene_buffer->width(), m_scene_buffer->height());
    engine::graphics::OpenGL::clear_buffers();
}

void PostProcessingController::draw() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto settings = get<SettingsController>();
    engine::graphics::OpenGL::disable_depth_testing();

    uint32_t source_texture = m_scene_buffer->color_texture(1);
    const int blur_passes = std::clamp(settings->u_bloom_blur_passes, 1, 20);
    for (int pass = 0; pass < blur_passes; ++pass) {
        const int target = pass % 2;
        m_blur_buffers[target]->bind();
        engine::graphics::OpenGL::set_viewport(m_blur_buffers[target]->width(),
                                               m_blur_buffers[target]->height());
        m_blur_shader->use();
        m_blur_shader->set_bool("horizontal", target == 0);
        engine::graphics::OpenGL::draw_fullscreen_texture(m_blur_shader, source_texture);
        source_texture = m_blur_buffers[target]->color_texture();
    }

    engine::graphics::Framebuffer::bind_default();
    engine::graphics::OpenGL::set_viewport(platform->window()->width(), platform->window()->height());
    m_shader->use();
    m_shader->set_float("exposure", settings->u_exposure);
    engine::graphics::OpenGL::draw_fullscreen_composite(
            m_shader, m_scene_buffer->color_texture(0), source_texture);
    engine::graphics::OpenGL::enable_depth_testing();
}

void PostProcessingController::terminate() {
    m_shader = nullptr;
    m_blur_shader = nullptr;
    for (auto &buffer: m_blur_buffers) {
        buffer.reset();
    }
    m_scene_buffer.reset();
}

void PostProcessingController::ResizeObserver::on_window_resize(int width, int height) {
    m_controller->m_scene_buffer->resize(width, height);
    for (auto &buffer: m_controller->m_blur_buffers) {
        buffer->resize(width, height);
    }
    engine::graphics::OpenGL::set_viewport(width, height);
}
}// namespace engine::black_lodge::app
