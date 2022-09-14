#include "SDLOpenGl.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <SDL.h>
#include <gl/glew.h>
#include <chrono>

#include <GUI_main.h>

SDL_Window* g_pMainWindow{};
SDL_GLContext g_gl_context{};
bool g_Quit{};
std::function<void(float)> g_UpdateCallback;

std::unordered_map<int, std::function<void()>> g_InputsMaps;

int OpenGl::Initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		std::cerr << "SDL_Init Error: " << SDL_GetError();
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_DisplayMode DM;
	SDL_GetCurrentDisplayMode(0, &DM);

	constexpr float w{ 1280 }, h{ 720 };

	g_pMainWindow = SDL_CreateWindow(
		"ECS Demo",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		int(w),
		int(h),
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);
	if (g_pMainWindow == nullptr)
	{
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError();
		return 1;
	}

	g_gl_context = SDL_GL_CreateContext(g_pMainWindow);
	if (g_gl_context == nullptr)
	{
		std::cerr << "Core::Initialize( ), error when calling SDL_GL_CreateContext: " << SDL_GetError() << std::endl;
		return 1;
	}

	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to initialize GLEW " << SDL_GetError() << std::endl;
		return 1;
	}

	if (SDL_GL_SetSwapInterval(1) < 0)
	{
		std::cerr << "Core::Initialize( ), error when calling SDL_GL_SetSwapInterval: " << SDL_GetError() << std::endl;
		return 1;
	}
	
	int width{}, height{};
	SDL_GL_GetDrawableSize(g_pMainWindow, &width, &height);

	glViewport(0, 0, width, height);

	// Enable color blending and use alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_CULL_FACE);

	return 0;
}

void OpenGl::Clear()
{
	glClearColor(0, 0, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGl::Clear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
}

SDL_Window* OpenGl::GetWindow()
{
	return g_pMainWindow;
}

void SDL::QuitProgram()
{
	g_Quit = true;
}

void ProcessInput()
{
	SDL_Event e;
	while (SDL_PollEvent(&e)) {

		GUI::ProcessEvents(&e);

		//float mult = 1.f;

		if (e.type == SDL_KEYDOWN) {
			if (e.key.repeat) break;
			{
				auto it = g_InputsMaps.find(e.key.keysym.sym);
				if (it != g_InputsMaps.end())
					it->second();
			}
		}
		else if (e.type == SDL_KEYUP) {
			if (e.key.repeat) break;
			//HandleKeyUp(e);
		}
		else if (e.type == SDL_MOUSEBUTTONDOWN) {
			//HandleMouseDown(e);
		}
		else if (e.type == SDL_MOUSEBUTTONUP) {
			//HandleMouseUp(e);
		}
		else if (e.type == SDL_MOUSEWHEEL) {
			//HandleMouseWheel(e);
		}
		else if (e.type == SDL_MOUSEMOTION) {
			//HandleMouseMotion(e);
		}
		else if (e.type == SDL_CONTROLLERBUTTONUP) {
			//HandleControllerButtonUp(e);
		}
		else if (e.type == SDL_CONTROLLERBUTTONDOWN) {
			//HandleControllerButtonDown(e);
		}
		//else if (e.type == SDL_CONTROLLERAXISMOTION) {
		//	HandleControllerAxis(e);
		//}
		else if (e.type == SDL_WINDOWEVENT) {
			//HandleWindowEvent(e);
		}
		else if (e.type == SDL_QUIT) {
			g_Quit = true;
		}
	}
}

void SDL::StartLoop()
{
	if (!g_UpdateCallback)
		return;

	auto begin = std::chrono::high_resolution_clock::now();

	while (!g_Quit)
	{
		ProcessInput();

		OpenGl::Clear();

		auto end = std::chrono::high_resolution_clock::now();

		g_UpdateCallback(std::chrono::duration<float>(end - begin).count());

		begin = end;

		SDL_GL_SwapWindow(g_pMainWindow);
	}
}

void SDL::SetUpdateCallback(const std::function<void(float)>& callback)
{
	g_UpdateCallback = callback;
}

void SDL::SetInputCallback(int key, const std::function<void()>& callback)
{
	g_InputsMaps[key] = callback;
}