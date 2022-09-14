#pragma once
#include <SDL_events.h>

#include "Headers/Registry.h"
#include "Headers/System.h"
#include "Headers/Entities.h"

struct SDL_Window;
union SDL_Event;

namespace GUI
{
	void InitializeAll();
	
	void InitializeSDL();
	void InitializeOpenGl(int majorVersion = 4, int minorVersion = 5);
	void InitializeImGui();
	
	void InitializeWindow(size_t width, size_t height);
	void SetWindowTitle(const char* title);
	void SetWindow(SDL_Window* window);
	SDL_Window* GetWindow();

	void ProcessEvents(SDL_Event* e);
	
	void InitializeGUI();
	void UpdateGUI();
	void Destroy();

}