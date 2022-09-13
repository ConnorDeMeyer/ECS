#include "NewTypeView.h"

constexpr float ReferenceRemovalInterval{ 1 };

void BaseTypeViewData::Update(float deltaTime)
{
	m_AccumulatedTime += deltaTime;

	// Check and remove invalid VoidReferencePointers
	if (m_AccumulatedTime >= ReferenceRemovalInterval)
	{
		m_AccumulatedTime -= ReferenceRemovalInterval;

		const size_t size = m_PendingDeleteReferences.size();
		for (size_t i{}; i < size; ++i)
		{
			auto ref = m_PendingDeleteReferences[size - i - 1];
			if (ref->GetReferencesAmount() == 0)
			{
				m_ReferencePool.deallocate(ref);
				m_PendingDeleteReferences[size - i - 1] = m_PendingDeleteReferences.back();
				m_PendingDeleteReferences.pop_back();
			}
		}
	}

	UpdateData();
}

void NewTypeView::Update(float deltaTime)
{
	m_Data->Update(deltaTime);
}
