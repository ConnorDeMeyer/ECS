#include "SorterThreadPool.h"

#include <condition_variable>
#include <chrono>

using namespace std::chrono_literals;

ThreadPool::ThreadPool()
{
	std::scoped_lock lock(m_StartThreadLock);

	for (uint32_t i{}; i < MaxThreads; ++i)
	{
		m_Threads[i] = std::jthread(ThreadPool::StartThread, std::ref(m_Quit), std::ref(m_Functions[i]));
	}
}

ThreadPool::~ThreadPool()
{
	m_Quit = true;
}

void ThreadPool::AddFunction(const std::function<void()>& function)
{
	for (uint32_t i{}; i < MaxThreads; ++i)
	{
		if (!m_Functions[i])
		{
			m_Functions[i] = function;
			return;
		}
	}
}

void ThreadPool::StartThread(const volatile bool& stopBool, std::function<void()>& functionToWatch)
{
	std::mutex waitMutex;
	std::unique_lock waitLock(waitMutex);
	std::condition_variable waitCond;

	while (true)
	{
		waitCond.wait_for(waitLock, 100ms, [stopBool, functionToWatch] {return stopBool || functionToWatch; });

		if (stopBool)
			return;

		if (functionToWatch)
		{
			functionToWatch();
			functionToWatch = std::function<void()>();
		}
	}
}
