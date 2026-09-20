#include <engine/graphics/Framebuffer.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/util/Errors.hpp>
#include <glad/glad.h>
#include <utility>

namespace engine::graphics {
namespace {
struct ColorFormatInfo {
    GLint internal_format;
    GLenum format;
    GLenum type;
};

ColorFormatInfo color_format_info(FramebufferFormat format) {
    switch (format) {
        case FramebufferFormat::RGBA8:
            return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
        case FramebufferFormat::RGBA16F:
            return {GL_RGBA16F, GL_RGBA, GL_FLOAT};
        case FramebufferFormat::RGBA32F:
            return {GL_RGBA32F, GL_RGBA, GL_FLOAT};
        default:
            RG_SHOULD_NOT_REACH_HERE("Invalid color attachment format");
    }
}

GLint texture_filter_to_opengl(TextureFilter filter) {
    switch (filter) {
        case TextureFilter::Nearest: return GL_NEAREST;
        case TextureFilter::Linear: return GL_LINEAR;
    }
    RG_SHOULD_NOT_REACH_HERE("Unhandled texture filter");
}

GLint texture_wrap_to_opengl(TextureWrap wrap) {
    switch (wrap) {
        case TextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
        case TextureWrap::Repeat: return GL_REPEAT;
    }
    RG_SHOULD_NOT_REACH_HERE("Unhandled texture wrap");
}
}// namespace

Framebuffer::Framebuffer(int width, int height)
    : Framebuffer(FramebufferDescription{.width = width, .height = height}) {
}

Framebuffer::Framebuffer(FramebufferDescription description)
    : m_description(std::move(description)) {
    validate_description();

    CHECKED_GL_CALL(glGenFramebuffers, 1, &m_fbo);
    m_color_textures.resize(m_description.color_formats.size());
    if (!m_color_textures.empty()) {
        CHECKED_GL_CALL(glGenTextures, static_cast<GLsizei>(m_color_textures.size()), m_color_textures.data());
    }

    if (m_description.depth_format.has_value()) {
        if (m_description.depth_format == FramebufferFormat::Depth32F) {
            CHECKED_GL_CALL(glGenTextures, 1, &m_depth_texture);
        } else {
            CHECKED_GL_CALL(glGenRenderbuffers, 1, &m_depth_stencil_rbo);
        }
    }

    allocate_attachments();
    bind_default();
}

Framebuffer::~Framebuffer() {
    if (m_depth_stencil_rbo != 0) {
        CHECKED_GL_CALL(glDeleteRenderbuffers, 1, &m_depth_stencil_rbo);
    }
    if (m_depth_texture != 0) {
        CHECKED_GL_CALL(glDeleteTextures, 1, &m_depth_texture);
    }
    if (!m_color_textures.empty()) {
        CHECKED_GL_CALL(glDeleteTextures, static_cast<GLsizei>(m_color_textures.size()), m_color_textures.data());
    }
    if (m_fbo != 0) {
        CHECKED_GL_CALL(glDeleteFramebuffers, 1, &m_fbo);
    }
}

void Framebuffer::validate_description() const {
    RG_GUARANTEE(m_description.width > 0 && m_description.height > 0,
                 "Framebuffer dimensions must be positive");
    RG_GUARANTEE(m_description.color_formats.size() <= 16,
                 "Framebuffer has too many color attachments");

    for (const auto format: m_description.color_formats) {
        RG_GUARANTEE(format == FramebufferFormat::RGBA8 || format == FramebufferFormat::RGBA16F ||
                             format == FramebufferFormat::RGBA32F,
                     "Invalid color attachment format");
    }

    if (m_description.depth_format.has_value()) {
        const auto format = *m_description.depth_format;
        RG_GUARANTEE(format == FramebufferFormat::Depth24Stencil8 || format == FramebufferFormat::Depth32F,
                     "Invalid depth attachment format");
    }

    if (m_description.depth_cubemap) {
        RG_GUARANTEE(m_description.depth_format == FramebufferFormat::Depth32F,
                     "A depth cubemap attachment requires the FramebufferFormat::Depth32F format");
        RG_GUARANTEE(m_description.width == m_description.height,
                     "A depth cubemap attachment requires square faces, got {}x{}",
                     m_description.width, m_description.height);
        RG_GUARANTEE(m_description.depth_wrap != TextureWrap::ClampToBorder,
                     "A depth cubemap attachment can't be sampled with TextureWrap::ClampToBorder");
    }
}

void Framebuffer::allocate_attachments() {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_fbo);

