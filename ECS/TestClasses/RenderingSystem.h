#pragma once
#include "Render.h"
#include "../System/System.h"
#include "../Registry/TypeView.h"

class RenderingSystem : public ViewSystem<Render>
{
	struct RenderData
	{
		Render* startPoint{};
		size_t offset{};
		uint64_t textureId{};
	};

public:

	RenderingSystem(const std::string& name, TypeView<Render>* view, int32_t executionOrder)
		: ViewSystem(name, view, executionOrder)
	{
		OrganizeData();
	}
	virtual ~RenderingSystem() = default;

	void Execute() override
	{
		if (m_TypeView->GetDataFlag() != ViewDataFlag::valid && m_DataFlagId != m_TypeView->GetDataFlagId())
		{
			OrganizeData();
		}
	}

private:

	void OrganizeData()
	{
		m_RenderData.clear();
		if (!m_TypeView->GetSize())
			return;
		
		uint64_t lastTextureId{};

		auto first = m_TypeView->begin();
		lastTextureId = first->TextureId;
		m_RenderData.emplace_back(&*first, 0, first->TextureId);
		

		for (Render& render : m_TypeView)
		{
			if (render.TextureId != lastTextureId)
			{
				m_RenderData.back().offset = &render - m_RenderData.back().startPoint;
				m_RenderData.emplace_back(&render, 0, render.TextureId);
			}
		}

		const Render* lastAddress = m_TypeView->GetData() + m_TypeView->GetSize();
		m_RenderData.back().offset = lastAddress - m_RenderData.back().startPoint;

		m_DataFlagId = m_TypeView->GetDataFlagId();
	}

private:

	std::vector<RenderData> m_RenderData;
	uint16_t m_DataFlagId{};

};