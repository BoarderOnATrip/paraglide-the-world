// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/Presentation/ParaglideWorldPresentationActor.h"

#include "Components/SceneComponent.h"
#include "World/Destinations/ParaglideDestinationPack.h"
#include "World/ParaglidePrototypeWorld.h"
#include "World/Presentation/ParaglideWorldPresentationComponent.h"

AParaglideWorldPresentationActor::AParaglideWorldPresentationActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	PresentationComponent = CreateDefaultSubobject<UParaglideWorldPresentationComponent>(TEXT("PresentationComponent"));
	ProceduralFallbackActorClass = AParaglidePrototypeWorld::StaticClass();
}

void AParaglideWorldPresentationActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshPresentation();
}

void AParaglideWorldPresentationActor::BeginPlay()
{
	Super::BeginPlay();
	RefreshPresentation();
}

void AParaglideWorldPresentationActor::SetPresentationMode(const EParaglideWorldPresentationMode NewMode)
{
	PresentationMode = NewMode;
	if (PresentationComponent)
	{
		PresentationComponent->SetPresentationMode(NewMode);
	}
}

EParaglideWorldPresentationMode AParaglideWorldPresentationActor::GetPresentationMode() const
{
	return PresentationMode;
}

void AParaglideWorldPresentationActor::SetDestinationAsset(TSoftObjectPtr<UParaglideDestinationPack> NewDestinationAsset)
{
	DestinationAsset = MoveTemp(NewDestinationAsset);
	RefreshPresentation();
}

const TSoftObjectPtr<UParaglideDestinationPack>& AParaglideWorldPresentationActor::GetDestinationAsset() const
{
	return DestinationAsset;
}

UParaglideDestinationPack* AParaglideWorldPresentationActor::GetLoadedDestinationAsset() const
{
	return LoadedDestinationAsset;
}

EParaglideWorldPresentationMode AParaglideWorldPresentationActor::ResolveRequestedPresentationMode() const
{
	if (PresentationMode != EParaglideWorldPresentationMode::Automatic || LoadedDestinationAsset == nullptr)
	{
		return PresentationMode == EParaglideWorldPresentationMode::Automatic
			? EParaglideWorldPresentationMode::ProceduralFallback
			: PresentationMode;
	}

	switch (LoadedDestinationAsset->DefaultPresentationMode)
	{
	case EParaglideDestinationPresentationMode::GaussianSplatExternalPlugin:
		return EParaglideWorldPresentationMode::GaussianSplatPlaceholder;
	case EParaglideDestinationPresentationMode::Hybrid:
		return EParaglideWorldPresentationMode::Hybrid;
	case EParaglideDestinationPresentationMode::PrototypeProcedural:
	default:
		return EParaglideWorldPresentationMode::ProceduralFallback;
	}
}

void AParaglideWorldPresentationActor::RefreshPresentation()
{
	LoadedDestinationAsset = DestinationAsset.IsNull() ? nullptr : DestinationAsset.LoadSynchronous();

	if (PresentationComponent)
	{
		PresentationComponent->SetPresentationMode(PresentationMode);
		PresentationComponent->RefreshPresentationPath();
	}
}

bool AParaglideWorldPresentationActor::IsPresentationReady() const
{
	return PresentationComponent ? PresentationComponent->IsPresentationReady() : false;
}

const FParaglideWorldPresentationReadiness& AParaglideWorldPresentationActor::GetPresentationReadiness() const
{
	static const FParaglideWorldPresentationReadiness EmptyReadiness;
	return PresentationComponent ? PresentationComponent->GetReadiness() : EmptyReadiness;
}

FParaglideWorldPresentationSnapshot AParaglideWorldPresentationActor::GetPresentationSnapshot() const
{
	FParaglideWorldPresentationSnapshot Snapshot;
	Snapshot.Readiness = GetPresentationReadiness();

	if (LoadedDestinationAsset != nullptr)
	{
		Snapshot.DestinationId = LoadedDestinationAsset->DestinationId;
		Snapshot.DestinationName = LoadedDestinationAsset->DisplayName;
		Snapshot.DestinationSummary = LoadedDestinationAsset->Summary;
	}

	return Snapshot;
}

void AParaglideWorldPresentationActor::SelectProceduralFallback()
{
	SetPresentationMode(EParaglideWorldPresentationMode::ProceduralFallback);
}

void AParaglideWorldPresentationActor::SelectGaussianSplatPlaceholder()
{
	SetPresentationMode(EParaglideWorldPresentationMode::GaussianSplatPlaceholder);
}
