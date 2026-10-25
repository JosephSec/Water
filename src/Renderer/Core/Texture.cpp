#include <Renderer/Core/Texture.hpp>
using namespace Renderer;

#include <utility>


Texture::Texture(const glm::ivec2 _size, GLenum _internalFormat, GLsizei _levels) : m_id(0) {
  glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
  glTextureStorage2D(m_id, _levels, _internalFormat, _size.x,_size.y);

  glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, _levels > 1? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
  glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
Texture::~Texture() {
  if(m_id != 0) glDeleteTextures(1, &m_id);
}

Texture::Texture(Texture &&_o) {
  m_id = std::exchange(_o.m_id, 0);
  m_internalFormat = std::exchange(_o.m_internalFormat, 0);
  m_levels = std::exchange(_o.m_levels, 0);

  m_size = std::exchange(_o.m_size, glm::ivec2{0,0});
}
Texture &Texture::operator=(Texture &&_o) {
  if(this != &_o) {
    if(m_id != 0) glDeleteTextures(1, &m_id);

    m_id = std::exchange(_o.m_id, 0);
    m_internalFormat = std::exchange(_o.m_internalFormat, 0);
    m_levels = std::exchange(_o.m_levels, 0);

    m_size = std::exchange(_o.m_size, glm::ivec2{0,0});
  }
  return *this;
}


void Texture::bindUnit(GLuint _unit) const noexcept {
  glBindTextureUnit(_unit, m_id);
}
void Texture::generateMipmaps() const noexcept {
  glGenerateMipmap(m_id);
}
