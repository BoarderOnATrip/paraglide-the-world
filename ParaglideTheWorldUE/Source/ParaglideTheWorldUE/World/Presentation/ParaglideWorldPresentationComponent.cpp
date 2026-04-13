// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/Presentation/ParaglideWorldPresentationComponent.h"

#include "Engine/Blueprint.h"
#include "UObject/UnrealType.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "World/Destinations/ParaglideDestinationPack.h"
#include "World/Presentation/ParaglideWorldPresentationActor.h"

UParaglideWorldPresentationComponent::UParaglideWorldPresentationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UParaglideWorldPresentationComponent::OnRegister()
{
	Super::OnRegister();
	RefreshPresentationPath();
}

void UParaglideWorldPresentationComponent::BeginPlay()
{
	Super::BeginPlay();
	RefreshPresentationPath();
}

void UParaglideWorldPresentationComponent::SetPresentationMode(const EParaglideWorldPresentationMode NewMode)
{
	PresentationMode = NewMode;
	RefreshPresentationPath();
}

EParaglideWorldPresentationMode UParaglideWorldPresentationComponent::GetPresentationMode() const
{
	return PresentationMode;
}

bool UParaglideWorldPresentationComponent::ActivateProceduralFallbackPath()
{
	return BuildProceduralFallbackPresentation();
}

bool UParaglideWorldPresentationComponent::ActivateHybridPath()
{
	int32 LoadedChunkCount = 0;
	const bool bHasGaussianPlaceholder = BuildGaussianSplatPlaceholderPresentation(LoadedChunkCount);
	if (bHasGaussianPlaceholder)
	{
		ReleaseProceduralFallbackPresentation();
	}
	const bool bBuiltFallback = bHasGaussianPlaceholder ? false : BuildProceduralFallbackPresentation();
	UpdateReadinessFlags(
		PresentationMode,
		bHasGaussianPlaceholder ? EParaglideWorldPresentationMode::Hybrid : EParaglideWorldPresentationMode::ProceduralFallback,
		bBuiltFallback,
		bHasGaussianPlaceholder,
		bHasGaussianPlaceholder,
		Readiness.TotalChunkCount,
		Readiness.DeclaredGaussianChunkCount,
		SpawnedGaussianActors.Num(),
		Readiness.PrimaryGaussianProvider,
		LoadedChunkCount);
	return bBuiltFallback || bHasGaussianPlaceholder;
}

bool UParaglideWorldPresentationComponent::ActivateGaussianSplatPlaceholderPath()
{
	int32 LoadedChunkCount = 0;
	const bool bHasGaussianPlaceholder = BuildGaussianSplatPlaceholderPresentation(LoadedChunkCount);
	if (bHasGaussianPlaceholder)
	{
		ReleaseProceduralFallbackPresentation();
	}
	const bool bBuiltFallback = bHasGaussianPlaceholder ? false : BuildProceduralFallbackPresentation();
	UpdateReadinessFlags(
		PresentationMode,
		bHasGaussianPlaceholder ? EParaglideWorldPresentationMode::GaussianSplatPlaceholder : EParaglideWorldPresentationMode::ProceduralFallback,
		bBuiltFallback,
		bHasGaussianPlaceholder,
		bHasGaussianPlaceholder,
		Readiness.TotalChunkCount,
		Readiness.DeclaredGaussianChunkCount,
		SpawnedGaussianActors.Num(),
		Readiness.PrimaryGaussianProvider,
		LoadedChunkCount);
	return bBuiltFallback || bHasGaussianPlaceholder;
}

