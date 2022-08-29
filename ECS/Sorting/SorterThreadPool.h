#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include <thread>
#include <mutex>


class ThreadPool
{
public:

	ThreadPool(const size_t threadAmount);
	~ThreadPool();

	void AddFunction(const std::function<void()>& function);

	const volatile bool& GetQuitBool() const { return m_Quit; }

	void QuitAndWait();

private:

	static void StartThread(const volatile bool& stopBool, std::function<void()>& functionToWatch);

private:

	std::vector<std::jthread> m_Threads;
	std::vector<std::function<void()>> m_Functions;

	std::mutex m_StartThreadLock;

	volatile bool m_Quit{};

};