    std::vector<GLenum> draw_buffers;
    draw_buffers.reserve(m_color_textures.size());

    for (size_t index = 0; index < m_color_textures.size(); ++index) {
        const auto info = color_format_info(m_description.color_formats[index]);
        const auto texture = m_color_textures[index];

        CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, texture);
        CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, info.internal_format, m_description.width,
                        m_description.height, 0, info.format, info.type, nullptr);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        CHECKED_GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, texture,
                        0);
        draw_buffers.push_back(GL_COLOR_ATTACHMENT0 + index);
    }

    if (draw_buffers.empty()) {
        CHECKED_GL_CALL(glDrawBuffer, GL_NONE);
        CHECKED_GL_CALL(glReadBuffer, GL_NONE);
    } else {
        CHECKED_GL_CALL(glDrawBuffers, static_cast<GLsizei>(draw_buffers.size()), draw_buffers.data());
    }

    if (m_description.depth_format == FramebufferFormat::Depth24Stencil8) {
        CHECKED_GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, m_depth_stencil_rbo);
        CHECKED_GL_CALL(glRenderbufferStorage, GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_description.width,
                        m_description.height);
        CHECKED_GL_CALL(glFramebufferRenderbuffer, GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                        m_depth_stencil_rbo);
    } else if (m_description.depth_format == FramebufferFormat::Depth32F) {
        const auto target = m_description.depth_cubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
        CHECKED_GL_CALL(glBindTexture, target, m_depth_texture);
        if (m_description.depth_cubemap) {
            for (int face = 0; face < 6; ++face) {
                CHECKED_GL_CALL(glTexImage2D, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_DEPTH_COMPONENT32F,
                                m_description.width, m_description.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            }
            CHECKED_GL_CALL(glTexParameteri, target, GL_TEXTURE_WRAP_R,
                            texture_wrap_to_opengl(m_description.depth_wrap));
            CHECKED_GL_CALL(glFramebufferTexture, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depth_texture, 0);
        } else {
            CHECKED_GL_CALL(glTexImage2D, target, 0, GL_DEPTH_COMPONENT32F, m_description.width,
                            m_description.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            CHECKED_GL_CALL(glTexParameteri, target, GL_TEXTURE_WRAP_S,
                            texture_wrap_to_opengl(m_description.depth_wrap));
            CHECKED_GL_CALL(glTexParameteri, target, GL_TEXTURE_WRAP_T,
                            texture_wrap_to_opengl(m_description.depth_wrap));
            CHECKED_GL_CALL(glTexParameterfv, target, GL_TEXTURE_BORDER_COLOR,
                            m_description.depth_border_color.data());
            CHECKED_GL_CALL(glFramebufferTexture2D, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, m_depth_texture, 0);
        }
        const auto filter = texture_filter_to_opengl(m_description.depth_filter);
        CHECKED_GL_CALL(glTexParameteri, target, GL_TEXTURE_MIN_FILTER, filter);
        CHECKED_GL_CALL(glTexParameteri, target, GL_TEXTURE_MAG_FILTER, filter);
    }

    const auto status = CHECKED_GL_CALL(glCheckFramebufferStatus, GL_FRAMEBUFFER);
    RG_GUARANTEE(status == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete");

    CHECKED_GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, 0);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_2D, 0);
    CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, 0);
}

void Framebuffer::bind() const {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, m_fbo);
}

void Framebuffer::bind_default() {
    CHECKED_GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
}

void Framebuffer::resize(int width, int height) {
    RG_GUARANTEE(width > 0 && height > 0, "Framebuffer dimensions must be positive");
    m_description.width = width;
    m_description.height = height;
    allocate_attachments();
    bind_default();
}

uint32_t Framebuffer::color_texture(std::size_t index) const {
    RG_GUARANTEE(index < m_color_textures.size(), "Color attachment index out of bounds");
    return m_color_textures[index];
}

uint32_t Framebuffer::depth_texture() const {
    RG_GUARANTEE(m_depth_texture != 0, "Framebuffer does not have a depth texture");
    return m_depth_texture;
}

uint32_t Framebuffer::depth_texture_target() const {
    RG_GUARANTEE(m_depth_texture != 0, "Framebuffer does not have a depth texture");
    return m_description.depth_cubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
}
}// namespace engine::graphics
