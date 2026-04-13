// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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
};
