#pragma once

#include <filesystem>

#include <Renderer/API.hpp>

#include <Renderer/Core/Shader.hpp>
#include <Renderer/Core/Cubemap.hpp>
#include <Renderer/Component/Camera.hpp>


namespace Renderer {
  class RENDER3D_API Skybox {
  public:
    static bool SplitToFaces(const std::filesystem::path &_file);


    Skybox(int _size = 256, GLenum _internalFormat = GL_RGBA8, int _levels = 1);
    ~Skybox();

    void operator()();

    void bindUnit(GLuint _unit) const noexcept {
      m_cubemap.bindUnit(_unit);
    }

    void draw(const Shader *_shader, const Renderer::Camera *_camera) const noexcept;

    bool loadFromFolder(const std::filesystem::path &_folder);


  private:
    GLuint m_vao;
    Cubemap m_cubemap;

    int m_size = 0;
    GLenum m_internalFormat = 0;
    int m_levels = 1;
  };
}