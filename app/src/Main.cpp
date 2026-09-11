#include <BlackLodgeApp.hpp>
#include <GUIController.hpp>
#include <MainController.hpp>
#include <PostProcessingController.hpp>
#include <SettingsController.hpp>
#include <ShadowController.hpp>
#include <spdlog/spdlog.h>

namespace engine::black_lodge::app {
void BlackLodgeApp::app_setup() {
    spdlog::info("Black Lodge App setup");

    auto main_controller = register_controller<MainController>();
    auto post_processing_controller = register_controller<PostProcessingController>();
    auto gui_controller = register_controller<GUIController>();
    auto settings_controller = register_controller<SettingsController>();
    auto shadow_controller = register_controller<ShadowController>();
    main_controller->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
    shadow_controller->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
    shadow_controller->before(main_controller);
    post_processing_controller->after(main_controller);
    gui_controller->after(post_processing_controller);
    settings_controller->after(gui_controller);
}
}// namespace engine::black_lodge::app

int main(int argc, char **argv) {
    return std::make_unique<engine::black_lodge::app::BlackLodgeApp>()->run(argc, argv);
}
