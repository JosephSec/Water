#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Font.hpp>

#include <Editor/UI/Graph.hpp>


namespace Editor {
  namespace UI {
    class PerformanceOverlay {
    public:
      static bool IsEnabled;

      static sf::FloatRect Rect;

      static Editor::UI::LineGraph fpsGraph;
      static Editor::UI::LineGraph vramGraph;
      static Editor::UI::LineGraph performanceGraph;


      static void init();
      static void update(float _dt);
      static void draw(const sf::Font &_font);

      static inline void ToggleEnabled(bool _enabled = !IsEnabled) {
        IsEnabled = _enabled;
      }
    };
  }
}