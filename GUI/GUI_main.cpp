#include "GUI_main.h"

#include <SDL.h>
#include <stdexcept>

#include "ImGui/backends/imgui_impl_opengl3.h"
#include "ImGui/backends/imgui_impl_sdl.h"
#include "ImGui/ImGuiExt/Style.h"
#include "ImGui/ImPlot/implot.h"

#include "Registry/EntityRegistry.h"

SDL_Window* g_pWindow{};

using namespace GUI;

void GUI::InitializeAll()
{
	InitializeSDL();
	InitializeOpenGl();
	InitializeImGui();
	InitializeGUI();
}

void GUI::InitializeSDL()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) != 0)
	{
		throw std::runtime_error(std::string("SDL_Init Error: ") + SDL_GetError());
	}
}

void GUI::InitializeOpenGl(int majorVersion, int minorVersion)
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, majorVersion);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minorVersion);
}

void GUI::InitializeImGui()
{
	ImGui::CreateContext();
	ImPlot::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;			// Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;			// Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;				// Enable Docking
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;			// Enable Multi-Viewport / Platform Windows

	if (!g_pWindow)
		InitializeWindow(1280, 720);

	ImGui_ImplSDL2_InitForOpenGL(g_pWindow, SDL_GL_GetCurrentContext());
	ImGui_ImplOpenGL3_Init("#version 130");

	SetImGuiStyle();
}

void GUI::InitializeWindow(size_t width, size_t height)
{
	g_pWindow = SDL_CreateWindow(
		"ECS GUI",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		int(width),
		int(height),
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);
	if (g_pWindow == nullptr)
	{
		throw std::runtime_error(std::string("SDL_CreateWindow Error: ") + SDL_GetError());
	}
}

void GUI::SetWindowTitle(const char* title)
{
	SDL_SetWindowTitle(g_pWindow, title);
}

void GUI::SetWindow(SDL_Window* window)
{
	assert(window);
	g_pWindow = window;
}

SDL_Window* GUI::GetWindow()
{
	return g_pWindow;
}

void GUI::ProcessEvents(SDL_Event* e)
{
	ImGui_ImplSDL2_ProcessEvent(e);
}

void GUI::InitializeGUI()
{

}

void GUI::UpdateGUI()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(g_pWindow);
	ImGui::NewFrame();

	System::UpdateAll();
	Registry::UpdateAll();
	Entities::UpdateAll();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::Destroy()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();

	ImPlot::DestroyContext();
	ImGui::DestroyContext();
}
