#include <System.hpp>
#include <User.hpp>
#include <Renderer.hpp>
#include <UI.hpp>

#include <filesystem>
#include <windows.h>


string System::PATH;

sf::Clock System::timeClock;
float System::deltaTime;

std::vector<string> System::assets;


void System::init() {
  char buffer[MAX_PATH]; GetModuleFileNameA(NULL, buffer, MAX_PATH);
  PATH = std::filesystem::path(buffer).parent_path().parent_path().string();

  Renderer::init();

  UI::Manager::init(PATH, Renderer::window, true);

  GenerateReleaseAssets();
}
void System::update() {
  deltaTime = timeClock.restart().asMilliseconds() * .001f;

  User::update();
  Renderer::update();
  UI::Manager::update();
}