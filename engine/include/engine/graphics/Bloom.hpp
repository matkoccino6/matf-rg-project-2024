#ifndef MATF_RG_PROJECT_BLOOM_HPP
#define MATF_RG_PROJECT_BLOOM_HPP

#include <array>
#include <memory>

namespace engine::graphics {
class Framebuffer;
}

namespace engine::resources {
class Shader;
}

namespace engine::graphics {
/**
 * @class Bloom
 * @brief Owns the HDR scene buffer and performs bloom blur and compositing.
 *
 * The application renders its scene between @ref begin_draw and @ref apply.
 * The scene framebuffer has two color attachments: the first contains the
 * rendered scene and the second contains the bright fragments used for bloom.
 */
class Bloom {
public:
    Bloom() = default;
    ~Bloom();

    Bloom(const Bloom &) = delete;
    Bloom &operator=(const Bloom &) = delete;

    void initialize(int width, int height, resources::Shader *composite_shader, resources::Shader *blur_shader);
    void terminate();

    void begin_draw();
    void apply(float exposure, int blur_passes);
    void resize(int width, int height);

private:
    std::unique_ptr<Framebuffer> m_scene_buffer;
    std::array<std::unique_ptr<Framebuffer>, 2> m_blur_buffers;
    resources::Shader *m_composite_shader{};
    resources::Shader *m_blur_shader{};
};
}// namespace engine::graphics

#endif// MATF_RG_PROJECT_BLOOM_HPP
