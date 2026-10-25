#include <Renderer/Utils/MeshCreator.hpp>
using namespace Renderer;


Mesh MeshCreator::Cube(const Transform &_transform, GLenum _usage, bool _saveCPUSize) {
  Mesh mesh;
  mesh();

  std::vector<Renderer::Mesh::Vertex> vertices = {
    Renderer::Mesh::Vertex{{-1,-1, 1}, {1,1,1,1}},
    Renderer::Mesh::Vertex{{-1, 1, 1}, {1,1,1,1}},
    Renderer::Mesh::Vertex{{ 1, 1, 1}, {1,1,1,1}},
    Renderer::Mesh::Vertex{{ 1,-1, 1}, {1,1,1,1}},
    
    Renderer::Mesh::Vertex{{-1,-1,-1}, {1,1,1,1}},
    Renderer::Mesh::Vertex{{-1, 1,-1}, {1,1,1,1}},
    Renderer::Mesh::Vertex{{ 1, 1,-1}, {1,1,1,1}},
    Renderer::Mesh::Vertex{{ 1,-1,-1}, {1,1,1,1}},
  };
  std::vector<Renderer::Mesh::Index> indices = {
    0,4,5, 0,5,1, //-x
    6,7,3, 6,3,2, //+x

    0,3,7, 0,7,4, //-y
    1,5,6, 1,6,2, //+y

    7,6,5, 7,5,4, //-z
    0,1,2, 0,2,3, //+z
  };

  mesh.setVertices(vertices, _usage, _saveCPUSize);
  mesh.setIndices(indices, _usage, _saveCPUSize);

  return mesh;
}