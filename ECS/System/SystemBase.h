#pragma once
#include <string>
#include "../Registry/TypeViewBase.h"

enum class ExecutionTime : int32_t
{
	Update = 0,
	PreUpdate = -1000,
	LateUpdate = 1000,
	Render = 2000,
	LateRender = 3000,
};

class SystemBase
{
public:

	SystemBase(const std::string& name, int32_t executionOrder = int32_t(ExecutionTime::Update)) : m_Name(name), m_ExecutionOrder(executionOrder) {}
	virtual ~SystemBase() = default;

	virtual void Execute() = 0;

	int32_t GetExecutionOrder() const { return m_ExecutionOrder; }
	const std::string& GetName() const { return m_Name; }

private:

	const std::string m_Name;
	const int32_t m_ExecutionOrder{};
};