#include "../Headers/Entities.h"
#include "../Headers/Registry.h"

#include "Entity/Entity.h"

#include "../ImGui/imgui.h"
#include "../ImGui/ImGuiExt/imgui_helpers.h"
#include "Registry/EntityRegistry.h"

void GUI::Entities::UpdateAll()
{
	DrawEntity();
	DrawComponentInfos();
}

entityId g_EntityId{ Entity::InvalidId };
EntityRegistry* g_pEntityRegistry;

std::vector<std::pair<uint32_t, std::string>> g_Components;
std::unordered_map<uint32_t, bool> g_OpenedComponentWindows;

#define FILL_IMGUI_FUNC(Type) {reflection::type_id<Type>(), [](const char* label, void* data) {return ImGui::CustomInput(label, *static_cast<Type*>(data)); }}
const std::unordered_map<uint32_t, std::function<bool(const char* label, void* data)>> g_ImGuiEditFunctions{
	FILL_IMGUI_FUNC(short),
	FILL_IMGUI_FUNC(unsigned short),
	FILL_IMGUI_FUNC(int),
	FILL_IMGUI_FUNC(unsigned int),
	FILL_IMGUI_FUNC(long long),
	FILL_IMGUI_FUNC(unsigned long long),
	FILL_IMGUI_FUNC(bool),
	FILL_IMGUI_FUNC(char),
	FILL_IMGUI_FUNC(unsigned char),
	FILL_IMGUI_FUNC(float),
	FILL_IMGUI_FUNC(double),

	FILL_IMGUI_FUNC(glm::vec2),
	FILL_IMGUI_FUNC(glm::vec3),
	FILL_IMGUI_FUNC(glm::vec4),
	FILL_IMGUI_FUNC(glm::ivec2),
	FILL_IMGUI_FUNC(glm::ivec3),
	FILL_IMGUI_FUNC(glm::ivec4),
	FILL_IMGUI_FUNC(glm::mat2x2),
	FILL_IMGUI_FUNC(glm::mat2x3),
	FILL_IMGUI_FUNC(glm::mat2x4),
	FILL_IMGUI_FUNC(glm::mat3x2),
	FILL_IMGUI_FUNC(glm::mat3x3),
	FILL_IMGUI_FUNC(glm::mat3x4),
	FILL_IMGUI_FUNC(glm::mat4x2),
	FILL_IMGUI_FUNC(glm::mat4x3),
	FILL_IMGUI_FUNC(glm::mat4x4),

	FILL_IMGUI_FUNC(SDL_Rect),
	FILL_IMGUI_FUNC(SDL_Rect),
	FILL_IMGUI_FUNC(SDL_Color),
};

std::vector<std::pair<uint32_t, std::string>> g_AvailableComponents;

void GUI::Entities::DrawEntity()
{
	ImGui::Begin("Entity");
	{
		if (ImGui::Button("Refresh")) RefreshEntityInfo();

		if (g_pEntityRegistry && g_EntityId != Entity::InvalidId)
		{
			ImGui::Text("EntityId: [%u]", g_EntityId);
			ImGui::Text("Components:");
			ImGui::Indent();
			for (auto& component : g_Components)
			{
				//bool isEnabled{g_pEntityRegistry.}

				ImGui::PushID(int(component.first));
				//if (ImGui::Checkbox(""))
				if (ImGui::Button("Remove"))
				{
					g_pEntityRegistry->RemoveComponentInstantly(component.first, g_EntityId);
					RefreshEntityInfo();
					ImGui::PopID();
					break;
				}
				ImGui::SameLine();

				if (ImGui::Button(component.second.c_str()))
				{
					g_OpenedComponentWindows[component.first] = true;
				}
				ImGui::PopID();
			}
			ImGui::Unindent();

			ImGui::Separator();
			ImGui::Text("Add Component");
			for (auto& component : g_AvailableComponents)
			{
				if (ImGui::Button(component.second.c_str()))
				{
					g_pEntityRegistry->AddComponentInstantly(component.first, g_EntityId);
					RefreshEntityInfo();
					break;
				}
			}
		}
	}
	ImGui::End();
}

