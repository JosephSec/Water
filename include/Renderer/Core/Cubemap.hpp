#pragma once

#include <Renderer/API.hpp>

#include <GL/glew.h>
#include <Renderer/Core/Shader.hpp>


namespace Renderer {
  class RENDER3D_API Cubemap {
  public:
    Cubemap(int _size, GLenum _internalFormat, int _levels = 1);
    ~Cubemap();

    Cubemap(Cubemap&& o) noexcept;
    Cubemap& operator=(Cubemap&& o) noexcept;

    Cubemap(const Cubemap&) = delete;
    Cubemap& operator=(const Cubemap&) = delete;


    void operator()();

    void generateMipmaps() const noexcept;

    void uploadFace(int _face, const void *_pixels, GLenum _format = GL_RGBA, GLenum _type = GL_UNSIGNED_BYTE, int _level = 0);

    void bindUnit(GLuint _unit) const;
    inline GLuint id() const noexcept {
      return m_id;
    }


  private:
    GLuint m_id = 0;
    int m_size = 0;
    GLenum m_internalFormat = GL_RGBA8;
    int m_levels = 1;
  };
}