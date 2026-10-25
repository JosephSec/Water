#include <System.hpp>
#include <User.hpp>

#include <Renderer/Core.hpp>
#include <Renderer/Core/Shader.hpp>
#include <Renderer/Core/Mesh.hpp>
#include <Renderer/Core/Texture.hpp>
#include <Renderer/Core/Skybox.hpp>
#include <Renderer/Component/Camera.hpp>
#include <Renderer/Hardware/GPU.hpp>
#include <Renderer/Hardware/Monitor.hpp>

#include <Editor/UI/PerformanceOverlay.hpp>

#include <random>
#include <chrono>

#include <iostream>


static GLuint fbo, vao;
static Renderer::Texture colorTex, depthTex;
static glm::ivec2 postSize;
static void resizePost(const glm::ivec2 _size) {
  postSize = _size;

  colorTex = Renderer::Texture(postSize, GL_RGBA8, 1);
  depthTex = Renderer::Texture(postSize, GL_DEPTH_COMPONENT24, 1);

  glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, colorTex.getID(), 0);
  glNamedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT,  depthTex.getID(), 0);
  if(glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "[POST PROCESS ERROR]: Post process framebuffer is incomplete\n";
  }
}
static void initPost(const glm::ivec2 _size) {
  glCreateFramebuffers(1, &fbo);
  glCreateVertexArrays(1, &vao);
  
  resizePost(_size);
}
static void beginPost() {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glViewport(0,0, postSize.x,postSize.y);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
static void endPost(Renderer::Shader *_shader, Renderer::Camera *_camera, float _t) {
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0,0, Renderer::Core::windowSize.x, Renderer::Core::windowSize.y);
  glDisable(GL_DEPTH_TEST);

  colorTex.bindUnit(1);
  depthTex.bindUnit(2);

  _shader->use();
  _shader->setUniform("uColor", 1);
  _shader->setUniform("uDepth", 2);
  _shader->setUniform("uViewSize", glm::vec2{Renderer::Core::windowSize});
  _shader->setUniform("uCameraWorldPos", _camera->transform.position);
  _shader->setUniform("uTime", _t);

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);

  glEnable(GL_DEPTH_TEST);
}


static Renderer::Skybox skybox;

