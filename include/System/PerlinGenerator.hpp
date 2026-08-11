#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #ifdef PERLINGEN_DLL_BUILD
    #define PERLINGEN_API __declspec(dllexport)
  #elif PERLINGEN_DLL
    #define PERLINGEN_API __declspec(dllimport)
  #else
    #define PERLINGEN_API
  #endif
#else
  #define PERLINGEN_API
#endif

#include <string>
#include <random>
#include <chrono>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector3.hpp>

using string = std::string;
using Vector2f = sf::Vector2f;
using Vector2u = sf::Vector2u;
using Vector2i = sf::Vector2i;
using Vector3f = sf::Vector3f;
using Color = sf::Color;


struct PERLINGEN_API PerlinObject {
public:
  unsigned int size = 30;
  float lowWeight = 1;
  float scale = 1;
  float resolution = 1;
  Vector2f shift;

  std::vector<double> data;
  int64_t key;
};

class PERLINGEN_API PerlinGenerator {
public:
  static string PATH;

  static void Generate(PerlinObject& object);

  static sf::Image GenerateImage(const PerlinObject& object, const Color& clrA, const Color& clrB);


  static inline int64_t generateKey() {
    return std::chrono::high_resolution_clock().now().time_since_epoch().count();
  }

private:
  static std::vector<int> permutation;

  static void generate(int64_t key);

  static double noise(double x, double y);
  static inline double fade(double t) {
    return t*t*t*(t*(t*6-15)+10);
  }
  static inline double lerp(double t, double a, double b) {
    return a + t*(b-a);
  }
  static inline double grad(int hash, double x, double y) {
    int h = hash & 7;
    double u = h < 4? x : y;
    double v = h < 4? y : x;

    return ((h & 1)? -u : u) + ((h & 2)? -v : v);
  }
};