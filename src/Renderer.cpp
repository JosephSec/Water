#include <Renderer.hpp>
#include <System.hpp>
#include <User.hpp>
#include <UI.hpp>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include <System/Rect.hpp>
#include <System/PerlinGenerator.hpp>

#include <map>


sf::RenderWindow* Renderer::window;
Vec2 Renderer::windowSize;
Vec2 Renderer::windowCenter;

sf::Font Renderer::font;
sf::Text Renderer::TextPrefab(font);


float Renderer::aspectRatio = 800.f / 600.f;
float Renderer::nearPlane = 0.1f;
float Renderer::farPlane = 1000.f;
float Renderer::fov = 60.f * deg2rad;
float Renderer::f = 1.0f / std::tan(fov / 2.f);

float Renderer::projection[16];
float Renderer::view[16];

Transform Renderer::camera(Vec3(-15,13,-30), Quaternion::Euler(30,25,0));

Shader Renderer::TriangleShader;
Shader Renderer::LineShader;

Water Renderer::water;

bool Renderer::WireframeView;
Vec2 Renderer::wind = Vec2one*5;
float Renderer::WaterUpdateTime;


static sf::Texture texture;
static sf::Sprite sprite(texture);


    void Water::update() {
      PerlinGenerator::Generate(perlin);
      
      sf::Image image = PerlinGenerator::GenerateImage(perlin, Color(0,0,150), Color(30,30,255));
      // if(!image.saveToFile(System::PATH+"/assets/Perlin.png"))
      //   Debug::error("couldn't save perlin data to png");
      
      if(!texture.loadFromImage(image))
        Debug::error("Cant generate image from perlin noise");
        
      sprite.setTexture(texture, true);
      sprite.setScale(Vec2one / (texture.getSize().x / 150.0f));
      sprite.setPosition(Vec2up * (Renderer::windowSize.y - 150));
      
      for(int i = 0; i < mesh.vertices.size(); i++) {
        mesh.vertices[i].y = perlin.data[i] * waveSize;
      }
    }


