#pragma once

#include <vector>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Color.hpp>
using Color = sf::Color;

#include <System/Vec2.hpp>
#include <System/Vec3.hpp>

#include <Renderer/Line.hpp>


class Mesh {
public:
  static void init();


  std::vector<Vec3> vertices;
  std::vector<Vec2> uvs;
  std::vector<uint32_t> indecies;

  unsigned int textureHandle;

  Mesh();

  void update();
  void draw() const;


  static Line GenerateWireframe(const Mesh& mesh) {
    Line line;
    line.vertices = mesh.vertices;
    line.indecies = std::vector<uint32_t>(mesh.indecies.size() * 2);

    const unsigned int triC = mesh.indecies.size()/3;
    for(int i = 0; i < triC; i++) {
      const unsigned int startA = i * 6;
      const unsigned int startB = i * 3;

      const uint32_t a = mesh.indecies[startB+0];
      const uint32_t b = mesh.indecies[startB+1];
      const uint32_t c = mesh.indecies[startB+2];

      line.indecies[startA+0] = a;
      line.indecies[startA+1] = b;
      line.indecies[startA+2] = b;
      line.indecies[startA+3] = c;
      line.indecies[startA+4] = c;
      line.indecies[startA+5] = a;
    }

    line.colors = std::vector<Color>(line.indecies.size(), Color::Magenta);
    return line;
  }
};