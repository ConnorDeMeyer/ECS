#pragma once
#include <fstream>
#include <vector>

class EntityRegistry;

template <typename T>
void WriteStream(std::ostream& stream, const T& val)
{
	stream.write(reinterpret_cast<const char*>(&val), sizeof(T));
}

template <typename T>
void ReadStream(std::istream& stream, T& val)
{
	stream.read(reinterpret_cast<char*>(&val), sizeof(T));
}

void SerializeRegistries(std::ostream& stream, const std::vector<const EntityRegistry*>& registries);
void SerializeRegistries(std::ostream& stream, const EntityRegistry* registries, const size_t size);

void DeserializeRegistries(std::istream& stream, std::vector<std::unique_ptr<EntityRegistry>>& outRegistries);