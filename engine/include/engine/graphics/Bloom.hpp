#ifndef MATF_RG_PROJECT_BLOOM_HPP
#define MATF_RG_PROJECT_BLOOM_HPP

#include <memory>
#include <vector>

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

    void initialize(int width, int height, resources::Shader *composite_shader,
                    resources::Shader *downsample_shader, resources::Shader *upsample_shader,
                    int mip_levels = 13);

    void terminate();

    void begin_draw();

    void apply(float exposure, int mip_levels, float bloom_filter_radius, float bloom_intensity);

    void resize(int width, int height);

private:
    std::unique_ptr<Framebuffer> m_scene_buffer;
    std::vector<std::unique_ptr<Framebuffer>> m_mip_buffers;
    resources::Shader *m_composite_shader{};
    resources::Shader *m_downsample_shader{};
    resources::Shader *m_upsample_shader{};
};
}// namespace engine::graphics

#endif// MATF_RG_PROJECT_BLOOM_HPP
