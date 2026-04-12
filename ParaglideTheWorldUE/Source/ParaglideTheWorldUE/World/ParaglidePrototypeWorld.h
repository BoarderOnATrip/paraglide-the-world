// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ParaglidePrototypeWorld.generated.h"

class UInstancedStaticMeshComponent;
class UDirectionalLightComponent;
class UExponentialHeightFogComponent;
class USceneComponent;
class USkyAtmosphereComponent;
class USkyLightComponent;
class UStaticMesh;
class UTextRenderComponent;

UCLASS()
class PARAGLIDETHEWORLDUE_API AParaglidePrototypeWorld : public AActor
{
	GENERATED_BODY()

public:
	AParaglidePrototypeWorld();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "Paraglide")
	void RebuildPrototypeWorld();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|World")
	bool bCenterOnActivePawn = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|World")
	float AnchorAglMeters = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|World")
	float WorldScale = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|World")
	float CourseLengthMeters = 1900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|World")
	float CourseWidthMeters = 420.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|World")
	float LaunchDeckLengthMeters = 110.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|World")
	float RidgeDistanceMeters = 760.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|World")
	float RidgeLengthMeters = 320.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|World")
	float LandingZoneDistanceMeters = 1540.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|World")
	float LandingZoneLengthMeters = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|World")
	float LandingZoneWidthMeters = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|World")
	int32 ThermalCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|World")
	int32 GateCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|World")
	float GateSpacingMeters = 420.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|World")
	float GateWidthMeters = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|World")
	float LabelWorldSize = 80.0f;

private:
	void RecenterToActivePawn();
	void RefreshAtmosphereMode();
	void ClearInstances();
	void BuildLaunchDeck();
	void BuildRidgeReference();
	void BuildLandingZone();
	void BuildThermals();
	void BuildGates();
	void BuildLabels();
	void BuildCliffArticulation();
	void BuildForestPatches();
	void BuildClouds();
	void BuildSkyMarkers();
	void BuildBackdropMountains();
	void BuildAtmosphericBands();
	void AddBoxInstance(UInstancedStaticMeshComponent* Component, const FVector& LocationMeters, const FVector& Scale, const FRotator& Rotation = FRotator::ZeroRotator) const;
	void AddSphereInstance(UInstancedStaticMeshComponent* Component, const FVector& LocationMeters, const FVector& Scale, const FRotator& Rotation = FRotator::ZeroRotator) const;
	void AddCylinderInstance(UInstancedStaticMeshComponent* Component, const FVector& LocationMeters, const FVector& Scale, const FRotator& Rotation = FRotator::ZeroRotator) const;
	void AddTextLabel(UTextRenderComponent* LabelComponent, const FVector& LocationMeters, const FString& Text, const FLinearColor& Color) const;
	FVector ToWorldLocation(const FVector& LocationMeters) const;

private:
	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide|Atmosphere")
	TObjectPtr<UDirectionalLightComponent> SunLight;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide|Atmosphere")
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide|Atmosphere")
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide|Atmosphere")
	TObjectPtr<UExponentialHeightFogComponent> HeightFog;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> GroundDeckInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> LaunchRampInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> ParagliderCanopyInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> ParagliderHarnessInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> ParagliderLineInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> RidgeInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> LandingZoneInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> ThermalColumnInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> ThermalCapInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> GateInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> CliffFaceInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> CliffSnowInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> TreeTrunkInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> TreeCanopyInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> CloudInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> WindMarkerInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> MeadowStripeInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> RockSpineInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> BackdropMountainInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UInstancedStaticMeshComponent> MistBandInstances;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UTextRenderComponent> LaunchLabel;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UTextRenderComponent> RidgeLabel;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UTextRenderComponent> LandingLabel;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UTextRenderComponent> ThermalLabel;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UTextRenderComponent> GateLabel;

	bool bHasCenteredOnPawn = false;
};
