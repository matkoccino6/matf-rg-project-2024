#include <algorithm>
#include <engine/graphics/Bloom.hpp>
#include <engine/graphics/Framebuffer.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/Shader.hpp>

namespace engine::graphics {
Bloom::~Bloom() {
    terminate();
}

void Bloom::initialize(int width, int height, resources::Shader *composite_shader,
                       resources::Shader *blur_shader) {
    m_composite_shader = composite_shader;
    m_blur_shader = blur_shader;

    m_scene_buffer = std::make_unique<Framebuffer>(
            FramebufferDescription{
                    .width = width,
                    .height = height,
                    .color_formats = {FramebufferFormat::RGBA16F, FramebufferFormat::RGBA16F},
                    .depth_format = FramebufferFormat::Depth24Stencil8});

    for (auto &buffer: m_blur_buffers) {
        buffer = std::make_unique<Framebuffer>(
                FramebufferDescription{
                        .width = width,
                        .height = height,
                        .color_formats = {FramebufferFormat::RGBA16F},
                        .depth_format = std::nullopt});
    }
}

void Bloom::terminate() {
    m_composite_shader = nullptr;
    m_blur_shader = nullptr;
    for (auto &buffer: m_blur_buffers) {
        buffer.reset();
    }
    m_scene_buffer.reset();
}

void Bloom::begin_draw() {
    m_scene_buffer->bind();
    OpenGL::set_viewport(m_scene_buffer->width(), m_scene_buffer->height());
    OpenGL::clear_buffers();
}

void Bloom::apply(float exposure, int blur_passes) {
    OpenGL::disable_depth_testing();

    uint32_t source_texture = m_scene_buffer->color_texture(1);
    const int passes = std::clamp(blur_passes, 1, 20);
    for (int pass = 0; pass < passes; ++pass) {
        const int target = pass % 2;
        m_blur_buffers[target]->bind();
        OpenGL::set_viewport(m_blur_buffers[target]->width(), m_blur_buffers[target]->height());
        m_blur_shader->use();
        m_blur_shader->set_bool("horizontal", target == 0);
        OpenGL::draw_fullscreen_texture(m_blur_shader, source_texture);
        source_texture = m_blur_buffers[target]->color_texture();
    }

    Framebuffer::bind_default();
    OpenGL::set_viewport(m_scene_buffer->width(), m_scene_buffer->height());
    m_composite_shader->use();
    m_composite_shader->set_float("exposure", exposure);
    OpenGL::draw_fullscreen_composite(m_composite_shader, m_scene_buffer->color_texture(0), source_texture);
    OpenGL::enable_depth_testing();
}

void Bloom::resize(int width, int height) {
    m_scene_buffer->resize(width, height);
    for (auto &buffer: m_blur_buffers) {
        buffer->resize(width, height);
    }
    OpenGL::set_viewport(width, height);
}
}// namespace engine::graphics
