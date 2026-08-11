#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include <System/Rect.hpp>

#include <UI/UIAPI.hpp>
#include <UI/UIWindow.hpp>
#include <UI/UITab.hpp>
#include <UI/UIColorSelect.hpp>
#include <UI/UITextEditor.hpp>


using VertexArray = sf::VertexArray;
using Color = sf::Color;
using Event = sf::Event;
using Text  = sf::Text;


namespace UI {
  class UI_API UIController {
  public:
    static UIWindow uiWindow;
    
    static Text listText;
    static Text titleText;

    static int globalValueX;


    static void init();
    static void update();
    static void draw();
    static void clear();

    static inline void SetTab(unsigned int tabIndex) {
      uiWindow.currentTab = &uiWindow.tabs[uiWindow.currentTabIndex = tabIndex];

      string str;
      for(UIObject* object : uiWindow.currentTab->objects) {
        if(dynamic_cast<UIHeader*>(object) == nullptr) str += object->name + '\n';
        else str += '\n'; //object type is a header
      }
      listText.setString(str);
      titleText.setString(uiWindow.currentTab->name);
    }
    static inline void PreviousTab() {
      const int tabCount = uiWindow.tabs.size();
      if(tabCount == 0) return;
      SetTab(uiWindow.currentTabIndex-- <= 0? tabCount-1 : uiWindow.currentTabIndex);
    }
    static void NextTab() {
      const int tabCount = uiWindow.tabs.size();
      if(tabCount == 0) return;
      SetTab(uiWindow.currentTabIndex++ >= tabCount-1? 0 : uiWindow.currentTabIndex);
    }

    static void ToggleActive();
    static void ResetPosition();
    static const bool HandleLeftClick(const Vec2& _mouse);


    static void AddTab(const string& name);
    static void AddBool(const string& str, bool& val);
    static void AddColor(const string& str, Color& val);

    template <typename T>
    static inline void AddTitle(const string& str, T* val, bool editable) {
      uiWindow.AddObject(new UITitle(str, val, editable));
    }
    
    static void AddHeader(const string& str = "");


    static UIBool* GetButton(const Vec2& v);
    static UIColor* GetColorButton(const Vec2& v);

    static int GetListPosition(int index = -1);

    static float getValueWidth();
  };
};