void UParaglideWorldPresentationComponent::RefreshPresentationPath()
{
	AParaglideWorldPresentationActor* PresentationOwner = GetPresentationOwner();
	UParaglideDestinationPack* DestinationPack = PresentationOwner != nullptr ? PresentationOwner->GetLoadedDestinationAsset() : nullptr;

	int32 TotalChunkCount = 0;
	int32 DeclaredGaussianChunkCount = 0;
	EParaglideGaussianPresentationProvider PrimaryGaussianProvider = EParaglideGaussianPresentationProvider::None;
	if (DestinationPack != nullptr)
	{
		for (const FParaglideDestinationPresentationLayer& Layer : DestinationPack->PresentationLayers)
		{
			const bool bLayerHasGaussianBinding = !Layer.ExternalPresentationAsset.IsNull() || !Layer.ExternalPresentationActorClass.IsNull();
			if (bLayerHasGaussianBinding)
			{
				++DeclaredGaussianChunkCount;
				if (PrimaryGaussianProvider == EParaglideGaussianPresentationProvider::None)
				{
					PrimaryGaussianProvider = Layer.GaussianProvider;
				}
			}

			TotalChunkCount += Layer.Chunks.Num();
			for (const FParaglideDestinationChunk& Chunk : Layer.Chunks)
			{
				const bool bChunkHasGaussianBinding = !Chunk.ExternalPresentationAsset.IsNull() || !Chunk.ExternalPresentationActorClass.IsNull();
				if (bChunkHasGaussianBinding)
				{
					++DeclaredGaussianChunkCount;
					if (PrimaryGaussianProvider == EParaglideGaussianPresentationProvider::None)
					{
						PrimaryGaussianProvider = Chunk.GaussianProvider;
					}
				}
			}
		}
	}

	Readiness.TotalChunkCount = TotalChunkCount;
	Readiness.DeclaredGaussianChunkCount = DeclaredGaussianChunkCount;
	Readiness.PrimaryGaussianProvider = PrimaryGaussianProvider;

	const EParaglideWorldPresentationMode RequestedMode = PresentationMode;
	EParaglideWorldPresentationMode EffectiveMode = PresentationMode;
	if (EffectiveMode == EParaglideWorldPresentationMode::Automatic && PresentationOwner != nullptr)
	{
		EffectiveMode = PresentationOwner->ResolveRequestedPresentationMode();
	}

	bool bProceduralReady = false;
	bool bGaussianPlaceholderReady = false;
	bool bSupportsSelectedMode = false;
	EParaglideWorldPresentationMode RuntimeMode = EParaglideWorldPresentationMode::ProceduralFallback;
	int32 LoadedChunkCount = 0;

	switch (EffectiveMode)
	{
	case EParaglideWorldPresentationMode::GaussianSplatPlaceholder:
		bGaussianPlaceholderReady = BuildGaussianSplatPlaceholderPresentation(LoadedChunkCount);
		if (SpawnedGaussianActors.Num() > 0)
		{
			ReleaseProceduralFallbackPresentation();
			RuntimeMode = EParaglideWorldPresentationMode::GaussianSplatPlaceholder;
			bSupportsSelectedMode = true;
		}
		else
		{
			bProceduralReady = BuildProceduralFallbackPresentation();
			RuntimeMode = bProceduralReady ? EParaglideWorldPresentationMode::ProceduralFallback : EParaglideWorldPresentationMode::GaussianSplatPlaceholder;
		}
		break;
	case EParaglideWorldPresentationMode::Hybrid:
		bGaussianPlaceholderReady = BuildGaussianSplatPlaceholderPresentation(LoadedChunkCount);
		if (SpawnedGaussianActors.Num() > 0)
		{
			ReleaseProceduralFallbackPresentation();
			RuntimeMode = EParaglideWorldPresentationMode::Hybrid;
			bSupportsSelectedMode = true;
		}
		else
		{
			bProceduralReady = BuildProceduralFallbackPresentation();
			RuntimeMode = EParaglideWorldPresentationMode::ProceduralFallback;
		}
		break;
	case EParaglideWorldPresentationMode::ProceduralFallback:
		ReleaseGaussianPresentation();
		bProceduralReady = BuildProceduralFallbackPresentation();
		RuntimeMode = EParaglideWorldPresentationMode::ProceduralFallback;
		bSupportsSelectedMode = bProceduralReady;
		break;
	case EParaglideWorldPresentationMode::Automatic:
	default:
		ReleaseGaussianPresentation();
		bProceduralReady = BuildProceduralFallbackPresentation();
		RuntimeMode = EParaglideWorldPresentationMode::ProceduralFallback;
		bSupportsSelectedMode = bProceduralReady;
		break;
	}

	if (RequestedMode == EParaglideWorldPresentationMode::Automatic)
	{
		bSupportsSelectedMode = bProceduralReady || bGaussianPlaceholderReady;
	}

	UpdateReadinessFlags(
		RequestedMode,
		RuntimeMode,
		bProceduralReady,
		bGaussianPlaceholderReady,
		bSupportsSelectedMode,
		TotalChunkCount,
		DeclaredGaussianChunkCount,
		SpawnedGaussianActors.Num(),
		PrimaryGaussianProvider,
		LoadedChunkCount);
}

