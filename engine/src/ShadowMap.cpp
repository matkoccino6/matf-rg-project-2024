#include <engine/graphics/Framebuffer.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/ShadowMap.hpp>
#include <engine/util/Errors.hpp>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <utility>

namespace engine::graphics {
ShadowMap::ShadowMap(ShadowMapDescription description)
    : m_description(description) {
    validate_description();
    m_buffer = std::make_unique<Framebuffer>(FramebufferDescription{
            .width = m_description.resolution,
            .height = m_description.resolution,
            .color_formats = {},
            .depth_format = FramebufferFormat::Depth32F,
            .depth_cubemap = m_description.type == ShadowMapType::Point,
            .depth_filter = TextureFilter::Linear,
            .depth_wrap = m_description.type == ShadowMapType::Point
                                  ? TextureWrap::ClampToEdge
                                  : TextureWrap::ClampToBorder});
    update_matrices();
}

ShadowMap::ShadowMap(const int resolution, const ShadowMapType type)
    : ShadowMap(ShadowMapDescription{.type = type, .resolution = resolution}) {
}

ShadowMap::~ShadowMap() = default;

void ShadowMap::validate_description() const {
    RG_GUARANTEE(m_description.resolution > 0, "Shadow map resolution must be positive");
    RG_GUARANTEE(m_description.near_plane > 0.0f && m_description.far_plane > m_description.near_plane,
                 "Shadow map clip planes are invalid");
    RG_GUARANTEE(m_description.field_of_view > 0.0f && m_description.field_of_view < 180.0f,
                 "Shadow map field of view is invalid");
    RG_GUARANTEE(m_description.left < m_description.right && m_description.bottom < m_description.top,
                 "Shadow map orthographic bounds are invalid");
}

void ShadowMap::update_matrices() {
    if (m_description.type == ShadowMapType::Basic) {
        const auto view = glm::lookAt(m_description.light_position, m_description.target, m_description.up);
        const auto projection = glm::ortho(m_description.left, m_description.right, m_description.bottom,
                                           m_description.top, m_description.near_plane, m_description.far_plane);
        m_light_view_projection = projection * view;
        return;
    }

    if (m_description.type == ShadowMapType::Spot) {
        const auto view = glm::lookAt(m_description.light_position, m_description.target, m_description.up);
        const auto projection = glm::perspective(glm::radians(m_description.field_of_view), 1.0f,
                                                 m_description.near_plane, m_description.far_plane);
        m_light_view_projection = projection * view;
        return;
    }

    const auto projection = glm::perspective(glm::radians(90.0f), 1.0f, m_description.near_plane,
                                             m_description.far_plane);
    constexpr std::array<glm::vec3, 6> directions{
            glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0),
            glm::vec3(0, -1, 0), glm::vec3(0, 0, 1), glm::vec3(0, 0, -1)};
    constexpr std::array<glm::vec3, 6> ups{
            glm::vec3(0, -1, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, 1),
            glm::vec3(0, 0, -1), glm::vec3(0, -1, 0), glm::vec3(0, -1, 0)};
    for (std::size_t face = 0; face < directions.size(); ++face) {
        m_point_view_projections[face] =
                projection * glm::lookAt(m_description.light_position,
                                         m_description.light_position + directions[face], ups[face]);
    }
}

void ShadowMap::begin_render() const {
    m_buffer->bind();
    OpenGL::set_viewport(m_description.resolution, m_description.resolution);
    CHECKED_GL_CALL(glClear, GL_DEPTH_BUFFER_BIT);
    CHECKED_GL_CALL(glEnable, GL_CULL_FACE);
    CHECKED_GL_CALL(glCullFace, GL_FRONT);
}

void ShadowMap::end_render() {
    CHECKED_GL_CALL(glCullFace, GL_BACK);
    Framebuffer::bind_default();
}

void ShadowMap::resize(const int resolution) {
    RG_GUARANTEE(resolution > 0, "Shadow map resolution must be positive");
    m_description.resolution = resolution;
    m_buffer->resize(resolution, resolution);
}

void ShadowMap::set_light(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up) {
    m_description.light_position = position;
    m_description.target = target;
    m_description.up = up;
    update_matrices();
}

void ShadowMap::set_clip_planes(const float near_plane, const float far_plane) {
    RG_GUARANTEE(near_plane > 0.0f && far_plane > near_plane, "Shadow map clip planes are invalid");
    m_description.near_plane = near_plane;
    m_description.far_plane = far_plane;
    update_matrices();
}

void ShadowMap::set_orthographic_bounds(const float left, const float right, const float bottom, const float top) {
    RG_GUARANTEE(left < right && bottom < top, "Shadow map orthographic bounds are invalid");
    m_description.left = left;
    m_description.right = right;
    m_description.bottom = bottom;
    m_description.top = top;
    update_matrices();
}

void ShadowMap::set_field_of_view(const float field_of_view) {
    RG_GUARANTEE(field_of_view > 0.0f && field_of_view < 180.0f,
                 "Shadow map field of view is invalid");
    m_description.field_of_view = field_of_view;
    update_matrices();
}

uint32_t ShadowMap::depth_texture() const {
    return m_buffer->depth_texture();
}

uint32_t ShadowMap::texture_target() const {
    return m_buffer->depth_texture_target();
}

void ShadowMap::bind_texture(const unsigned texture_unit) const {
    CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0 + texture_unit);
    CHECKED_GL_CALL(glBindTexture, texture_target(), depth_texture());
}
}// namespace engine::graphics
