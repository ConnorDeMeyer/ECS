#include "../Headers/System.h"

#include <vector>
#include <memory>

#include "../ImGui/imgui.h"

#include "System/SystemBase.h"

#include "TypeInformation/ECSTypeInformation.h"
#include "TypeInformation/TypeInformation.h"
#include "../Headers/Registry.h"

void GUI::System::UpdateAll()
{
	DrawGlobalSystemList();
}





struct SystemInfo
{
	SystemParameters parameters;
	std::vector<std::pair<uint32_t, std::string>> types;
};

EntityRegistry g_Registry;
std::vector<SystemInfo> g_GlobalSystemInfo;

void GUI::System::DrawGlobalSystemList()
{
	if (g_GlobalSystemInfo.empty())
		RefreshGlobalSystemList();

	ImGui::Begin("All Systems", nullptr, ImGuiWindowFlags_MenuBar);
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Sort"))
			{
				if (ImGui::MenuItem("by name"))
				{
					std::sort(g_GlobalSystemInfo.begin(), g_GlobalSystemInfo.end(),
						[](const SystemInfo& info0, const SystemInfo& info1)
						{
							return info0.parameters.name < info1.parameters.name;
						});
				}
				if (ImGui::MenuItem("by execution time"))
				{
					std::sort(g_GlobalSystemInfo.begin(), g_GlobalSystemInfo.end(),
						[](const SystemInfo& info0, const SystemInfo& info1)
						{
							return info0.parameters.executionTime < info1.parameters.executionTime;
						});
				}
				if (ImGui::MenuItem("by update interval"))
				{
					std::sort(g_GlobalSystemInfo.begin(), g_GlobalSystemInfo.end(),
						[](const SystemInfo& info0, const SystemInfo& info1)
						{
							return info0.parameters.updateInterval < info1.parameters.updateInterval;
						});
				}
				if (ImGui::MenuItem("by type amount"))
				{
					std::sort(g_GlobalSystemInfo.begin(), g_GlobalSystemInfo.end(),
						[](const SystemInfo& info0, const SystemInfo& info1)
						{
							return info0.types.size() < info1.types.size();
						});
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		for (auto& systemInfo : g_GlobalSystemInfo)
		{
			ImGui::PushID(systemInfo.parameters.name.c_str());

			if (ImGui::Button("Add"))
			{
				Registry::AddSystem(systemInfo.parameters.name);
			}
			ImGui::SameLine();
			if (ImGui::Button("Remove"))
			{
				Registry::RemoveSystem(systemInfo.parameters.name);
			}
			ImGui::SameLine();
			ImGui::Text(systemInfo.parameters.name.c_str());
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();

				ImGui::Text(systemInfo.parameters.name.c_str());
				ImGui::Text("ExecutionTime: %d", systemInfo.parameters.executionTime);
				ImGui::Text("Update Interval: %.3f", systemInfo.parameters.updateInterval);
				ImGui::Text("Types:");

				ImGui::BeginChild("SysTypes", ImVec2(ImGui::GetContentRegionAvail().x, float(systemInfo.types.size()) * 24.f), true, ImGuiWindowFlags_NoScrollbar);
				for (auto& type : systemInfo.types)
				{
					ImGui::Text(type.second.c_str());
				}
				ImGui::EndChild();

				ImGui::EndTooltip();
			}

			ImGui::PopID();
		}

	}
	ImGui::End();
}

void GUI::System::RefreshGlobalSystemList()
{
	for (auto& SystemAdder : ECSTypeInformation::GetSystemAdders())
	{
		auto system = SystemAdder.second(&g_Registry);

		std::vector<std::pair<uint32_t, std::string>> typeInfos;
		auto typeIds = system->GetTypeIds();
		for (auto& typeId : typeIds)
			typeInfos.emplace_back(typeId, std::string(TypeInformation::GetTypeName(typeId)));

		g_GlobalSystemInfo.emplace_back(system->GetSystemParameters(), std::move(typeInfos));

		g_Registry.RemoveSystem(system->GetSystemParameters().name);
	}
}
