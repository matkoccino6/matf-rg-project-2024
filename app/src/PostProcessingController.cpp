#include <PostProcessingController.hpp>
#include <SettingsController.hpp>
#include <engine/core/Engine.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <memory>

namespace engine::black_lodge::app {
void PostProcessingController::initialize() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();

    m_bloom.initialize(platform->window()->width(),
                       platform->window()->height(),
                       resources->shader("post_processing"),
                       resources->shader("bloom_downsample"),
                       resources->shader("bloom_upsample"));
    platform->register_platform_event_observer(std::make_unique<ResizeObserver>(this));
}

void PostProcessingController::begin_draw() {
    m_bloom.begin_draw();
}

void PostProcessingController::draw() {
    auto settings = get<SettingsController>();
    m_bloom.apply(settings->u_exposure, settings->u_bloom_mip_levels, settings->u_bloom_filter_radius,
                  settings->u_bloom_intensity);
}

void PostProcessingController::terminate() {
    m_bloom.terminate();
}

void PostProcessingController::ResizeObserver::on_window_resize(int width, int height) {
    m_controller->m_bloom.resize(width, height);
}
}// namespace engine::black_lodge::app
