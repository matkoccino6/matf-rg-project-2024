#include <BlackLodgeApp.hpp>
#include <GUIController.hpp>
#include <MainController.hpp>
#include <SettingsController.hpp>
#include <spdlog/spdlog.h>

namespace engine::black_lodge::app {
void BlackLodgeApp::app_setup() {
    spdlog::info("Black Lodge App setup");

    auto main_controller = register_controller<MainController>();
    auto gui_controller = register_controller<GUIController>();
    auto settings_controller = register_controller<SettingsController>();
    main_controller->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
    gui_controller->after(main_controller);
    settings_controller->after(gui_controller);
}
}// namespace engine::blackLodge::app

int main(int argc, char **argv) {
    return std::make_unique<engine::black_lodge::app::BlackLodgeApp>()->run(argc, argv);
}