const FParaglideWorldPresentationReadiness& UParaglideWorldPresentationComponent::GetReadiness() const
{
	return Readiness;
}

bool UParaglideWorldPresentationComponent::IsPresentationReady() const
{
	return Readiness.bIsReady;
}

bool UParaglideWorldPresentationComponent::CanUseGaussianSplatPlaceholder() const
{
	const AParaglideWorldPresentationActor* PresentationOwner = GetPresentationOwner();
	const UParaglideDestinationPack* DestinationPack = PresentationOwner != nullptr ? PresentationOwner->GetLoadedDestinationAsset() : nullptr;
	if (DestinationPack == nullptr)
	{
		return false;
	}

	for (const FParaglideDestinationPresentationLayer& Layer : DestinationPack->PresentationLayers)
	{
		if (!Layer.ExternalPresentationAsset.IsNull() || !Layer.ExternalPresentationActorClass.IsNull())
		{
			return true;
		}

		for (const FParaglideDestinationChunk& Chunk : Layer.Chunks)
		{
			if (!Chunk.ExternalPresentationAsset.IsNull() || !Chunk.ExternalPresentationActorClass.IsNull())
			{
				return true;
			}
		}
	}

	return false;
}

bool UParaglideWorldPresentationComponent::BuildProceduralFallbackPresentation()
{
	AParaglideWorldPresentationActor* PresentationOwner = GetPresentationOwner();
	if (PresentationOwner == nullptr || GetWorld() == nullptr || PresentationOwner->ProceduralFallbackActorClass == nullptr)
	{
		return false;
	}

	if (IsValid(SpawnedFallbackActor))
	{
		return true;
	}

	for (TActorIterator<AActor> It(GetWorld(), PresentationOwner->ProceduralFallbackActorClass); It; ++It)
	{
		if (*It != PresentationOwner && It->GetOwner() == PresentationOwner)
		{
			SpawnedFallbackActor = *It;
			return true;
		}
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = PresentationOwner;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SpawnedFallbackActor = GetWorld()->SpawnActor<AActor>(
		PresentationOwner->ProceduralFallbackActorClass,
		PresentationOwner->GetActorTransform(),
		SpawnParameters);
	return IsValid(SpawnedFallbackActor);
}

bool UParaglideWorldPresentationComponent::BuildGaussianSplatPlaceholderPresentation(int32& OutLoadedChunkCount)
{
	AParaglideWorldPresentationActor* PresentationOwner = GetPresentationOwner();
	UParaglideDestinationPack* DestinationPack = PresentationOwner != nullptr ? PresentationOwner->GetLoadedDestinationAsset() : nullptr;
	OutLoadedChunkCount = 0;
	if (PresentationOwner == nullptr || GetWorld() == nullptr || DestinationPack == nullptr)
	{
		ReleaseGaussianPresentation();
		return false;
	}

	ReleaseGaussianPresentation();

	bool bSpawnedAnyActors = false;
	for (const FParaglideDestinationPresentationLayer& Layer : DestinationPack->PresentationLayers)
	{
		bSpawnedAnyActors |= SpawnGaussianPresentationBinding(
			Layer.ExternalPresentationAsset,
			Layer.ExternalPresentationActorClass,
			Layer.GaussianProvider,
			FTransform::Identity);

		for (const FParaglideDestinationChunk& Chunk : Layer.Chunks)
		{
			const bool bSpawnedChunkActor = SpawnGaussianPresentationBinding(
				Chunk.ExternalPresentationAsset,
				Chunk.ExternalPresentationActorClass,
				Chunk.GaussianProvider,
				Chunk.LocalTransform);
			bSpawnedAnyActors |= bSpawnedChunkActor;
			if (bSpawnedChunkActor)
			{
				++OutLoadedChunkCount;
			}
		}
	}

	return bSpawnedAnyActors;
}

bool UParaglideWorldPresentationComponent::SpawnGaussianPresentationBinding(
	const FSoftObjectPath& AssetPath,
	const FSoftClassPath& ActorClassPath,
	const EParaglideGaussianPresentationProvider Provider,
	const FTransform& LocalTransform)
{
	return SpawnGaussianActorFromAssetPath(AssetPath, Provider, LocalTransform) ||
		SpawnGaussianActorFromClassPath(ActorClassPath, LocalTransform);
}

bool UParaglideWorldPresentationComponent::SpawnGaussianActorFromClassPath(
	const FSoftClassPath& ActorClassPath,
	const FTransform& LocalTransform)
{
	if (ActorClassPath.IsNull())
	{
		return false;
	}

	UClass* ActorClass = ActorClassPath.TryLoadClass<AActor>();
	if (ActorClass == nullptr)
	{
		return false;
	}

	return SpawnGaussianActorFromResolvedClass(ActorClass, LocalTransform);
}

bool UParaglideWorldPresentationComponent::SpawnGaussianActorFromResolvedClass(
	UClass* ActorClass,
	const FTransform& LocalTransform)
{
	if (ActorClass == nullptr)
	{
		return false;
	}

	AParaglideWorldPresentationActor* PresentationOwner = GetPresentationOwner();
	if (PresentationOwner == nullptr || GetWorld() == nullptr)
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = PresentationOwner;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FTransform SpawnTransform = LocalTransform * PresentationOwner->GetActorTransform();
	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParameters);
	if (SpawnedActor == nullptr)
	{
		return false;
	}

	SpawnedActor->AttachToActor(PresentationOwner, FAttachmentTransformRules::KeepWorldTransform);
	SpawnedGaussianActors.Add(SpawnedActor);
	return true;
}

