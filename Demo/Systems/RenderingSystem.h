#pragma once
#include <System/System.h>
#include "../Components/Render.h"

class RenderingSystem : public ViewSystem<Render>
{
	struct RenderData
	{
		Render* startPoint{};
		size_t offset{};
		uint32_t textureId{};
	};

public:

	RenderingSystem(const SystemParameters& parameters);
	~RenderingSystem();

	void Execute() override;

private:

	unsigned int m_GLBuffer{};
	unsigned int m_GLVertexArray{};

	uint32_t m_BufferSize{};

	std::unique_ptr<class Shader> m_pShader{};
	std::unique_ptr<class Texture2D> m_pTexture{};

};
