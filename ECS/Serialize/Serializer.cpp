#include "Serializer.h"

#include "../Registry/EntityRegistry.h"

void SerializeRegistries(std::ostream& stream, const std::vector<const EntityRegistry*>& registries)
{
	SerializeRegistries(stream, *registries.data(), registries.size());
}

void SerializeRegistries(std::ostream& stream, const EntityRegistry* registries, const size_t size)
{
	const uint32_t architecture = sizeof(size_t) * 8;
	WriteStream(stream, architecture);

	WriteStream(stream, size);

	for (size_t i{}; i < size; ++i)
	{
		registries[i].Serialize(stream);
	}
}

void DeserializeRegistries(std::istream& stream, std::vector<std::unique_ptr<EntityRegistry>>& outRegistries)
{
	uint32_t architecture{};
	ReadStream(stream, architecture);

	if (architecture != sizeof(size_t) * 8)
		throw std::runtime_error("File does not match architecture");

	uint32_t registryAmount{};
	ReadStream(stream, registryAmount);

	for (size_t i{}; i < registryAmount; ++i)
	{
		auto& registry = outRegistries.emplace_back(new EntityRegistry());
		registry->Deserialize(stream);
	}
}