bool UParaglideWorldPresentationComponent::SpawnGaussianActorFromAssetPath(
	const FSoftObjectPath& AssetPath,
	const EParaglideGaussianPresentationProvider Provider,
	const FTransform& LocalTransform)
{
	if (Provider == EParaglideGaussianPresentationProvider::NanoGS && SpawnNanoGSActorFromAssetPath(AssetPath, LocalTransform))
	{
		return true;
	}

	UClass* ActorClass = ResolveGaussianActorClass(AssetPath);
	if (ActorClass == nullptr)
	{
		return false;
	}

	return SpawnGaussianActorFromResolvedClass(ActorClass, LocalTransform);
}

bool UParaglideWorldPresentationComponent::SpawnNanoGSActorFromAssetPath(
	const FSoftObjectPath& AssetPath,
	const FTransform& LocalTransform)
{
	if (AssetPath.IsNull())
	{
		return false;
	}

	UObject* LoadedAsset = AssetPath.TryLoad();
	if (LoadedAsset == nullptr || LoadedAsset->GetClass()->GetName() != TEXT("GaussianSplatAsset"))
	{
		return false;
	}

	UClass* NanoGSActorClass = LoadClass<AActor>(nullptr, TEXT("/Script/NanoGS.GaussianSplatActor"));
	if (NanoGSActorClass == nullptr)
	{
		return false;
	}

	AParaglideWorldPresentationActor* PresentationOwner = GetPresentationOwner();
	if (PresentationOwner == nullptr || GetWorld() == nullptr)
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = PresentationOwner;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FTransform SpawnTransform = LocalTransform * PresentationOwner->GetActorTransform();
	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(NanoGSActorClass, SpawnTransform, SpawnParameters);
	if (SpawnedActor == nullptr)
	{
		return false;
	}

	UObject* GaussianSplatComponentObject = nullptr;
	if (const FObjectProperty* ComponentProperty = FindFProperty<FObjectProperty>(SpawnedActor->GetClass(), TEXT("GaussianSplatComponent")))
	{
		GaussianSplatComponentObject = ComponentProperty->GetObjectPropertyValue_InContainer(SpawnedActor);
	}

	if (GaussianSplatComponentObject == nullptr)
	{
		SpawnedActor->Destroy();
		return false;
	}

	if (UFunction* SetSplatAssetFunction = GaussianSplatComponentObject->FindFunction(TEXT("SetSplatAsset")))
	{
		struct FSetSplatAssetParams
		{
			UObject* NewAsset = nullptr;
		};

		FSetSplatAssetParams Params;
		Params.NewAsset = LoadedAsset;
		GaussianSplatComponentObject->ProcessEvent(SetSplatAssetFunction, &Params);
	}
	else
	{
		SpawnedActor->Destroy();
		return false;
	}

	SpawnedActor->AttachToActor(PresentationOwner, FAttachmentTransformRules::KeepWorldTransform);
	SpawnedGaussianActors.Add(SpawnedActor);
	return true;
}

