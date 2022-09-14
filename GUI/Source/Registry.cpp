#include "../Headers/Registry.h"
#include "../Headers/Entities.h"

#include <cctype>

#include "Registry/EntityRegistry.h"
#include "Registry/TypeView.h"

#include "../ImGui/imgui.h"
#include "../ImGui/ImPlot/implot.h"
#include "../ImGui/imgui_internal.h"

void GUI::Registry::UpdateAll()
{
	DrawEntityRegistryInfo();
	DrawTypeViews();
	DrawEntities();
	DrawTypeBindings();
	DrawSystems();
	DrawProfiler();
}


std::vector<std::pair<EntityRegistry*, std::string>> g_RegisteredRegistries;

EntityRegistry* g_pSelectedRegistry;
std::vector<TypeViewInfo> g_SelectedRegistryInfos;

void GUI::Registry::DrawEntityRegistryInfo()
{
	ImGui::Begin("Entity Registry Information", nullptr);
	{
		if (ImGui::BeginTabBar("regList"))
		{
			for (auto& registeredReg : g_RegisteredRegistries)
				if (ImGui::BeginTabItem(registeredReg.second.c_str()))
				{
					g_pSelectedRegistry = registeredReg.first;
					ImGui::EndTabItem();
				}
			ImGui::EndTabBar();
		}

		if (g_pSelectedRegistry)
		{
			ImGui::Text("Amount of Entities: %u", g_pSelectedRegistry->GetEntities().size());
			ImGui::Text("Amount of Type Views: %u", g_pSelectedRegistry->GetTypeViews().size());
			ImGui::Text("Amount of Type Bindings: %u", g_pSelectedRegistry->GetTypeBindings().size());
			ImGui::Text("Amount of Systems: %u", g_pSelectedRegistry->GetSystems().size());

			auto context = ImGui::GetCurrentContext();
			auto& io = context->IO;

			ImGui::Separator();
			float deltaTime =  1.f / io.Framerate;
			static bool isPaused{ true };
			ImGui::Text("Delta Time: [%.3f]", deltaTime);
			ImGui::Text("Frame Rate: [%.1f]", io.Framerate);
			ImGui::Checkbox("Paused", &isPaused);
			if (!isPaused) g_pSelectedRegistry->Update(deltaTime); ///// UPDATE REGISTRY
		}
		else
		{
			ImGui::Text("No Registry Selected");
		}
	}
	ImGui::End();
}

