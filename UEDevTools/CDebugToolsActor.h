// copyright 2021 Russ 'trdwll' Treadwell <trdwll.com> - Licensed under MIT

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ImGUIBlueprintLibrary/Public/ImGuiCommon.h"

#include "CDebugToolsActor.generated.h"

UCLASS()
class ACDebugToolsActor : public AActor
{
	GENERATED_BODY()

public:
	ACDebugToolsActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void BeginDestroy() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FVector m_CurrentPlayerLocation;

#if WITH_IMGUI
	void ImguiTick();


	void FetchActors();

	bool m_bInspector;
	bool m_bDisplayStats;
	bool m_bDisplaySystemSpecs;

	TArray<AActor*> m_ActorsInLevel;

	FImGuiDelegateHandle m_ImguiTickHandle;

	void CreateOverlay();
	template<uint8 T>
	void InspectorObjectProperties(AActor* Actor);
	void InspectorWindow();

	struct FActorHierachy
	{
		UClass* CurrentClass;
		TArray<AActor*> Children;
		TMap<UClass*, struct FActorHierachy*> ChildNodes;

		void Add(AActor* actor, TArray<UClass*> klasses)
		{
			Children.Add(actor);

			if (klasses.Num() > 0)
			{
				UClass* klass = klasses.Pop();

				if (!ChildNodes.Contains(klass))
				{
					FActorHierachy* a = new FActorHierachy();
					a->CurrentClass = klass;
					ChildNodes.Add(klass, a);
				}

				FActorHierachy* h = *ChildNodes.Find(klass);
				h->Add(actor, klasses);
			}
		}

		FActorHierachy() {}
	};

	struct FInspectorItem
	{
		FString Name;
		float Lifetime;
		AActor* Actor;

		FInspectorItem() {}
	};

	FInspectorItem m_SelectedInspectorItem;

	void Traverse(const FActorHierachy& hier);

#endif

	/**
	 * Gets the CPU Brand Name information.
	 */
	FString GetCPUBrandName()
	{
#if PLATFORM_WINDOWS
		return FWindowsPlatformMisc::GetCPUBrand();
#elif PLATFORM_LINUX
		return FLinuxPlatformMisc::GetCPUBrand();
#elif PLATFORM_MAC
		return FMacPlatformMisc::GetCPUBrand();
#else
		return FString();
#endif
	}

	/**
	 * Gets the CPU Vendor Name information.
	 */
	FString GetCPUVendorName()
	{
#if PLATFORM_WINDOWS
		return FWindowsPlatformMisc::GetCPUVendor();
#elif PLATFORM_LINUX
		return FLinuxPlatformMisc::GetCPUVendor();
#elif PLATFORM_MAC
		return FMacPlatformMisc::GetCPUVendor();
#else
		return FString();
#endif
	}

	/**
	 * Gets the GPU Brand Name information.
	 */
	FString GetGPUBrandName()
	{
#if PLATFORM_WINDOWS
		return FWindowsPlatformMisc::GetPrimaryGPUBrand();
#elif PLATFORM_LINUX
		return FLinuxPlatformMisc::GetPrimaryGPUBrand();
#elif PLATFORM_MAC
		return FMacPlatformMisc::GetPrimaryGPUBrand();
#else
		return FString();
#endif
	}

	/**
	 * Gets the GPU Driver information.
	 */
	FString GetGPUDriverInfo()
	{
		return GRHIAdapterUserDriverVersion;
	}

	/**
	 * Gets the number of Cores in the Platforms CPU
	 */
	int32 GetCPUCores()
	{
#if PLATFORM_WINDOWS
		return FWindowsPlatformMisc::NumberOfCores();
#elif PLATFORM_LINUX
		return FLinuxPlatformMisc::NumberOfCores();
#elif PLATFORM_MAC
		return FMacPlatformMisc::NumberOfCores();
#else
		return -1;
#endif
	}

	FString GetOSVersion()
	{
#if PLATFORM_WINDOWS
		return FWindowsPlatformMisc::GetOSVersion();
#elif PLATFORM_LINUX
		return FLinuxPlatformMisc::GetOSVersion();
#elif PLATFORM_MAC
		return FMacPlatformMisc::GetOSVersion();
#else
		return FString();
#endif
	}

	FPlatformMemoryStats GetMemoryStats()
	{
#if PLATFORM_WINDOWS
		return FWindowsPlatformMemory::GetStats();
#elif PLATFORM_MAC
		return FMacPlatformMemory::GetStats();
#else
		return {};
#endif
	}

	FString GetOperatingSystem()
	{
#if PLATFORM_WINDOWS
		return "Windows";
#elif PLATFORM_LINUX
		return "Linux";
#elif PLATFORM_MAC
		return "Mac";
#else
		return FString();
#endif
	}
};
