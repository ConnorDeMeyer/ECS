#pragma once
#include <functional>

namespace OpenGl
{
	int Initialize();

	void Clear();
}

namespace SDL
{
	void QuitProgram();

	void StartLoop();

	void SetUpdateCallback(const std::function<void(float)>& callback);

	void SetInputCallback(int key, const std::function<void()>& callback);
}