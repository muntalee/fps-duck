#include "renderer/Window.hpp"

bool Window::Init(const char *title, int width, int height)
{
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
    return false;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  window = SDL_CreateWindow(title, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if (!window)
  {
    std::cerr << "Window Error: " << SDL_GetError() << std::endl;
    return false;
  }

  context = SDL_GL_CreateContext(window);
  if (!context)
  {
    std::cerr << "GL Context Error: " << SDL_GetError() << std::endl;
    return false;
  }

  SDL_GL_MakeCurrent(window, context);
  SDL_GL_SetSwapInterval(1);

  if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
  {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return false;
  }

  // ImGui setup
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  io = &ImGui::GetIO();
  ImGui::StyleColorsDark();
  ImGui_ImplSDL3_InitForOpenGL(window, context);
  ImGui_ImplOpenGL3_Init("#version 330");

  return true;
}

void Window::BeginFrame()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
}

void Window::EndFrame()
{
  ImGui::Render();
  glViewport(0, 0, (int)io->DisplaySize.x, (int)io->DisplaySize.y);
  // Don't clear the color buffer here — 3D scene has already been rendered
  // by the RenderSystem. Only clear depth so ImGui draws on top correctly.
  glClearDepth(1.0);
  glClear(GL_DEPTH_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  SDL_GL_SwapWindow(window);
}

void Window::Cleanup()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  SDL_GL_DestroyContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
