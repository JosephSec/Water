#pragma once

#include <vector>
#include <SFML/Graphics/Color.hpp>
using Color = sf::Color;

#include <System/Vec3.hpp>


class Line {
public:
  static void init();


  std::vector<Vec3> vertices;
  std::vector<Color> colors;
  std::vector<uint32_t> indecies;

  Line();

  void update();
  void draw() const;
};