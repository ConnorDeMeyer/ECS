#include "RenderingSystem.h"

#include <cassert>

#include "GL/glew.h"
#include "../Shader/Shader.h"
#include "../Shader/Texture.h"

RenderingSystem::RenderingSystem(const SystemParameters& parameters)
	: ViewSystem(parameters)
{
	glGenVertexArrays(1, &m_GLVertexArray);
	glGenBuffers(1, &m_GLBuffer);

	glBindVertexArray(m_GLVertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, m_GLBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Render), nullptr, GL_DYNAMIC_DRAW);
	m_BufferSize = 1;

	glVertexAttribPointer(0, sizeof(glm::vec3) / 4, GL_FLOAT, GL_FALSE, sizeof(Render), (void*)offsetof(Render,Render::Transform));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, sizeof(glm::vec3) / 4, GL_FLOAT, GL_FALSE, sizeof(Render), (void*)12);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, sizeof(glm::vec3) / 4, GL_FLOAT, GL_FALSE, sizeof(Render), (void*)24);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, sizeof(glm::vec4) / 4, GL_FLOAT, GL_FALSE, sizeof(Render), (void*)offsetof(Render, Render::Color));
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(4, sizeof(glm::vec4) / 4, GL_FLOAT, GL_FALSE, sizeof(Render), (void*)offsetof(Render, Render::Uvs));
	glEnableVertexAttribArray(4);

	glVertexAttribPointer(5, sizeof(glm::vec2) / 4, GL_FLOAT, GL_FALSE, sizeof(Render), (void*)offsetof(Render, Render::Pivot));
	glEnableVertexAttribArray(5);

	glVertexAttribPointer(6, sizeof(float) / 4, GL_FLOAT, GL_FALSE, sizeof(Render),		(void*)offsetof(Render, Render::Depth));
	glEnableVertexAttribArray(6);

	glVertexAttribPointer(7, sizeof(uint32_t) / 4, GL_FLOAT, GL_FALSE, sizeof(Render),	(void*)offsetof(Render, Render::textureId));
	glEnableVertexAttribArray(7);

	m_pShader = std::make_unique<Shader>(
		std::filesystem::path("Resources/Shaders/texture.fs"),
		std::filesystem::path("Resources/Shaders/rectangle.vs"),
		std::filesystem::path("Resources/Shaders/rectangle.gs")
	);

	m_pTexture = std::make_unique<Texture2D>("Resources/Textures/fumoFace.png");
	//m_pTexture = std::make_unique<Texture2D>("Resources/Textures/81642.png");
	//glBindTexture(GL_TEXTURE_2D, m_pTexture->GetId());
}

RenderingSystem::~RenderingSystem()
{
	glDeleteBuffers(1, &m_GLBuffer);
	glDeleteVertexArrays(1, &m_GLVertexArray);
}

void RenderingSystem::Execute()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pTexture->GetId());

	m_pShader->SetActive();
	assert(!glGetError());

	m_pShader->SetInt("Texture", 0);
	assert(!glGetError());
	m_pShader->SetMat3("cameraTransform",
		{
			2.f / 1280,0,0,
			0,2.f / 720,0,
			-1, -1, 1 });

	assert(!glGetError());
	size_t elementsAmount = m_TypeView->GetActiveAmount();

	glBindBuffer(GL_ARRAY_BUFFER, m_GLBuffer);

	if (elementsAmount > m_BufferSize)
	{
		glBufferData(GL_ARRAY_BUFFER, sizeof(Render) * elementsAmount, m_TypeView->GetData(), GL_DYNAMIC_DRAW);
		m_BufferSize = uint32_t(elementsAmount);
	}
	else
	{
		auto pBuffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		std::memcpy(pBuffer, m_TypeView->GetData(), elementsAmount * sizeof(Render));
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, m_GLBuffer);
	glBindVertexArray(m_GLVertexArray);

	glDrawArrays(GL_POINTS, 0, GLsizei(elementsAmount));

	assert(!glGetError());
}

RegisterSystem<RenderingSystem> renderSys(SystemParameters{"RenderingSystem", int32_t(ExecutionTime::Render)});

