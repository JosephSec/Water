#include <Editor/UI/PerformanceOverlay.hpp>
using namespace Editor::UI;

#include <Renderer/Core.hpp>
#include <Renderer/Hardware/Monitor.hpp>
#include <Renderer/Hardware/GPU.hpp>


namespace {
  sf::Color lerp(sf::Color a, sf::Color b, float t) {
    auto l = [t](uint8_t x, uint8_t y) { return static_cast<uint8_t>(x + (y - x) * t); };
    return sf::Color(l(a.r, b.r), l(a.g, b.g), l(a.b, b.b), l(a.a, b.a));
  }

  // 0 = bad (red), 0.5 = warn (yellow), 1 = good (green)
  sf::Color statusColor(float t) {
    t = std::clamp(t, 0.f, 1.f);
    const sf::Color red(255, 70, 70), yellow(255, 210, 60), green(80, 230, 120);
    return t < .5f ? lerp(red, yellow, t * 2.f) : lerp(yellow, green, (t - .5f) * 2.f);
  }

  void drawGradientRect(sf::FloatRect r, sf::Color top, sf::Color bottom) {
    sf::VertexArray va(sf::PrimitiveType::TriangleStrip, 4);
    va[0] = {r.position, top};
    va[1] = {{r.position.x + r.size.x, r.position.y}, top};
    va[2] = {{r.position.x, r.position.y + r.size.y}, bottom};
    va[3] = {r.position + r.size, bottom};
    Renderer::Core::window->draw(va);
  }

  // Draws text with its top-left at `pos` (bounds offset compensated), optional drop shadow
  void drawText(sf::Text &text, const std::string &str, sf::Vector2f pos,
                sf::Color color, sf::Vector2f shadow = {0, 0}) {
    text.setString(str);
    pos -= text.getLocalBounds().position;
    if(shadow != sf::Vector2f{0, 0}) {
      text.setFillColor(sf::Color(0, 0, 0, 170));
      text.setPosition(pos + shadow);
      Renderer::Core::window->draw(text);
    }
    text.setFillColor(color);
    text.setPosition(pos);
    Renderer::Core::window->draw(text);
  }
}



bool PerformanceOverlay::IsEnabled = false;

sf::FloatRect PerformanceOverlay::Rect = sf::FloatRect{{15.0f, 15.0f}, {350.0f, 200.0f}};

Editor::UI::LineGraph PerformanceOverlay::fpsGraph(60, 0, 240.0f);
Editor::UI::LineGraph PerformanceOverlay::vramGraph(60, 0, 1);
Editor::UI::LineGraph PerformanceOverlay::performanceGraph(60, 0, 1);


void PerformanceOverlay::init() {  
  fpsGraph.max = Renderer::Monitor::RefreshRate;
  fpsGraph.setTimerLength(1 / 15.0f);

  vramGraph.max = Renderer::GPU::VRAMCapacity;
  vramGraph.setTimerLength(1 / 15.0f);

  performanceGraph.max = 1;
  performanceGraph.setTimerLength(1 / 15.0f);
}
void PerformanceOverlay::update(float _dt) {
  const float fps = 1.0f / _dt;
  const int vram = Renderer::GPU::VRAMInUse;
  const float performanceT = (fps / fpsGraph.max + (1 - (vram / vramGraph.max))) / 2.0f;

  fpsGraph.update(fps, _dt);
  vramGraph.update(vram, _dt);
  performanceGraph.update(performanceT, _dt);
}
void PerformanceOverlay::draw(const sf::Font &_font) {
  if(!IsEnabled) return;

  constexpr int   rowCount      = 3;
  constexpr float rowGap        = 4.0f;
  constexpr float graphFraction = 0.6f;
  constexpr float padding       = 8.0f;

  const sf::Color phosphor(60, 255, 120);
  const sf::Color dim = lerp(sf::Color::Black, phosphor, .6f);

  // Layout, recomputed every frame so resizing Rect just works
  const float rowHeight = (Rect.size.y - rowGap * (rowCount - 1)) / rowCount;
  const sf::Vector2f rowSize{Rect.size.x, rowHeight};
  if(rowSize.x < 16.0f || rowSize.y < 16.0f) return; // too small to be legible

  const float graphWidth = rowSize.x * graphFraction;
  const float textWidth  = rowSize.x - graphWidth - padding * 2.0f;
  const unsigned baseSize = static_cast<unsigned>(std::clamp(rowHeight * 0.28f, 8.0f, 22.0f));

  struct Element {
    Editor::UI::LineGraph *graph;
    std::string name;
    std::string value;
  };

  const Element elements[rowCount] = {
    {&fpsGraph,  "FPS",  std::to_string(static_cast<int>(fpsGraph.getValue()))},
    {&vramGraph, "VRAM", std::to_string(static_cast<int>(vramGraph.getValue() / 1000)) + "KB"},
    {&performanceGraph, "PERF", std::to_string(static_cast<int>(performanceGraph.getValue() * 100.0f)) + "%"},
  };

  sf::RectangleShape shape;
  sf::Text text(_font);
  text.setStyle(sf::Text::Style::Regular);

  // Draws a line of text, shrinking the font until it fits textWidth
  auto drawLine = [&](const std::string &str, sf::Vector2f pos, sf::Color color, sf::Vector2f shadow = {0, 0}) {
    unsigned size = baseSize;
    text.setCharacterSize(size);
    text.setString(str);
    while(size > 7 && text.getLocalBounds().size.x > textWidth) {
      text.setCharacterSize(--size);
      text.setString(str);
    }
    drawText(text, str, pos, color, shadow);
  };

  for(int i = 0; i < rowCount; i++) {
    const Element &e = elements[i];
    const sf::Vector2f start = Rect.position + sf::Vector2f{0, (rowHeight + rowGap) * i};

    // Panel (inset by the outline so it stays inside Rect)
    shape.setOutlineThickness(1);
    shape.setOutlineColor(dim);
    shape.setPosition(start + sf::Vector2f{1, 1});
    shape.setSize(rowSize - sf::Vector2f{2, 2});
    shape.setFillColor(sf::Color(0, 12, 4, 240));
    Renderer::Core::window->draw(shape);

    // Scanlines
    shape.setOutlineThickness(0);
    shape.setFillColor(sf::Color(0, 0, 0, 70));
    shape.setSize({rowSize.x - 2.0f, 1.0f});
    for(float y = 1.0f; y < rowSize.y - 1.0f; y += 3.0f) {
      shape.setPosition(start + sf::Vector2f{1, y});
      Renderer::Core::window->draw(shape);
    }

    // Label above, value below, roughly centered vertically
    const float mid = rowHeight * 0.5f;
    drawLine("[" + e.name + "]", start + sf::Vector2f{padding, mid - baseSize * 1.1f}, dim);
    drawLine("> " + e.value,     start + sf::Vector2f{padding, mid + baseSize * 0.15f}, phosphor, {1, 1});

    // Graph fills the right side
    e.graph->draw(sf::FloatRect{start + sf::Vector2f{rowSize.x - graphWidth, 2.0f},
                                sf::Vector2f{graphWidth - 4.0f, rowHeight - 4.0f}}, phosphor);
  }
}