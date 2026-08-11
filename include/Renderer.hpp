#pragma once

#include <SFML/Graphics.hpp>
using Color = sf::Color;

#include <System/Vec2.hpp>
#include <System/Transform.hpp>
#include <System/PerlinGenerator.hpp>

#include <Renderer/Shader.hpp>
#include <Renderer/Mesh.hpp>
#include <Renderer/Line.hpp>


static inline void SetMatrixVal(float out[16], float in[16]) {
  for(int i = 0; i < 16; i++) out[i] = in[i];
}


class Water {
public:
  PerlinObject perlin;

  Transform transform;
  Mesh mesh;
  Line wireframe;

  float waveSize = 1;


  void update();
};
class Renderer {
public:
  static sf::RenderWindow* window;
  static Vec2 windowSize;
  static Vec2 windowCenter;

  static sf::Font font;
  static sf::Text TextPrefab;


  static float aspectRatio;
  static float nearPlane;
  static float farPlane;
  static float fov;
  static float f;

  static float projection[16];
  static float view[16];

  static Transform camera;
  
  static Shader TriangleShader;
  static Shader LineShader;

  static Water water;

  static bool WireframeView;
  static Vec2 wind;
  static float WaterUpdateTime;


  static void init();
  static void initGL();

  static void update();
  static void draw();
  static inline void draw(const sf::Drawable& drawable) {
    window->draw(drawable);
  }

  static void clear();


  static void UpdateViewMatrix() {
    SetMatrixVal(view, identityMatrix);
    float rotMat[16];
    Quaternion::Inverse(camera.rotation).getMatrix(rotMat);
    float transMat[16];
    SetMatrixVal(transMat, identityMatrix);
    for(int i = 0; i < 16; i++)
      view[i] = (i % 5 == 0) ? 1.0f : 0.0f;

    for(int row = 0; row < 3; row++)
      for (int col = 0; col < 3; col++)
      view[col + row * 4] = rotMat[col + row * 4];

    Vec3 p = -Vec3(camera.position.x, camera.position.y, camera.position.z);
    view[12] = p.x * view[0] + p.y * view[4] + p.z * view[8];
    view[13] = p.x * view[1] + p.y * view[5] + p.z * view[9];
    view[14] = p.x * view[2] + p.y * view[6] + p.z * view[10];
    view[15] = 1.0f;
  }
  static inline void HandleResize() {
    const Vec2 windowSize = window->getSize();
    
    window->setView(sf::View(sf::FloatRect(Vec2(), windowSize)));
    glViewport(0,0, windowSize.x, windowSize.y);

    aspectRatio = windowSize.x / windowSize.y;

    float newProjection[16] = {
      f/aspectRatio, 0, 0, 0,
      0, f, 0, 0,
      0, 0, (farPlane+nearPlane)/(farPlane-nearPlane), 1,
      0, 0, (-2*farPlane*nearPlane)/(farPlane-nearPlane), 0
    };

    SetMatrixVal(projection, newProjection);
  }


private:
  static void pushGLUIStates() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glUseProgram(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  static void popGLUIStates() {
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
  }
};


#include <glm/gtc/type_ptr.hpp>

static Vec3 ApplyTransform(const Vec3& vec, const Transform& transform, bool isDirection = true) {
  float mat4[16];
  transform.getMatrix(mat4);
  glm::vec3 result = glm::make_mat4(mat4) * glm::vec4(vec.x, vec.y, vec.z, !isDirection);
  return Vec3(result.x, result.y, result.z);
}

static Vec3 ScreenToWorld(const Vec2& screen, float depth = 0) {
  glm::vec4 ndc = glm::vec4(0,0,0,1);
  ndc.x = 2.0f * screen.x / Renderer::windowSize.x - 1.0f;
  ndc.y = 1.0f - 2.0f * screen.y / Renderer::windowSize.y;
  ndc.z = 2.0f * depth - 1.0f;

  glm::vec4 eye = glm::inverse(glm::make_mat4(Renderer::projection)) * ndc;
  eye /= eye.w;

  glm::vec4 world = glm::inverse(glm::make_mat4(Renderer::view)) * eye;
  return Vec3(world.x, world.y, world.z);
}

static Vec2 WorldToScreen(const Vec3& world) {
  glm::vec4 clip = glm::make_mat4(Renderer::projection) * glm::make_mat4(Renderer::view) * glm::vec4(world.x, world.y, world.z, 1.0f);
  glm::vec3 ndc = glm::vec3(clip) / clip.w;
  return Vec2((ndc.x + 1.0f) * 0.5f * Renderer::windowSize.x, (1.0f - ndc.y) * 0.5f * Renderer::windowSize.y);
}