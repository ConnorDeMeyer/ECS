#pragma once
#include <fstream>
#include <iostream>

struct Render
{
	float Transformation[3][3]{};
	float Color[4]{};
	float UV[4]{};
	float Pivot[2]{};
	float Depth{};
	uint64_t TextureId{};

	void Serialize(std::ofstream& stream)
	{
		std::cout << "Serializing Render\n";
		stream.write(reinterpret_cast<const char*>(this), sizeof(Render));
	}

	void Deserialize(std::ifstream& stream)
	{
		std::cout << "Deserializing Render\n";
		stream.read(reinterpret_cast<char*>(this), sizeof(Render));
	}
};

inline bool SortCompare(const Render& r0, const Render& r1)
{
	return r0.TextureId < r1.TextureId;
}
