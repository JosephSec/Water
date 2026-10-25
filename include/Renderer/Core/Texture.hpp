#pragma once

#include <glm/vec2.hpp>

#include <Renderer/API.hpp>

#include <GL/glew.h>


namespace Renderer {
  class RENDER3D_API Texture {
  public:
    Texture() : m_id(0), m_internalFormat(GL_RGBA8), m_levels(1), m_size(glm::ivec2{0,0}) {}
    Texture(const glm::ivec2 _size, GLenum _internalFormat = GL_RGBA8, GLsizei _levels = 1);
    ~Texture();

    Texture(Texture &&_o);
    Texture &operator=(Texture &&_o);

    Texture(Texture &) = delete;
    Texture &operator=(Texture &) = delete;


    void bindUnit(GLuint _unit) const noexcept;
    void generateMipmaps() const noexcept;

    inline GLuint getID() const noexcept {
      return m_id;
    }


  private:
    GLuint  m_id = 0;
    GLenum  m_internalFormat = GL_RGBA8;
    GLsizei m_levels = 1;

    glm::ivec2 m_size = glm::ivec2(0);
  };
}