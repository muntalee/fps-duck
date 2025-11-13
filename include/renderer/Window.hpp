#pragma once
#include "Core.hpp"

class Window
{
public:
  SDL_Window *window = nullptr;
  SDL_GLContext context = nullptr;
  ImGuiIO *io = nullptr;

  bool Init(const char *title, int width, int height);
  void BeginFrame();
  void EndFrame();
  void Cleanup();
};
