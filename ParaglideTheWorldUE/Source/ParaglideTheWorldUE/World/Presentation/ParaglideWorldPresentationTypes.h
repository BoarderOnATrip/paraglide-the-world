// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "World/Destinations/ParaglideDestinationPack.h"
#include "ParaglideWorldPresentationTypes.generated.h"

UENUM(BlueprintType)
enum class EParaglideWorldPresentationMode : uint8
{
	Automatic,
	ProceduralFallback,
	Hybrid,
	GaussianSplatPlaceholder,
};

USTRUCT(BlueprintType)
struct FParaglideWorldPresentationReadiness
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	EParaglideWorldPresentationMode RequestedMode = EParaglideWorldPresentationMode::Automatic;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	EParaglideWorldPresentationMode RuntimeMode = EParaglideWorldPresentationMode::ProceduralFallback;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bHasDestinationAsset = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bHasPresentationHost = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bHasProceduralFallback = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bHasGaussianSplatPlaceholder = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bSupportsSelectedMode = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bIsReady = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	int32 TotalChunkCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	int32 DeclaredGaussianChunkCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	int32 LiveGaussianActorCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	EParaglideGaussianPresentationProvider PrimaryGaussianProvider = EParaglideGaussianPresentationProvider::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	int32 LoadedChunkCount = 0;
};

USTRUCT(BlueprintType)
struct FParaglideWorldPresentationSnapshot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	FName DestinationId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	FText DestinationName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	FText DestinationSummary;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	FParaglideWorldPresentationReadiness Readiness;
};
