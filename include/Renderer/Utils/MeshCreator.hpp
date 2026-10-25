#pragma once

#include <Renderer/API.hpp>

#include <Renderer/Core/Transform.hpp>
#include <Renderer/Core/Mesh.hpp>


namespace Renderer {
  class RENDER3D_API MeshCreator {
  public:
    static Mesh Cube(const Transform &_transform = Transform(), GLenum _usage = GL_STATIC_DRAW, bool _saveCPUSize = true);
  };
}