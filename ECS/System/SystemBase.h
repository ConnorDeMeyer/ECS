#pragma once
#include <string>
#include <assert.h>
#include <bitset>

#include "../Registry/TypeViewBase.h"

/**
 * named values for execution times used in systems.
 * these values may be changed and other values may be used instead of these in the constructor of SystemParameters
 */
enum class ExecutionTime : int32_t
{
	Update = 0,
	PreUpdate = -1000,
	LateUpdate = 1000,
	Render = 2000,
	LateRender = 3000,
};

enum class SystemFlags : uint8_t
{
	SubSystem = 0,
	DefaultSystem = 1,

	SIZE,
};

/**
 * Contains parameters relevant to systems.
 * - name: Name of the system. It will register the system as that name and can be added to the registry using the name.
 * - executionTime: When the system should execute compared to other systems. Lower numbers will execute before higher numbers.
 * - updateInterval: The time it takes between each Execute call. 0.f for no interval
 */
struct SystemParameters
{
	/**
	* @param _name: Name of the system. It will register the system as that name and can be added to the registry using the name.
	* @param _executionTime: When the system should execute compared to other systems. Lower numbers will execute before higher numbers.
	* @param _updateInterval: The time it takes between each Execute call. 0.f for no interval
	*/
	SystemParameters(const std::string& _name, int32_t _executionTime = int32_t(ExecutionTime::Update), float _updateInterval = 0.f)
		: name(_name), executionTime(_executionTime), updateInterval(_updateInterval)
	{}

	std::string name;
	int32_t executionTime = int32_t(ExecutionTime::Update);
	float updateInterval = 0.f;
};

/**
 * SystemBase is the abstract base class for all systems.
 * the virtual functions that can be overriden are:
 *  - Execute(): this method should contain code that acts upon the components
 *	- Initialize(): optional method that is called when the system is added to the registry and a TypeView or TypeBinding is assigned
 */
class SystemBase
{
public:

	SystemBase(const SystemParameters& params) : m_Parameters(params) { assert(params.updateInterval >= 0.f); }
	SystemBase() = default;
	virtual ~SystemBase() = default;

	virtual void Execute() = 0;
	virtual void Initialize() {}
	virtual size_t GetEntityAmount() = 0;
	virtual void PrintTypes(std::ostream& stream) = 0;

	void Update(float DeltaTime)
	{
		if ( (m_AccumulatedTime += DeltaTime) > m_Parameters.updateInterval)
		{
			m_DeltaTime = (m_Parameters.updateInterval == 0.f) ? DeltaTime : m_Parameters.updateInterval;
			Execute();
			m_AccumulatedTime = (m_Parameters.updateInterval == 0.f) ? 0.f : m_AccumulatedTime - m_Parameters.updateInterval;
		}
	}

	const SystemParameters& GetSystemParameters() const { return m_Parameters; }
	float GetDeltaTime() const { return m_DeltaTime; }
	float GetAccumulatedTime() const { return m_AccumulatedTime; }

	bool IsSubSystem() const { return m_Flags[uint8_t(SystemFlags::SubSystem)]; }
	bool IsDefaulySystem() const { return m_Flags[uint8_t(SystemFlags::DefaultSystem)]; }

	void SetFlag(SystemFlags flag, bool value) { m_Flags[uint8_t(flag)] = value; }

private:

	SystemParameters m_Parameters;
	float m_DeltaTime;
	float m_AccumulatedTime;
	std::bitset<uint8_t(SystemFlags::SIZE)> m_Flags;
};