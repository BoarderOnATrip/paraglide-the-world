// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/Presentation/ParaglideWorldPresentationComponent.h"

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
	const bool bHasGaussianPlaceholder = BuildGaussianSplatPlaceholderPresentation();
	const bool bBuiltFallback = BuildProceduralFallbackPresentation();
	UpdateReadinessFlags(
		PresentationMode,
		bBuiltFallback ? EParaglideWorldPresentationMode::ProceduralFallback : EParaglideWorldPresentationMode::Hybrid,
		bBuiltFallback,
		bHasGaussianPlaceholder,
		false,
		Readiness.TotalChunkCount,
		Readiness.DeclaredGaussianChunkCount);
	return bBuiltFallback || bHasGaussianPlaceholder;
}

bool UParaglideWorldPresentationComponent::ActivateGaussianSplatPlaceholderPath()
{
	const bool bHasGaussianPlaceholder = BuildGaussianSplatPlaceholderPresentation();
	const bool bBuiltFallback = BuildProceduralFallbackPresentation();
	UpdateReadinessFlags(
		PresentationMode,
		bBuiltFallback ? EParaglideWorldPresentationMode::ProceduralFallback : EParaglideWorldPresentationMode::GaussianSplatPlaceholder,
		bBuiltFallback,
		bHasGaussianPlaceholder,
		false,
		Readiness.TotalChunkCount,
		Readiness.DeclaredGaussianChunkCount);
	return bBuiltFallback || bHasGaussianPlaceholder;
}

void UParaglideWorldPresentationComponent::RefreshPresentationPath()
{
	AParaglideWorldPresentationActor* PresentationOwner = GetPresentationOwner();
	UParaglideDestinationPack* DestinationPack = PresentationOwner != nullptr ? PresentationOwner->GetLoadedDestinationAsset() : nullptr;

	int32 TotalChunkCount = 0;
	int32 DeclaredGaussianChunkCount = 0;
	if (DestinationPack != nullptr)
	{
		for (const FParaglideDestinationPresentationLayer& Layer : DestinationPack->PresentationLayers)
		{
			if (!Layer.ExternalPresentationAsset.IsNull())
			{
				++DeclaredGaussianChunkCount;
			}

			TotalChunkCount += Layer.Chunks.Num();
			for (const FParaglideDestinationChunk& Chunk : Layer.Chunks)
			{
				if (!Chunk.ExternalPresentationAsset.IsNull())
				{
					++DeclaredGaussianChunkCount;
				}
			}
		}
	}

	Readiness.TotalChunkCount = TotalChunkCount;
	Readiness.DeclaredGaussianChunkCount = DeclaredGaussianChunkCount;

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

	switch (EffectiveMode)
	{
	case EParaglideWorldPresentationMode::GaussianSplatPlaceholder:
		bGaussianPlaceholderReady = BuildGaussianSplatPlaceholderPresentation();
		bProceduralReady = BuildProceduralFallbackPresentation();
		RuntimeMode = bProceduralReady ? EParaglideWorldPresentationMode::ProceduralFallback : EParaglideWorldPresentationMode::GaussianSplatPlaceholder;
		break;
	case EParaglideWorldPresentationMode::Hybrid:
		bGaussianPlaceholderReady = BuildGaussianSplatPlaceholderPresentation();
		bProceduralReady = BuildProceduralFallbackPresentation();
		RuntimeMode = EParaglideWorldPresentationMode::Hybrid;
		break;
	case EParaglideWorldPresentationMode::ProceduralFallback:
		bProceduralReady = BuildProceduralFallbackPresentation();
		RuntimeMode = EParaglideWorldPresentationMode::ProceduralFallback;
		bSupportsSelectedMode = bProceduralReady;
		break;
	case EParaglideWorldPresentationMode::Automatic:
	default:
		bProceduralReady = BuildProceduralFallbackPresentation();
		RuntimeMode = EParaglideWorldPresentationMode::ProceduralFallback;
		bSupportsSelectedMode = bProceduralReady;
		break;
	}

	UpdateReadinessFlags(
		RequestedMode,
		RuntimeMode,
		bProceduralReady,
		bGaussianPlaceholderReady,
		bSupportsSelectedMode,
		TotalChunkCount,
		DeclaredGaussianChunkCount);
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
		if (!Layer.ExternalPresentationAsset.IsNull())
		{
			return true;
		}

		for (const FParaglideDestinationChunk& Chunk : Layer.Chunks)
		{
			if (!Chunk.ExternalPresentationAsset.IsNull())
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
		if (*It != PresentationOwner)
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

bool UParaglideWorldPresentationComponent::BuildGaussianSplatPlaceholderPresentation()
{
	return CanUseGaussianSplatPlaceholder();
}

void UParaglideWorldPresentationComponent::ReleaseProceduralFallbackPresentation()
{
	if (IsValid(SpawnedFallbackActor))
	{
		SpawnedFallbackActor->Destroy();
		SpawnedFallbackActor = nullptr;
	}
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
	const int32 DeclaredGaussianChunkCount)
{
	Readiness.RequestedMode = RequestedMode;
	Readiness.RuntimeMode = RuntimeMode;
	Readiness.TotalChunkCount = TotalChunkCount;
	Readiness.DeclaredGaussianChunkCount = DeclaredGaussianChunkCount;
	Readiness.bHasDestinationAsset = GetPresentationOwner() && !GetPresentationOwner()->GetDestinationAsset().IsNull();
	Readiness.bHasPresentationHost = GetPresentationOwner() != nullptr;
	Readiness.bHasProceduralFallback = bProceduralReady;
	Readiness.bHasGaussianSplatPlaceholder = bGaussianPlaceholderReady;
	Readiness.bSupportsSelectedMode = bSupportsSelectedMode;
	Readiness.bIsReady = Readiness.bHasPresentationHost && (bProceduralReady || bGaussianPlaceholderReady);
}