static bool wireframeMode = false;
static bool paused = false;
static bool doPost = false;
int main(int argc, char *argv[]) {
  System::init();
  Renderer::Core::init(true);
  Renderer::Core::window->setActive(true);
  initPost(Renderer::Core::windowSize);
  Editor::UI::PerformanceOverlay::init();

  sf::Font font(System::PATH+"/assets/Roboto.ttf");

  Renderer::Shader unlitShader("Unlit");
  Renderer::Shader postShader("PostProcess");
  Renderer::Shader skyboxShader("Skybox");
  Renderer::Shader waterShader("Water");

  Renderer::Camera *camera = new Renderer::Camera();
  camera->farPlane = 100000;

  skybox();
  skybox.loadFromFolder(System::PATH+"/assets/skyboxs/StormyNight");


  glm::ivec2 size;
  float spacing = 0;
  static glm::vec3 sunDir = glm::vec3{1,.08f,-1};
  
  { //Get Water Params
    std::string str;
    std::cout << "Enter Water Size: ";
    std::getline(std::cin, str);
    size = glm::ivec2(std::stoi(str));

    std::cout << "Enter Water Spacing: ";
    std::getline(std::cin, str);
    spacing = std::stof(str);

    system("cls");
  } //Get Water Params

  Renderer::Mesh water; {
    water();

    std::vector<Renderer::Mesh::Vertex> vertices((size.x + 1) * (size.y + 1));
    for(int y = 0; y <= size.y; y++) {
      for(int x = 0; x <= size.x; x++) {
        vertices[x + y * (size.x + 1)] = Renderer::Mesh::Vertex{
          glm::vec3{x - static_cast<float>(size.x + 1) / 2.0f, 0, -(y - static_cast<float>(size.y + 1) / 2.0f)} * spacing,
          {0,0,1,1},
        };
      }
    }
    water.setVertices(vertices, GL_STATIC_DRAW, false);

    std::vector<Renderer::Mesh::Index> indices(size.x * size.y * 6);
    for(int y = 0; y < size.y; y++) {
      for(int x = 0; x < size.x; x++) {
        const uint64_t start = (static_cast<uint64_t>(x) + static_cast<uint64_t>(y) * size.x) * 6;

        const Renderer::Mesh::Index a = x + y * (size.x + 1);
        const Renderer::Mesh::Index b = a + size.x + 1;
        const Renderer::Mesh::Index c = b + 1;
        const Renderer::Mesh::Index d = a + 1;

        indices[start + 0] = a;
        indices[start + 1] = b;
        indices[start + 2] = c;

        indices[start + 3] = a;
        indices[start + 4] = c;
        indices[start + 5] = d;
      }
    }
    water.setIndices(indices, GL_STATIC_DRAW, false);
  }


  std::cout << "Active Monitor Refresh Rate: " << Renderer::Monitor::GetCurrentMonitorRefreshRate() << '\n';
  std::cout << "GPU: " << Renderer::GPU::GetGPUName() << " | Available VRAM: " << Renderer::GPU::VRAMCapacity / (1024 * 1024) << "MB\n";
  std::cout << "Used VRAM: " << Renderer::GPU::VRAMInUse / 1000 << "KB | " << Renderer::GPU::GetUsedVRAM() << "%\n";
  while(Renderer::Core::window->isOpen()) {
    while(const auto &eventOpt = Renderer::Core::window->pollEvent()) {
      const auto &event = *eventOpt;

      if(event.is<sf::Event::Closed>()) Renderer::Core::window->close();
      else if(const auto *resized = event.getIf<sf::Event::Resized>()) {
        Renderer::Core::UpdateWindowSize();
        resizePost(Renderer::Core::windowSize);
      }
      else if(event.is<sf::Event::FocusLost>()) User::HandleFocusLost();

      else if(const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        User::HandleKeyPressed(keyPressed);
        if(keyPressed->code == sf::Keyboard::Key::Space) paused = !paused;
        else if(keyPressed->code == sf::Keyboard::Key::F1) Editor::UI::PerformanceOverlay::ToggleEnabled();
        else if(keyPressed->code == sf::Keyboard::Key::F2) {
          wireframeMode = !wireframeMode;
          glPolygonMode(GL_FRONT_AND_BACK, wireframeMode? GL_LINE : GL_FILL);
        }
        else if(keyPressed->code == sf::Keyboard::Key::F3) doPost = !doPost;

        else if(keyPressed->code == sf::Keyboard::Key::X) {
          std::stringstream ss;
          ss << std::format("Sun Direction: ({}, {}, {})", sunDir.x,sunDir.y,sunDir.z) << '\n';
          std::cout << ss.str();
        }
      }
      else if(const auto *mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) User::HandleMouseButtonPressed(mouseButtonPressed);
      else if(const auto *mouseButtonReleased = event.getIf<sf::Event::MouseButtonReleased>()) User::HandleMouseButtonReleased(mouseButtonReleased);
    }

    static float t = 0;
    if(paused == false) t += System::deltaTime;
    // sunDir = glm::vec3{glm::cos(t), .6f, glm::sin(t)};

    { //Update
      System::update();

      { //Check if window moved
        static sf::Vector2i windowLastPosition = Renderer::Core::window->getPosition();
        const sf::Vector2i windowCurrentPosition = Renderer::Core::window->getPosition();
        if(windowCurrentPosition != windowLastPosition) {
          windowLastPosition = windowCurrentPosition;
          Renderer::Monitor::RefreshRate = Renderer::Monitor::GetCurrentMonitorRefreshRate();
          Editor::UI::PerformanceOverlay::fpsGraph.max = Renderer::Monitor::RefreshRate;
        }
      } //Check if window moved

      if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) { //Camera Control
        static constexpr float slowModeSpeed = 5;
        static constexpr float defaultModeSpeed = 25;
        static constexpr float fastModeSpeed = 50;

        glm::vec3 moveDir = glm::vec3(0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) moveDir -= glm::vec3(0,0,1);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) moveDir -= glm::vec3(1,0,0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) moveDir += glm::vec3(0,0,1);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) moveDir += glm::vec3(1,0,0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) moveDir -= glm::vec3(0,1,0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) moveDir += glm::vec3(0,1,0);
        if(glm::length(moveDir) != 0) {
          float speed = System::deltaTime;
          if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) speed *= fastModeSpeed;
          else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) speed *= slowModeSpeed;
          else speed *= defaultModeSpeed;

          camera->transform.position += (camera->transform.rotation * glm::normalize(moveDir)) * speed;
        }

        if(glm::length(glm::vec2(User::Cursor::delta)) != 0) {
          const glm::vec2 lookDelta = -glm::vec2(User::Cursor::delta) * System::deltaTime;

          const glm::quat pitch = glm::angleAxis(lookDelta.y, glm::vec3(1,0,0));
          const glm::quat yaw = glm::angleAxis(lookDelta.x, glm::vec3(0,1,0));
          camera->transform.rotation = glm::normalize(yaw * camera->transform.rotation * pitch);
        }
      } //Camera Control

      Editor::UI::PerformanceOverlay::update(System::deltaTime);

      const glm::mat4x4 projMat = camera->getProjectionMatrix(Renderer::Core::windowSize);
      const glm::mat4x4 viewMat = camera->getViewMatrix();

      unlitShader.use();
      unlitShader.setUniform("uProj", projMat);
      unlitShader.setUniform("uView", viewMat);
      waterShader.use();
      waterShader.setUniform("uProj", projMat);
      waterShader.setUniform("uView", viewMat);
      postShader.use();
      postShader.setUniform("uProj", projMat);
      postShader.setUniform("uView", viewMat);
      skyboxShader.use();
      skyboxShader.setUniform("uProj", projMat);
      skyboxShader.setUniform("uView", viewMat);

      User::update();
    } //Update

    { //Render
      Renderer::Core::clear();

      { //Scene
        { //GL Init
          glEnable(GL_DEPTH_TEST);
          glDepthMask(GL_TRUE);
          glDepthFunc(GL_LESS);
          // glEnable(GL_CULL_FACE);
          glDisable(GL_CULL_FACE);
          glCullFace(GL_BACK);
          glFrontFace(GL_CW);
          glDisable(GL_BLEND);

          glPolygonMode(GL_FRONT_AND_BACK, wireframeMode? GL_LINE : GL_FILL);
        } //GL Init

        if(doPost) beginPost();
        skybox.draw(&skyboxShader, camera);
        { //Water
          waterShader.use();
          waterShader.setUniform("uModel", Renderer::Transform({0,-3,0}).getModelMatrix());

          waterShader.setUniform("uCamPos", camera->transform.position);
          waterShader.setUniform("uSunDir", glm::normalize(sunDir));
          waterShader.setUniform("uSunColor", glm::vec3{.75f, .82f, 1.0f} * 1.5f);

          // Wave shape
          waterShader.setUniform("uWindSpeed", 30.0f);       // shader clamps to 2..30, so 200 was acting as 30
          waterShader.setUniform("uAmplitudeScale", 3.5f);   // the real "wave height" knob, linear multiplier
          waterShader.setUniform("uChoppiness", 2.2f);       // sharper crests, pinched troughs
          waterShader.setUniform("uWindDir", glm::vec2{1,0.6f});

          // Whitecaps
          waterShader.setUniform("uFoamThreshold", .45f);    // lower = more foam
          waterShader.setUniform("uFoamIntensity", 1.3f);

          waterShader.setUniform("uFogColor", glm::vec4{.68f,.74f,.78f,1});
          waterShader.setUniform("uFogDensity", .1f);

          waterShader.setUniform("uMeshSpacing", spacing);
          waterShader.setUniform("uSky", 1);
          waterShader.setUniform("uTime", t);

          skybox.bindUnit(1);

          water.use();
          water.draw();
        } //Water
        if(doPost) endPost(&postShader, camera, t);
      } //Scene
      { //UI
        { //GL Init
          glBindVertexArray(0);
          glBindBuffer(GL_ARRAY_BUFFER, 0);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
          glUseProgram(0);

          glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

          Renderer::Core::window->pushGLStates();
        } //GL Init


        Editor::UI::PerformanceOverlay::draw(font);

        Renderer::Core::window->popGLStates();
      } //UI



      Renderer::Core::display();
    } //Render
  }

  Renderer::Core::end();

  return 0;
}