void GUI::Registry::DrawTypeViews()
{
	ImGui::Begin("Type Views", nullptr);
	{
		if (g_pSelectedRegistry)
		{
			ImGui::Text("Type Views");
			if (!g_SelectedRegistryInfos.empty())
			{
				static size_t SelectedView{};
				SelectedView = SelectedView >= g_SelectedRegistryInfos.size() ? 0 : SelectedView;

				if (ImGui::BeginCombo("Type Views", g_SelectedRegistryInfos[SelectedView].typeName.c_str()))
				{
					for (size_t i{}; i < g_SelectedRegistryInfos.size(); ++i)
					{
						if (ImGui::Selectable(g_SelectedRegistryInfos[i].typeName.c_str(), i == SelectedView))
						{
							SelectedView = i;
						}
					}
					ImGui::EndCombo();
				}
				ImGui::Separator();
				{
					auto& viewInfo = g_SelectedRegistryInfos[SelectedView];
					auto view = g_pSelectedRegistry->GetTypeView(viewInfo.typeId);

					ImGui::Text("Type Name: [%s]", viewInfo.typeName.c_str());
					ImGui::Text("Type Id: [%u]", viewInfo.typeId);
					ImGui::Text("Element Amount: [%u]", viewInfo.totalSize);
					ImGui::Text("Element Size: [%u] Bytes", viewInfo.ElementSize);
					ImGui::Text("Total Size: [%u] Bytes", viewInfo.ElementSize * viewInfo.totalSize);
					ImGui::Text("Active Elements: [%u]", viewInfo.activeAmount);
					ImGui::Text("Inactive Elements: [%u]", viewInfo.inactiveAmount);
					if (ImGui::CollapsingHeader("Entities"))
					{
						if (ImGui::BeginTable("EntTabl", int(ImGui::GetContentRegionAvail().x) / 50, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
						{
							for (auto entityId : view->GetRegisteredEntities())
							{
								if (entityId == Entity::InvalidId)
									break;

								char buffer[16];
								sprintf_s(buffer, "%u", entityId);
								ImGui::TableNextColumn();
								if (ImGui::Selectable(buffer))
								{
									Entities::SetEntity(entityId, *g_pSelectedRegistry);
								}
							}
							ImGui::EndTable();
						}
					}
				}
			}
			else
			{
				ImGui::Text("No Views in registry");
			}
		}
		else
		{
			ImGui::Text("No Selected Registry");
		}
	}
	ImGui::End();
}

bool contains(const char* str, const char* query)
{
	const char* str_it{ str };
	const char* query_it{ query };
	while (*str_it != '\0')
	{
		query_it = (*str_it == *query_it) ? ++query_it : query;
		if (*query_it == '\0')
			return true;
		++str_it;
	}
	return false;
}

bool isNum(char val)
{
	return val >= '0' && val <= '9';
}

void GUI::Registry::DrawEntities()
{
	ImGui::Begin("Registry Entities");
	{
		if (g_pSelectedRegistry)
		{
			static char buffer[16]{};
			ImGui::InputText("Search", buffer, 16, ImGuiInputTextFlags_CallbackEdit, [](ImGuiInputTextCallbackData* data)
				{
					if (data->CursorPos && !isNum(data->Buf[data->CursorPos - 1]))
						data->DeleteChars(data->CursorPos - 1, 1);
					return 0;
				});
			ImGui::SameLine();
			if (ImGui::Button("Select"))
			{
				entityId id = entityId(std::stoull(buffer));
				if (g_pSelectedRegistry->GetEntities().contains(id))
				{
					Entities::SetEntity(id, *g_pSelectedRegistry);
				}
			}

			if (ImGui::BeginTable("EntTabl", int(ImGui::GetContentRegionAvail().x) / 50, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
			{
				for (auto entityId : g_pSelectedRegistry->GetEntities())
				{
					char buffer2[16];
					sprintf_s(buffer2, "%u", entityId);
					if (buffer[0] == '\0' || (buffer[0] != '\0' && contains(buffer2, buffer)))
					{
						ImGui::TableNextColumn();
						if (ImGui::Selectable(buffer2))
						{
							Entities::SetEntity(entityId, *g_pSelectedRegistry);
						}
					}
				}
				ImGui::EndTable();
			}
		}
	}
	ImGui::End();
}

void GUI::Registry::DrawTypeBindings()
{
	ImGui::Begin("Type Bindings");
	{
		if (g_pSelectedRegistry)
		{
			auto& bindings{ g_pSelectedRegistry->GetTypeBindings() };

			auto region = ImGui::GetContentRegionAvail();
			region.y /= 2.f;
			ImGui::BeginChild("Binding Selector", region, true);
			static TypeBinding* pSelectedBinding{};
			static EntityRegistry* pCachedBinding{};
			if (g_pSelectedRegistry != pCachedBinding)
			{
				pSelectedBinding = nullptr;
				pCachedBinding = g_pSelectedRegistry;
			}

			for (auto& binding : bindings)
			{
				ImGui::PushID(binding.get());
				for (size_t i{}; i < binding->GetTypeAmount(); ++i)
				{
					ImGui::Text(TypeInformation::GetTypeName(binding->GetTypeIds()[i]).c_str());
				}
				if (ImGui::Button("Select"))
				{
					pSelectedBinding = binding.get();
				}
				ImGui::Separator();
				ImGui::PopID();
			}
			ImGui::EndChild();

			if (pSelectedBinding)
			{
				for (size_t i{}; i < pSelectedBinding->GetTypeAmount(); ++i)
				{
					ImGui::Text(TypeInformation::GetTypeName(pSelectedBinding->GetTypeIds()[i]).c_str());
				}
				ImGui::Text("Size: [%u]", pSelectedBinding->GetSize());
				if (ImGui::CollapsingHeader("Entities"))
				{
					if (ImGui::BeginTable("EntTablBinding", int(ImGui::GetContentRegionAvail().x) / 50, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
					{
						for (auto entityId : pSelectedBinding->GetEntities())
						{
							if (entityId.first == Entity::InvalidId)
								break;

							char buffer[16];
							sprintf_s(buffer, "%u", entityId.first);
							ImGui::TableNextColumn();
							if (ImGui::Selectable(buffer))
							{
								Entities::SetEntity(entityId.first, *g_pSelectedRegistry);
							}
						}
						ImGui::EndTable();
					}
				}

			}

		}
	}
	ImGui::End();
}

void GUI::Registry::DrawSystems()
{
	ImGui::Begin("Registry Systems", nullptr, ImGuiWindowFlags_MenuBar);
	{
		if (g_pSelectedRegistry)
		{
			static bool View_ExecutionTime{ true };

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("Options"))
				{
					if (ImGui::MenuItem("Disable All"))
					{
						for (auto& sys : g_pSelectedRegistry->GetSystems())
							sys->Disable();
					}
					if (ImGui::MenuItem("Enable All"))
					{
						for (auto& sys : g_pSelectedRegistry->GetSystems())
							sys->Enable();
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("View"))
				{
					if (ImGui::MenuItem("Execution Time", 0, View_ExecutionTime))
						View_ExecutionTime = !View_ExecutionTime;
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			auto& systems = g_pSelectedRegistry->GetSystems();

			int32_t lastExecutionTime{ systems.empty() ? 0 : systems.begin()->get()->GetSystemParameters().executionTime + 1 };

			for (auto& system : systems)
			{
				ImGui::PushID(system.get());

				const int32_t executionTime{ system->GetSystemParameters().executionTime };
				if (View_ExecutionTime && lastExecutionTime != executionTime)
				{
					lastExecutionTime = executionTime;
					ImGui::Text("Execution Time: [%i]", executionTime);
				}

				if (system->IsEnabled())
				{
					if (ImGui::Button("Disable", ImVec2(70, 0)))
					{
						system->Disable();
					}
					ImGui::SameLine();
					ImGui::Text(system->GetSystemParameters().name.c_str());
				}
				else
				{
					if (ImGui::Button("Enable", ImVec2(70, 0)))
					{
						system->Enable();
					}
					ImGui::SameLine();
					ImGui::TextDisabled(system->GetSystemParameters().name.c_str());
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();

					ImGui::Text(system->GetSystemParameters().name.c_str());
					ImGui::Text("ExecutionTime: %d", system->GetSystemParameters().executionTime);
					ImGui::Text("Update Interval: %.3f", system->GetSystemParameters().updateInterval);
					ImGui::Text("Types:");

					ImGui::BeginChild("SysTypes", ImVec2(ImGui::GetContentRegionAvail().x, float(system->GetTypeIds().size()) * 24.f), true, ImGuiWindowFlags_NoScrollbar);
					for (auto& type : system->GetTypeIds())
					{
						ImGui::Text( TypeInformation::GetTypeName(type).c_str());
					}
					ImGui::EndChild();

					ImGui::EndTooltip();
				}

				ImGui::PopID();

			}
		}
	}
	ImGui::End();
}

void GUI::Registry::DrawProfiler()
{
	ImGui::Begin("Registry Profiler", nullptr, ImGuiWindowFlags_MenuBar);
	{
		if (g_pSelectedRegistry)
		{
			auto& profiler = g_pSelectedRegistry->GetProfilerInfo();

			constexpr size_t BufferSize{ 64 };

			static std::vector<const EntityRegistry::ProfilerInfo*> SortedProfiles;
			static std::vector<char> CharBuffer;
			static std::vector<const char*> stringLocations;
			static std::vector<float> PlotValuesTotal;
			static std::vector<float> PlotValuesIndividual;
			static std::function<bool(const EntityRegistry::ProfilerInfo*, const EntityRegistry::ProfilerInfo*)> ProfilerSorter;
			static std::vector<double> Positions;
			const char* groupName[]{ "Execution Time" };

			CharBuffer.resize(profiler.size() * BufferSize);

			SortedProfiles.clear();
			for (auto& profile : profiler)
				SortedProfiles.emplace_back(&profile.second);

			if (ProfilerSorter)
				std::sort(SortedProfiles.begin(), SortedProfiles.end(), ProfilerSorter);
			
			size_t count{};
			PlotValuesTotal.resize(SortedProfiles.size());
			PlotValuesIndividual.resize(SortedProfiles.size());
			for (auto& profile : SortedProfiles)
			{
				PlotValuesTotal[count] = std::chrono::duration<float>(profile->timeToExecuteSystem).count();
				PlotValuesIndividual[count] = std::chrono::duration<float>(profile->timeToExecutePerComponent).count();
				sprintf_s(CharBuffer.data() + count * BufferSize, BufferSize, "%s", profile->system->GetSystemParameters().name.c_str());
				++count;
			}

			if (count >= stringLocations.size())
			{
				stringLocations.resize(count);
				Positions.resize(count);
				for (size_t i{}; i < count; ++i)
				{
					stringLocations[i] = CharBuffer.data() + i * BufferSize;
					Positions[i] = double(i);
				}
			}

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("Sort"))
				{
					if (ImGui::MenuItem("by name"))
					{
						ProfilerSorter = [](const EntityRegistry::ProfilerInfo* info0, const EntityRegistry::ProfilerInfo* info1)
						{
							return info0->system->GetSystemParameters().name < info1->system->GetSystemParameters().name;
						};
					}
					if (ImGui::MenuItem("by total time"))
					{
						ProfilerSorter = [](const EntityRegistry::ProfilerInfo* info0, const EntityRegistry::ProfilerInfo* info1)
						{
							return info0->timeToExecuteSystem < info1->timeToExecuteSystem;
						};
					}
					if (ImGui::MenuItem("by individual time"))
					{
						ProfilerSorter = [](const EntityRegistry::ProfilerInfo* info0, const EntityRegistry::ProfilerInfo* info1)
						{
							return info0->timeToExecutePerComponent < info1->timeToExecutePerComponent;
						};
					}
					if (ImGui::MenuItem("by execution time"))
					{
						ProfilerSorter = [](const EntityRegistry::ProfilerInfo* info0, const EntityRegistry::ProfilerInfo* info1)
						{
							return info0->system->GetSystemParameters().executionTime < info1->system->GetSystemParameters().executionTime;
						};
					}
					if (ImGui::MenuItem("by update interval"))
					{
						ProfilerSorter = [](const EntityRegistry::ProfilerInfo* info0, const EntityRegistry::ProfilerInfo* info1)
						{
							return info0->system->GetSystemParameters().updateInterval < info1->system->GetSystemParameters().updateInterval;
						};
					}
					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			if (ImPlot::BeginPlot("Total time to execute System"))
			{
				ImPlot::SetupAxes( "Time", "System", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
				ImPlot::SetupAxisTicks(ImAxis_Y1, Positions.data(), int(Positions.size()), stringLocations.data());
				ImPlot::PlotBarGroups(groupName, PlotValuesTotal.data(), 1, int(PlotValuesTotal.size()), 0.67, 0, ImPlotBarGroupsFlags_Horizontal);
				ImPlot::EndPlot();
			}

			if (ImPlot::BeginPlot("Time to Execute per component"))
			{
				ImPlot::SetupAxes("Time", "System", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
				ImPlot::SetupAxisTicks(ImAxis_Y1, Positions.data(), int(Positions.size()), stringLocations.data());
				ImPlot::PlotBarGroups(groupName, PlotValuesIndividual.data(), 1, int(PlotValuesIndividual.size()), 0.67, 0, ImPlotBarGroupsFlags_Horizontal);
				ImPlot::EndPlot();
			}

			for (auto info : SortedProfiles) // Make sure SYSTEM_PROFILER is defined in all projects
			{
				ImGui::Text("System name: %s", info->system->GetSystemParameters().name.c_str());
				ImGui::Text("Times Executed: [%u]", info->timesExecuted);
				ImGui::Text("Time to Execute: [%i ms]", info->timeToExecuteSystem.count());
				ImGui::Text("Time to Execute Per component: [%i us]", info->timeToExecutePerComponent.count());
				ImGui::Separator();
			}
		}
	}
	ImGui::End();
}

void GUI::Registry::SetEntityRegistry(EntityRegistry* pRegistry)
{
	g_pSelectedRegistry = pRegistry;
	if (g_RegisteredRegistries.end() == std::find_if(g_RegisteredRegistries.begin(), g_RegisteredRegistries.end(),
		[pRegistry](std::pair<EntityRegistry*, std::string>& element) {return element.first == pRegistry; }))
	{
		RegisterEntityRegistry(pRegistry);
	}
	RefreshRegistryInfo();
}

EntityRegistry* GUI::Registry::GetEntityRegistry()
{
	return g_pSelectedRegistry;
}

void GUI::Registry::RegisterEntityRegistry(EntityRegistry* pRegistry)
{
	auto it = std::find_if(g_RegisteredRegistries.begin(), g_RegisteredRegistries.end(), [pRegistry](const std::pair<EntityRegistry*, std::string>& element)
		{
			return element.first == pRegistry;
		});
	if (it == g_RegisteredRegistries.end())
		g_RegisteredRegistries.emplace_back(pRegistry, std::string{ std::to_string((size_t)pRegistry) });
}

void GUI::Registry::RegisterEntityRegistry(EntityRegistry* pRegistry, const char* name)
{
	auto it = std::find_if(g_RegisteredRegistries.begin(), g_RegisteredRegistries.end(), [pRegistry](const std::pair<EntityRegistry*, std::string>& element)
		{
			return element.first == pRegistry;
		});
	if (it == g_RegisteredRegistries.end())
	{
		g_RegisteredRegistries.emplace_back(pRegistry, std::string{ name });
	}
	else
	{
		it->second = name;
	}
}

void GUI::Registry::RemoveRegisteredRegistry(EntityRegistry* pRegistry)
{
	//g_RegisteredRegistries.erase(std::remove(g_RegisteredRegistries.begin(), g_RegisteredRegistries.end(), pRegistry), g_RegisteredRegistries.end());
	g_RegisteredRegistries.erase(std::remove_if(g_RegisteredRegistries.begin(), g_RegisteredRegistries.end(), 
		[pRegistry](const std::pair<EntityRegistry*, std::string>& element)
		{
			return element.first == pRegistry;
		}), g_RegisteredRegistries.end());
}

void GUI::Registry::RefreshRegistryInfo()
{
	if (g_pSelectedRegistry)
	{
		RefreshTypeViews();
	}
}

void GUI::Registry::RefreshTypeViews()
{
	if (g_pSelectedRegistry)
	{
		g_SelectedRegistryInfos.clear();
		for (auto& view : g_pSelectedRegistry->GetTypeViews())
		{
			g_SelectedRegistryInfos.emplace_back(view.second->GetInfo());
		}
	}
}

void GUI::Registry::AddSystem(const std::string& name)
{
	if (g_pSelectedRegistry)
	{
		g_pSelectedRegistry->AddSystem(name);
	}
}

void GUI::Registry::RemoveSystem(const std::string& name)
{
	if (g_pSelectedRegistry)
	{
		g_pSelectedRegistry->RemoveSystem(name);
	}
}