void Renderer::init() {
  sf::ContextSettings settings;
  settings.depthBits = 24;
  settings.stencilBits = 8;
  settings.antiAliasingLevel = 4;
  settings.majorVersion = 3;
  settings.minorVersion = 3;
  settings.attributeFlags = sf::ContextSettings::Default;

  window = new sf::RenderWindow(sf::VideoMode(Vec2(800,600)), "Window", sf::State::Windowed, settings);
  window->setFramerateLimit(120);
  window->requestFocus();

  initGL();
  UpdateViewMatrix();
  HandleResize();
  update();

  if(!font.openFromFile(System::PATH+"/assets/Roboto.ttf"))
    Debug::error("font file missing or can't be loaded");
  else System::assets.push_back("Roboto.ttf");
  
  TextPrefab.setFont(font);
  TextPrefab.setString("[EMPTY STRING]");


  TriangleShader = Shader("Triangle");
  LineShader = Shader("Line");
  
  Mesh::init();
  Line::init();


  const Vec2 gridSize = Vec2one*50;
  const Vec3 gridHalf = Vec3((gridSize.x+1) / 2.0f, 0, (gridSize.y+1) / 2.0f);

  water.perlin.size = gridSize.x + 1;
  water.perlin.lowWeight = 2.5f;
  water.perlin.scale = .1f;
  water.waveSize = 2.5f;

  for(int y = 0; y <= gridSize.y; y++) {
    for(int x = 0; x <= gridSize.x; x++) {
      water.mesh.vertices.push_back(Vec3(x,0,y) - gridHalf);
    }
  }

  for(int y = 0; y < gridSize.y; y++) {
    for(int x = 0; x < gridSize.x; x++) {
      const unsigned int start = x + (y * (gridSize.x + 1));

      water.mesh.indecies.push_back(start);
      water.mesh.indecies.push_back(start + gridSize.x + 1);
      water.mesh.indecies.push_back(start + 1 + gridSize.x + 1);
      
      water.mesh.indecies.push_back(start);
      water.mesh.indecies.push_back(start + 1 + gridSize.x + 1);
      water.mesh.indecies.push_back(start + 1);
    }
  }

  for(const int& tri : water.mesh.indecies) {
    const Vec3 pos = water.mesh.vertices[tri] + gridHalf;
    water.mesh.uvs.push_back(Vec2(pos.x / (gridSize.x+1), pos.z / (gridSize.y+1)));
  }

  water.mesh.textureHandle = texture.getNativeHandle();


  for(int i = 0; i < water.mesh.vertices.size(); i++) {
    water.mesh.vertices[i].y = water.perlin.data[i] * 2.5f;
  }
  
  water.wireframe = Mesh::GenerateWireframe(water.mesh);
}
void Renderer::initGL() {
  GLenum err = glewInit();
  if(err != GLEW_OK) std::cerr << "GLEW init error: " << glewGetErrorString(err) << '\n';

  glEnable(GL_DEPTH_TEST);

  glCullFace(GL_BACK);
  glFrontFace(GL_CW);

  glClearColor(.1f,.1f,.15f, 1);
}
void Renderer::update() {
  windowSize = window->getSize();
  windowCenter = windowSize/2;

  if(window->hasFocus()) {
    if(User::GetKeyDown(Keyboard::Key::X)) {
      Debug::log("position", camera.position);
      Debug::log("rotation", camera.rotation.getEuler());
    }

    if(User::GetMouseButton(Mouse::Button::Right)) {
      static Vec2 mouseAnchor;
      static Vec2 mouseDelta;
  
      if(User::GetMouseButtonDown(Mouse::Button::Right)) mouseAnchor = User::mousePos;
  
      mouseDelta = User::mousePos-mouseAnchor;
      Mouse::setPosition(mouseAnchor, *Renderer::window);
  
  
      static const float moveSpeed = 8;
      static const float quickSpeed = 15;
      Vec3 moveDir;
      if(User::GetKey(Keyboard::Key::W)) moveDir += Vec3forward;
      if(User::GetKey(Keyboard::Key::S)) moveDir -= Vec3forward;
      if(User::GetKey(Keyboard::Key::A)) moveDir -= Vec3right;
      if(User::GetKey(Keyboard::Key::D)) moveDir += Vec3right;
      if(User::GetKey(Keyboard::Key::Q)) moveDir -= Vec3up;
      if(User::GetKey(Keyboard::Key::E)) moveDir += Vec3up;
  
      moveDir = Quaternion::Normalize(camera.rotation)*Quaternion(moveDir.normalize())*Quaternion::Normalize(Quaternion::Inverse(camera.rotation));
      if(moveDir.magnitude() > 0) camera.position += moveDir * (User::GetKey(Keyboard::Key::LShift)? quickSpeed : moveSpeed) * System::deltaTime;
  
      static Vec2 rot = Vec2(camera.rotation.getEuler().x, camera.rotation.getEuler().y);
      rot.x += mouseDelta.y * System::deltaTime;
      rot.y += mouseDelta.x * System::deltaTime;
  
      camera.rotation = Quaternion::AngleAxis(rot.y, Vec3up) * Quaternion::AngleAxis(rot.x, Vec3right);
  
      Renderer::UpdateViewMatrix();
    }
  }

  water.perlin.shift += Vector2f(wind * System::deltaTime);

  water.update();
}
static Vec3 getColorVec(const Color& color) {
  return Vec3(color.r, color.g, color.b) / 255.0f;
}
void Renderer::draw() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_CULL_FACE);

  float model[16];
  if(!WireframeView) {
    glUseProgram(TriangleShader);
    TriangleShader.SetUniform("projection", projection);
    TriangleShader.SetUniform("view", view);

    TriangleShader.SetUniform("lightDirection", Vec3up);
    TriangleShader.SetUniform("lightColor", getColorVec(Color::White));

    water.transform.getMatrix(model);
    TriangleShader.SetUniform("model", model);

    water.mesh.draw();

  } else { //Wireframe View Active
    glUseProgram(LineShader);
    LineShader.SetUniform("projection", projection);
    LineShader.SetUniform("view", view);

    water.transform.getMatrix(model);
    LineShader.SetUniform("model", model);
    Mesh::GenerateWireframe(water.mesh).draw();
  }

  {
    pushGLUIStates();


    draw(sprite);
    UI::Manager::draw();


    popGLUIStates();
  }

  window->display();
}

void Renderer::clear() {
  glDeleteProgram(TriangleShader);

  delete window;
}