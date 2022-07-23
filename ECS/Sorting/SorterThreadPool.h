#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include <thread>
#include <mutex>


class ThreadPool
{
public:

	static constexpr uint32_t MaxThreads{ 8 };

	ThreadPool();
	~ThreadPool();

	void AddFunction(const std::function<void()>& function);

	const volatile bool& GetQuitBool() const { return m_Quit; }

private:

	static void StartThread(const volatile bool& stopBool, std::function<void()>& functionToWatch);

private:

	std::array<std::jthread, MaxThreads> m_Threads;
	std::array<std::function<void()>, MaxThreads> m_Functions;

	std::mutex m_StartThreadLock;

	volatile bool m_Quit{};

};
