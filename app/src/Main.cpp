#include <BlackLodgeApp.hpp>
#include <spdlog/spdlog.h>
#include <MainController.hpp>
#include <GUIController.hpp>

namespace engine::blackLodge::app {
    void BlackLodgeApp::app_setup() {
        spdlog::info("Black Lodge App setup");

        auto main_controller = register_controller<MainController>();
        auto gui_controller  = register_controller<GUIController>();
        main_controller->after(engine::core::Controller::get<engine::core::EngineControllersEnd>());
        gui_controller->after(main_controller);
    }
}

int main(int argc, char **argv) {
    return std::make_unique<engine::blackLodge::app::BlackLodgeApp>()->run(argc, argv);
}
