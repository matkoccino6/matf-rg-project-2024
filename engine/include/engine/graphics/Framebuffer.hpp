#ifndef MATF_RG_PROJECT_FRAMEBUFFER_HPP
#define MATF_RG_PROJECT_FRAMEBUFFER_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace engine::graphics {
enum class FramebufferFormat {
    RGBA8,
    RGBA16F,
    RGBA32F,
    Depth24Stencil8,
    Depth32F
};

struct FramebufferDescription {
    int width{};
    int height{};
    std::vector<FramebufferFormat> color_formats{FramebufferFormat::RGBA8};
    std::optional<FramebufferFormat> depth_format{FramebufferFormat::Depth24Stencil8};
};

class Framebuffer {
public:
    Framebuffer(int width, int height);
    explicit Framebuffer(FramebufferDescription description);

    ~Framebuffer();

    Framebuffer(const Framebuffer &) = delete;
    Framebuffer &operator=(const Framebuffer &) = delete;

    void bind() const;
    static void bind_default();

    void resize(int width, int height);

    uint32_t color_texture(std::size_t index = 0) const;
    uint32_t depth_texture() const;

    int width() const {
        return m_description.width;
    }

    int height() const {
        return m_description.height;
    }

private:
    void allocate_attachments();
    void validate_description() const;

    FramebufferDescription m_description;
    uint32_t m_fbo{};
    std::vector<uint32_t> m_color_textures;
    uint32_t m_depth_stencil_rbo{};
    uint32_t m_depth_texture{};
};
}// namespace engine::graphics

#endif//MATF_RG_PROJECT_FRAMEBUFFER_HPP