void GUI::Entities::DrawComponentInfos()
{
	for (auto& windowOpen : g_OpenedComponentWindows)
	{
		if (windowOpen.second)
			DrawComponentInfo(windowOpen.first, &windowOpen.second);
	}
}

void DrawField(const ClassFieldInfo& fieldInfo, void* data)
{
	const auto it = g_ImGuiEditFunctions.find(fieldInfo.typeId);
	if (it != g_ImGuiEditFunctions.end())
	{
		it->second(fieldInfo.name.c_str(), static_cast<uint8_t*>(data) + fieldInfo.offset);
	}
	else
	{
		ImGui::Indent();
		for (auto& subInfo : TypeInformation::GetFieldInfo(fieldInfo.typeId))
		{
			DrawField(subInfo.second, static_cast<uint8_t*>(data) + fieldInfo.offset);
		}
		ImGui::Unindent();
	}
}

void GUI::Entities::DrawComponentInfo(uint32_t component, bool* window)
{
	ImGui::Begin(TypeInformation::GetTypeName(component).c_str(), window, ImGuiWindowFlags_AlwaysAutoResize);
	{
		auto& typeViews = Registry::GetEntityRegistry()->GetTypeViews();
		auto it = typeViews.find(component);
		if (it != typeViews.end() && it->second->Contains(g_EntityId))
		{
			auto& fieldInfos = TypeInformation::GetFieldInfo(component);
			auto reference = it->second->GetVoidReference(g_EntityId);
			for (auto& fieldInfo : fieldInfos)
				DrawField(fieldInfo.second, reference.Data());

			if (fieldInfos.empty())
			{
				ImGui::Text("No Field info found in this Component");
				ImGui::TextWrapped("Add the method [void RegisterMemberInfo(ClassMemberAdder& adder)] to the Component");
				ImGui::TextWrapped("use the RegisterMemberVar of adder and with the field: [adder.RegisterMemberVar(field);]");
			}
		}
	}
	ImGui::End();
}

void GUI::Entities::SetEntity(Entity* pEntity)
{
	if (pEntity)
	{
		g_EntityId = pEntity->GetId();
		g_pEntityRegistry = &pEntity->GetRegistry();
		RefreshEntityInfo();
	}
	else
	{
		g_EntityId = Entity::InvalidId;
		g_pEntityRegistry = nullptr;
	}
	RefreshEntityInfo();
}

void GUI::Entities::SetEntity(entityId id, EntityRegistry& registry)
{
	if (id != Entity::InvalidId)
	{
		g_EntityId = id;
		g_pEntityRegistry = &registry;
	}
	RefreshEntityInfo();
}

entityId GUI::Entities::GetEntityId()
{
	return g_EntityId;
}

EntityRegistry* GUI::Entities::GetEntityRegistry()
{
	return g_pEntityRegistry;
}

void GUI::Entities::RefreshEntityInfo()
{
	g_Components.clear();
	if (g_pEntityRegistry && g_EntityId != Entity::InvalidId)
	{
		for (auto& view : g_pEntityRegistry->GetTypeViews())
		{
			if (view.second->Contains(g_EntityId))
			{
				g_Components.emplace_back(view.first, TypeInformation::GetTypeName(view.first));
			}
		}
	}

	RefreshAvailableComponents();

	g_OpenedComponentWindows.clear();
}

void GUI::Entities::RefreshAvailableComponents()
{
	g_AvailableComponents.clear();

	if (g_pEntityRegistry && g_EntityId != Entity::InvalidId)
	{
		for (auto& info : TypeInformation::GetAllTypeInformation())
			g_AvailableComponents.emplace_back(info.first, info.second.m_TypeName);

		const size_t size{ g_AvailableComponents.size() };
		for (size_t i{}; i < size; ++i)
		{
			const size_t pos{ size - i - 1 };

			for (auto& component : g_Components)
			{
				if (component.first == g_AvailableComponents[pos].first)
				{
					g_AvailableComponents[pos] = std::move(g_AvailableComponents.back());
					g_AvailableComponents.pop_back();
					break;
				}
			}
		}
	}
}
