// copyright 2021 Russ 'trdwll' Treadwell <trdwll.com> - Licensed under MIT

#include "Developer/CDebugToolsActor.h"

#include "Algo/Reverse.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GenericPlatform/GenericPlatformDriver.h"
#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformMisc.h"
#elif PLATFORM_LINUX
#include "Linux/LinuxPlatformMisc.h"
#elif PLATFORM_MAC
#include "Mac/MacPlatformMisc.h"
#endif

ACDebugToolsActor::ACDebugToolsActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACDebugToolsActor::BeginPlay()
{
	Super::BeginPlay();

#if WITH_IMGUI

	FetchActors();

	m_ImguiTickHandle = FImGuiModule::Get().AddWorldImGuiDelegate(FImGuiDelegate::CreateUObject(this, &ACDebugToolsActor::ImguiTick));

#endif
}

void ACDebugToolsActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetOwner() != nullptr)
	{
		m_CurrentPlayerLocation = GetOwner()->GetActorLocation();
	}

#if WITH_IMGUI
#endif
}

void ACDebugToolsActor::BeginDestroy()
{
	Super::BeginDestroy();
}

void ACDebugToolsActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

#if WITH_IMGUI

	FImGuiModule::Get().RemoveImGuiDelegate(m_ImguiTickHandle);

#endif
}

#if WITH_IMGUI

void ACDebugToolsActor::ImguiTick()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("World"))
		{
			ImGui::MenuItem("Inspector", NULL, &m_bInspector);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Tools"))
		{
			ImGui::MenuItem("Profiler");
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Overlays"))
		{
			ImGui::MenuItem("Stats", NULL, &m_bDisplayStats);
			ImGui::MenuItem("System Specs", NULL, &m_bDisplaySystemSpecs);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Demo"))
		{
			FImGuiModule::Get().ToggleShowDemo();
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	CreateOverlay();

	if (m_bInspector)
	{
		InspectorWindow();
	}
}

void ACDebugToolsActor::FetchActors()
{
	m_ActorsInLevel.Reset();
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		m_ActorsInLevel.Add(*It);
	}
}

void ACDebugToolsActor::CreateOverlay()
{
	if (m_bDisplayStats)
	{
		extern ENGINE_API float GAverageFPS;
		extern ENGINE_API float GAverageMS;
		extern ENGINE_API uint32 GGPUFrameTime;
		extern RENDERCORE_API uint32 GRenderThreadTime;
		extern RENDERCORE_API uint32 GGameThreadTime;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

		ImGui::SetNextWindowBgAlpha(0.35f);
		if (ImGui::Begin("Game Info", NULL, window_flags))
		{
			ImGui::Text("FPS: %f (%fms)", GAverageFPS, GAverageMS);
			ImGui::Text("GPU: %fms", FPlatformTime::ToMilliseconds(GGPUFrameTime));
			ImGui::Text("Render Thread: %fms", FPlatformTime::ToMilliseconds(GRenderThreadTime));
			ImGui::Text("Game Thread: %fms", FPlatformTime::ToMilliseconds(GGameThreadTime));
			float Loc[] = {m_CurrentPlayerLocation.X, m_CurrentPlayerLocation.Y, m_CurrentPlayerLocation.Z};
			ImGui::InputFloat3("Current Location", Loc);
			//ImGui::Text("Current Location: %s", TCHAR_TO_UTF8(*GetOwner()->GetActorLocation().ToString()));
		}

		static float values[2000] = {0};
		static int values_offset = 0;
		values[values_offset] = GAverageFPS;
		values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);

		char overlay[32];
		sprintf(overlay, "FPS %f", GAverageFPS);
		ImGui::PlotLines("", values, IM_ARRAYSIZE(values), values_offset, overlay, -1.0f, 2500.0f, ImVec2(0, 80));

		ImGui::End();
	}

	if (m_bDisplaySystemSpecs)
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

		ImGui::SetNextWindowBgAlpha(0.35f);
		if (ImGui::Begin("System Info", NULL, window_flags))
		{
			ImGui::Text("OS Info: %s (%s)", TCHAR_TO_UTF8(*GetOperatingSystem()), TCHAR_TO_UTF8(*GetOSVersion()));
			ImGui::Text("CPU Brand: %s", TCHAR_TO_UTF8(*GetCPUBrandName()));
			ImGui::Text("CPU Vendor: %s", TCHAR_TO_UTF8(*GetCPUVendorName()));
			ImGui::Text("CPU Cores: %d", GetCPUCores());
			ImGui::Text("GPU Brand: %s", TCHAR_TO_UTF8(*GetGPUBrandName()));
			ImGui::Text("GPU Driver: %s", TCHAR_TO_UTF8(*GetGPUDriverInfo()));
			ImGui::Text("Ram Usage: %lluMB / %lluGB", GetMemoryStats().UsedPhysical / 1024 / 1024, GetMemoryStats().TotalPhysical / 1024 / 1024 / 1024);
		}

		ImGui::End();
	}
}

