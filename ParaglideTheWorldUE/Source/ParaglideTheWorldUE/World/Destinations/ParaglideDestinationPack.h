// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UObject/SoftObjectPath.h"
#include "ParaglideDestinationPack.generated.h"

UENUM(BlueprintType)
enum class EParaglideDestinationPresentationMode : uint8
{
	PrototypeProcedural UMETA(DisplayName = "Prototype Procedural"),
	GaussianSplatExternalPlugin UMETA(DisplayName = "Gaussian Splat External Plugin"),
	Hybrid UMETA(DisplayName = "Hybrid"),
};

UENUM(BlueprintType)
enum class EParaglideGaussianPresentationProvider : uint8
{
	None UMETA(DisplayName = "None"),
	XverseXV3dGS UMETA(DisplayName = "XVERSE XV3dGS"),
	UEGaussianSplatting UMETA(DisplayName = "UEGaussianSplatting"),
};

USTRUCT(BlueprintType)
struct FParaglideDestinationCollisionMetadata
{
	GENERATED_BODY()

	// Collision here is descriptive first: a pack can declare coarse gameplay surfaces
	// even when the visual source is an external splat representation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	bool bEnableCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	bool bTreatAsTerrain = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	float GroundClearanceBiasMeters = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	float LandingSurfaceThicknessMeters = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	FBox LocalBounds = FBox(EForceInit::ForceInit);
};

USTRUCT(BlueprintType)
struct FParaglideDestinationAirflowMetadata
{
	GENERATED_BODY()

	// Airflow is authored as a gameplay field, not a rendering artifact.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	float WindHeadingDeg = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	float WindSpeedKmh = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	float Turbulence = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	float RidgeLiftBias = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	float ThermalDensity = 0.0f;
};

USTRUCT(BlueprintType)
struct FParaglideDestinationChunk
{
	GENERATED_BODY()

	// Chunks are the smallest addressable streaming unit in a destination pack.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	FName ChunkId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	EParaglideDestinationPresentationMode PresentationMode = EParaglideDestinationPresentationMode::PrototypeProcedural;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	FTransform LocalTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	FVector BoundsExtentMeters = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	FSoftObjectPath ExternalPresentationAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	FSoftClassPath ExternalPresentationActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	EParaglideGaussianPresentationProvider GaussianProvider = EParaglideGaussianPresentationProvider::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	FParaglideDestinationCollisionMetadata Collision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	FParaglideDestinationAirflowMetadata Airflow;
};

USTRUCT(BlueprintType)
struct FParaglideDestinationPresentationLayer
{
	GENERATED_BODY()

	// Layers group chunks by intent, not by implementation detail.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	FName LayerId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	EParaglideDestinationPresentationMode PresentationMode = EParaglideDestinationPresentationMode::PrototypeProcedural;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	FSoftObjectPath ExternalPresentationAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	FSoftClassPath ExternalPresentationActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	EParaglideGaussianPresentationProvider GaussianProvider = EParaglideGaussianPresentationProvider::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Destination")
	TArray<FParaglideDestinationChunk> Chunks;
};

UCLASS(BlueprintType)
class PARAGLIDETHEWORLDUE_API UParaglideDestinationPack : public UDataAsset
{
	GENERATED_BODY()

public:
	// Pack-level defaults let a destination load cleanly even if individual layers
	// are authored for mixed procedural and splat-backed presentation.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paraglide|Destination")
	FName DestinationId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paraglide|Destination")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paraglide|Destination")
	FText Summary;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paraglide|Destination")
	EParaglideDestinationPresentationMode DefaultPresentationMode = EParaglideDestinationPresentationMode::Hybrid;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paraglide|Destination")
	FParaglideDestinationCollisionMetadata Collision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paraglide|Destination")
	FParaglideDestinationAirflowMetadata Airflow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paraglide|Destination")
	TArray<FParaglideDestinationPresentationLayer> PresentationLayers;
};
