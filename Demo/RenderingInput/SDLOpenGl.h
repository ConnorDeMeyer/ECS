#pragma once
#include <functional>

struct SDL_Window;

namespace OpenGl
{
	int Initialize();

	void Clear();
	void Clear(float r, float g, float b, float a);

	SDL_Window* GetWindow();

}

namespace SDL
{
	void QuitProgram();

	void StartLoop();

	void SetUpdateCallback(const std::function<void(float)>& callback);

	void SetInputCallback(int key, const std::function<void()>& callback);
}