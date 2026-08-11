#pragma once

#include <string>
#include <iostream>
#include <GL/glew.h>
using string = std::string;

#include <System/Vec2.hpp>
#include <System/Vec3.hpp>
#include <SFML/Graphics/Color.hpp>
using Color = sf::Color;


class Shader {
public:
  GLuint shader, vertex, fragment;

  Shader() {}
  Shader(const string& name);

  inline operator GLuint() const {
    return shader;
  }

  inline void SetUniform(const string& _uniform, float matrix[16]) const {
    glUniformMatrix4fv(glGetUniformLocation(shader, _uniform.c_str()), 1, GL_FALSE, matrix);
  }
  inline void SetUniform(const string& _uniform, const float& val) const {
    glUniform1f(glGetUniformLocation(shader, _uniform.c_str()), val);
  }
  inline void SetUniform(const string& _uniform, const int& val) const {
    glUniform1i(glGetUniformLocation(shader, _uniform.c_str()), val);
  }
  inline void SetUniform(const string& _uniform, const Vec2& vec) const {
    glUniform2f(glGetUniformLocation(shader, _uniform.c_str()), vec.x, vec.y);
  }
  inline void SetUniform(const string& _uniform, const Vec3& vec) const {
    glUniform3f(glGetUniformLocation(shader, _uniform.c_str()), vec.x, vec.y, vec.z);
  }
  inline void SetUniform(const string& _uniform, const Color& clr) const {
    glUniform4f(glGetUniformLocation(shader, _uniform.c_str()), clr.r/255.0f, clr.g/255.0f, clr.b/255.0f, clr.a/255.0f);
    // glUniform4f(glGetUniformLocation(shader, _uniform.c_str()), clr.r, clr.g, clr.b, clr.a);
  }


  static string LoadShaderFile(const string& _fileName);
  static GLuint CompileShader(GLenum type, const char* src);
};