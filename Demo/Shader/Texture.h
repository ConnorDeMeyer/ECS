#pragma once
#include <cstdint>
#include <filesystem>

class Texture2D final
{
	friend class RenderManager;
	friend class ResourceManager;

public:
	//SDL_Texture* GetSDLTexture() const { return m_Texture; }
	Texture2D() = default;

	Texture2D(const Texture2D&) = delete;
	Texture2D& operator=(const Texture2D&) = delete;

	Texture2D(Texture2D&& other) noexcept
		: m_Id{ other.m_Id }
		, m_Width{ other.m_Width }
		, m_Height{ other.m_Height }
	{
		other.m_Id = 0;
		other.m_Height = 0;
		other.m_Width = 0;
	}

	Texture2D& operator=(Texture2D&& other) noexcept
	{
		m_Id = other.m_Id;
		other.m_Id = 0;
		m_Width = other.m_Width;
		other.m_Width = 0;
		m_Height = other.m_Height;
		other.m_Height = 0;
		return *this;
	}

	explicit Texture2D(uint32_t id, int width, int height)
		: m_Id{ id }, m_Width{ width }, m_Height{ height } {}

	Texture2D(const std::filesystem::path& path);

	~Texture2D();

	uint32_t GetId() const { return m_Id; }
	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }

	inline bool IsValid() { return m_Id; }
	inline operator bool() { return IsValid(); }


private:
	uint32_t m_Id{};
	int m_Width{};
	int m_Height{};

	static bool InitializedIMG;
};