UClass* UParaglideWorldPresentationComponent::ResolveGaussianActorClass(const FSoftObjectPath& AssetPath) const
{
	if (AssetPath.IsNull())
	{
		return nullptr;
	}

	if (UClass* DirectClass = LoadClass<AActor>(nullptr, *AssetPath.ToString()))
	{
		return DirectClass;
	}

	UObject* LoadedAsset = AssetPath.TryLoad();
	if (UClass* LoadedClass = Cast<UClass>(LoadedAsset))
	{
		return LoadedClass->IsChildOf(AActor::StaticClass()) ? LoadedClass : nullptr;
	}

	if (const UBlueprint* BlueprintAsset = Cast<UBlueprint>(LoadedAsset))
	{
		return BlueprintAsset->GeneratedClass != nullptr && BlueprintAsset->GeneratedClass->IsChildOf(AActor::StaticClass())
			? BlueprintAsset->GeneratedClass
			: nullptr;
	}

	return nullptr;
}

void UParaglideWorldPresentationComponent::ReleaseProceduralFallbackPresentation()
{
	if (IsValid(SpawnedFallbackActor))
	{
		SpawnedFallbackActor->Destroy();
		SpawnedFallbackActor = nullptr;
	}
}

void UParaglideWorldPresentationComponent::ReleaseGaussianPresentation()
{
	for (AActor* GaussianActor : SpawnedGaussianActors)
	{
		if (IsValid(GaussianActor))
		{
			GaussianActor->Destroy();
		}
	}

	SpawnedGaussianActors.Reset();
}

AParaglideWorldPresentationActor* UParaglideWorldPresentationComponent::GetPresentationOwner() const
{
	return Cast<AParaglideWorldPresentationActor>(GetOwner());
}

void UParaglideWorldPresentationComponent::UpdateReadinessFlags(
	const EParaglideWorldPresentationMode RequestedMode,
	const EParaglideWorldPresentationMode RuntimeMode,
	const bool bProceduralReady,
	const bool bGaussianPlaceholderReady,
	const bool bSupportsSelectedMode,
	const int32 TotalChunkCount,
	const int32 DeclaredGaussianChunkCount,
	const int32 LiveGaussianActorCount,
	const EParaglideGaussianPresentationProvider PrimaryGaussianProvider,
	const int32 LoadedChunkCount)
{
	Readiness.RequestedMode = RequestedMode;
	Readiness.RuntimeMode = RuntimeMode;
	Readiness.TotalChunkCount = TotalChunkCount;
	Readiness.DeclaredGaussianChunkCount = DeclaredGaussianChunkCount;
	Readiness.LiveGaussianActorCount = LiveGaussianActorCount;
	Readiness.LoadedChunkCount = LoadedChunkCount;
	Readiness.PrimaryGaussianProvider = PrimaryGaussianProvider;
	Readiness.bHasDestinationAsset = GetPresentationOwner() && !GetPresentationOwner()->GetDestinationAsset().IsNull();
	Readiness.bHasPresentationHost = GetPresentationOwner() != nullptr;
	Readiness.bHasProceduralFallback = bProceduralReady;
	Readiness.bHasGaussianSplatPlaceholder = bGaussianPlaceholderReady;
	Readiness.bSupportsSelectedMode = bSupportsSelectedMode;
	Readiness.bIsReady = Readiness.bHasPresentationHost && (bProceduralReady || bGaussianPlaceholderReady);
}
