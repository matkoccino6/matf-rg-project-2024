#include <algorithm>
// clang-format off
#include <glad/glad.h>
// clang-format on
#include <engine/graphics/Bloom.hpp>
#include <engine/graphics/Framebuffer.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/Shader.hpp>

namespace engine::graphics {
Bloom::~Bloom() {
    terminate();
}

void Bloom::initialize(int width, int height, resources::Shader *composite_shader,
                       resources::Shader *downsample_shader, resources::Shader *upsample_shader,
                       int mip_levels) {
    m_composite_shader = composite_shader;
    m_downsample_shader = downsample_shader;
    m_upsample_shader = upsample_shader;

    m_scene_buffer = std::make_unique<Framebuffer>(
            FramebufferDescription{
                    .width = width,
                    .height = height,
                    .color_formats = {FramebufferFormat::RGBA16F},
                    .depth_format = FramebufferFormat::Depth24Stencil8});

    m_mip_buffers.clear();
    const int levels = std::clamp(mip_levels, 1, 13);
    m_mip_buffers.reserve(levels);
    for (int level = 0; level < levels; ++level) {
        m_mip_buffers.push_back(std::make_unique<Framebuffer>(
                FramebufferDescription{
                        .width = std::max(width >> (level + 1), 1),
                        .height = std::max(height >> (level + 1), 1),
                        .color_formats = {FramebufferFormat::RGBA16F},
                        .depth_format = std::nullopt}));
    }
}

void Bloom::terminate() {
    m_composite_shader = nullptr;
    m_downsample_shader = nullptr;
    m_upsample_shader = nullptr;
    m_mip_buffers.clear();
    m_scene_buffer.reset();
}

void Bloom::begin_draw() {
    m_scene_buffer->bind();
    OpenGL::set_viewport(m_scene_buffer->width(), m_scene_buffer->height());
    OpenGL::clear_buffers();
}

void Bloom::apply(float exposure, int mip_levels, float bloom_filter_radius, float bloom_intensity) {
    OpenGL::disable_depth_testing();

    const int levels = std::clamp(mip_levels, 1, static_cast<int>(m_mip_buffers.size()));
    uint32_t source_texture = m_scene_buffer->color_texture(0);
    for (int level = 0; level < levels; ++level) {
        auto &buffer = m_mip_buffers[level];
        buffer->bind();
        OpenGL::set_viewport(buffer->width(), buffer->height());
        m_downsample_shader->use();
        m_downsample_shader->set_int("mipLevel", level);
        OpenGL::draw_fullscreen_texture(m_downsample_shader, source_texture);
        source_texture = buffer->color_texture();
    }

    CHECKED_GL_CALL(glEnable, GL_BLEND);
    CHECKED_GL_CALL(glBlendFunc, GL_ONE, GL_ONE);
    for (int level = levels - 2; level >= 0; --level) {
        auto &buffer = m_mip_buffers[level];
        buffer->bind();
        OpenGL::set_viewport(buffer->width(), buffer->height());
        m_upsample_shader->use();
        m_upsample_shader->set_float("filterRadius", bloom_filter_radius);
        OpenGL::draw_fullscreen_texture(m_upsample_shader, source_texture);
        source_texture = buffer->color_texture();
    }
    CHECKED_GL_CALL(glDisable, GL_BLEND);

    Framebuffer::bind_default();
    OpenGL::set_viewport(m_scene_buffer->width(), m_scene_buffer->height());
    m_composite_shader->use();
    m_composite_shader->set_float("exposure", exposure);
    m_composite_shader->set_float("bloomIntensity", bloom_intensity);
    OpenGL::draw_fullscreen_composite(m_composite_shader, m_scene_buffer->color_texture(0), source_texture);
    OpenGL::enable_depth_testing();
}

void Bloom::resize(int width, int height) {
    m_scene_buffer->resize(width, height);
    for (std::size_t level = 0; level < m_mip_buffers.size(); ++level) {
        m_mip_buffers[level]->resize(std::max(width >> (level + 1), 1),
                                     std::max(height >> (level + 1), 1));
    }
    OpenGL::set_viewport(width, height);
}
}// namespace engine::graphics