template <uint8 T>
void ACDebugToolsActor::InspectorObjectProperties(AActor* Actor)
{
	UClass* Class = Actor->GetClass();
	for (TFieldIterator<FProperty> PropertyIterator(Class); PropertyIterator; ++PropertyIterator)
	{
		FProperty* Property = *PropertyIterator;
		FName const PropertyName = Property->GetFName();

		if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
		{
			if (T == 1)
			{
				ImGui::Text(TCHAR_TO_UTF8(*PropertyName.ToString()));
				ImGui::SameLine();
				bool bewl = BoolProperty->GetPropertyValue(Class);
				ImGui::Checkbox("", &bewl);
			}
		}
		else if (FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
		{
			if (T == 2)
			{
				ImGui::Text(TCHAR_TO_UTF8(*PropertyName.ToString()));
				ImGui::SameLine();
				if (NumericProperty->IsFloatingPoint())
				{
					float f = NumericProperty->GetFloatingPointPropertyValue(Property);
					ImGui::InputFloat("", &f);
				}
				else if (NumericProperty->IsInteger())
				{
					int32 i = (int32)NumericProperty->GetSignedIntPropertyValue(Property);
					ImGui::InputInt("", &i);
				}
			}
		}
		else if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
		{
			if (T == 3)
			{
				ImGui::Text(TCHAR_TO_UTF8(*PropertyName.ToString()));
				ImGui::SameLine();

				//char* items[32];

				UEnum* enoom = EnumProperty->GetEnum();

				FString EnumName;
				enoom->GetName(EnumName);

				TArray<char*> EnumValueNames;
				for (int32 i = 0; i < enoom->GetMaxEnumValue(); i++)
				{
					const FString EnumValue = enoom->GetNameStringByIndex(i);
					//EnumValueNames.Add((char*)EnumValue.GetCharArray().GetData());
					EnumValueNames.Add(TCHAR_TO_UTF8(*EnumValue));
				}

				/*const char* selected = NULL;
				if (ImGui::BeginCombo("", selected))
				{
					for (uint8 i = 0; i < EnumValueNames.Num(); i++)
					{
						bool bIsSelected = (selected == EnumValueNames[0]);
						if (ImGui::Selectable(EnumValueNames[i], bIsSelected))
						{
							selected = EnumValueNames[i];
						}
						if (bIsSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}*/

				int32 selected = 1;
				ImGui::Combo("", &selected, EnumValueNames.GetData(), EnumValueNames.Num());
			}
		}
		else if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
		{
			if (T == 4)
			{
				ImGui::Text(TCHAR_TO_UTF8(*PropertyName.ToString()));
			}
		}
	}
}

void ACDebugToolsActor::InspectorWindow()
{
	if (ImGui::Begin("Inspector", &m_bInspector))
	{
		if (ImGui::Button("Fetch Actors"))
		{
			FetchActors();
		}

		ImGui::SameLine();
		char buffer[100] = "Search";
		ImGui::Text("Search");
		ImGui::SameLine();
		if (ImGui::InputText("", buffer, IM_ARRAYSIZE(buffer)))
		{

		}

		ImGui::BeginChild("left pane", ImVec2(300, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
		if (ImGui::TreeNodeEx((void*)(intptr_t)0, ImGuiTreeNodeFlags_DefaultOpen, "AActor (%d)", m_ActorsInLevel.Num()))
		{
			FActorHierachy hierarchy;
			hierarchy.CurrentClass = AActor::StaticClass();
			for (AActor* Actor : m_ActorsInLevel)
			{
				UClass* Klass = Actor->GetClass();
				TArray<UClass*> Klasses;

				while (Klass != AActor::StaticClass())
				{
					Klasses.Add(Klass);
					if (Klass == nullptr)
					{
						break;
					}
					Klass = Klass->GetSuperClass();
				}

				hierarchy.Add(Actor, Klasses);
			}

			Traverse(hierarchy);

			ImGui::TreePop();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		{
			ImGui::BeginGroup();
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));  // Leave room for 1 line below us

			ImGui::Separator();
			if (ImGui::BeginTabBar("##InspectorTabs", ImGuiTabBarFlags_None))
			{
				if (ImGui::BeginTabItem("Properties"))
				{
					ImGui::Text("Item Name: %s", TCHAR_TO_UTF8(*m_SelectedInspectorItem.Name));
					if (m_SelectedInspectorItem.Actor != nullptr)
					{
						const FVector ActorLoc = m_SelectedInspectorItem.Actor->GetActorLocation();
						float Loc[] = {ActorLoc.X, ActorLoc.Y, ActorLoc.Z};
						ImGui::Text("Location");
						ImGui::SameLine();
						ImGui::SliderFloat3("", Loc, -50000.0f, 50000.0f);
						ImGui::SameLine();
						if (ImGui::Button("Copy"))
						{
							ImGui::SetClipboardText(TCHAR_TO_UTF8(*ActorLoc.ToString()));
						}

						ImGui::Spacing();
						ImGui::Spacing();

						if (ImGui::BeginTabBar("##InspectorPropertyTabs", ImGuiTabBarFlags_None))
						{
							if (ImGui::BeginTabItem("Bools"))
							{
								InspectorObjectProperties<1>(m_SelectedInspectorItem.Actor);

								ImGui::EndTabItem();
							}

							if (ImGui::BeginTabItem("Numbers"))
							{
								InspectorObjectProperties<2>(m_SelectedInspectorItem.Actor);

								ImGui::EndTabItem();
							}

							if (ImGui::BeginTabItem("Enums"))
							{
								InspectorObjectProperties<3>(m_SelectedInspectorItem.Actor);

								ImGui::EndTabItem();
							}

							if (ImGui::BeginTabItem("Objects"))
							{
								ImGui::Text("Unable to visualize these members, but we're listing them.");
								ImGui::Spacing();
								ImGui::Spacing();
								InspectorObjectProperties<4>(m_SelectedInspectorItem.Actor);

								ImGui::EndTabItem();
							}

							ImGui::EndTabBar();
						}
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Details"))
				{
					ImGui::Text("LifeTime: %f", m_SelectedInspectorItem.Lifetime);
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::EndChild();
			ImGui::EndGroup();
		}
	}
	ImGui::End();
}

void ACDebugToolsActor::Traverse(const FActorHierachy& hier)
{
	int32 i = 0;
	for (const auto& v : hier.ChildNodes)
	{
		if (v.Key == nullptr)
		{
			break;
		}

		FString ClassName;
		v.Key->GetName(ClassName);
		if (ImGui::TreeNode((void*)(intptr_t)i, "%s (%d)", TCHAR_TO_UTF8(*ClassName), v.Value->Children.Num()))
		{
			if (ImGui::IsItemClicked())
			{
				m_SelectedInspectorItem.Name = FString::Printf(TEXT("%s (%d)"), *ClassName, v.Value->Children.Num());
				m_SelectedInspectorItem.Lifetime = -1;
				m_SelectedInspectorItem.Actor = nullptr;
			}

			if (v.Value->ChildNodes.Num() > 0)
			{
				Traverse(*v.Value);
			}
			else if (v.Value->Children.Num() > 0)
			{
				int32 j = 0;
				for (AActor* ac : v.Value->Children)
				{
					FString ClassName2;
					ac->GetName(ClassName2);
					const FString Name = FString::Printf(TEXT("%s"), *ClassName2);
					if (ImGui::Selectable(TCHAR_TO_UTF8(*Name), m_SelectedInspectorItem.Name == Name))
					{
						m_SelectedInspectorItem.Name = Name;
						m_SelectedInspectorItem.Lifetime = ac->GetGameTimeSinceCreation();
						m_SelectedInspectorItem.Actor = ac;
					}
					j++;
				}
			}

			ImGui::TreePop();
		}

		i++;
	}
}

#endif
