// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/ParaglidePrototypeWorld.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	FVector MeterToWorld(const FVector& Value, const float WorldScale)
	{
		return Value * WorldScale;
	}
}

AParaglidePrototypeWorld::AParaglidePrototypeWorld()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	GroundDeckInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GroundDeckInstances"));
	GroundDeckInstances->SetupAttachment(SceneRoot);
	GroundDeckInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CubeMesh.Succeeded())
	{
		GroundDeckInstances->SetStaticMesh(CubeMesh.Object);
	}

	LaunchRampInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("LaunchRampInstances"));
	LaunchRampInstances->SetupAttachment(SceneRoot);
	LaunchRampInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CubeMesh.Succeeded())
	{
		LaunchRampInstances->SetStaticMesh(CubeMesh.Object);
	}

	ParagliderCanopyInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ParagliderCanopyInstances"));
	ParagliderCanopyInstances->SetupAttachment(SceneRoot);
	ParagliderCanopyInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CubeMesh.Succeeded())
	{
		ParagliderCanopyInstances->SetStaticMesh(CubeMesh.Object);
	}

	ParagliderHarnessInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ParagliderHarnessInstances"));
	ParagliderHarnessInstances->SetupAttachment(SceneRoot);
	ParagliderHarnessInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CubeMesh.Succeeded())
	{
		ParagliderHarnessInstances->SetStaticMesh(CubeMesh.Object);
	}

	ParagliderLineInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ParagliderLineInstances"));
	ParagliderLineInstances->SetupAttachment(SceneRoot);
	ParagliderLineInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CubeMesh.Succeeded())
	{
		ParagliderLineInstances->SetStaticMesh(CubeMesh.Object);
	}

	RidgeInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RidgeInstances"));
	RidgeInstances->SetupAttachment(SceneRoot);
	RidgeInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CubeMesh.Succeeded())
	{
		RidgeInstances->SetStaticMesh(CubeMesh.Object);
	}

	LandingZoneInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("LandingZoneInstances"));
	LandingZoneInstances->SetupAttachment(SceneRoot);
	LandingZoneInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CubeMesh.Succeeded())
	{
		LandingZoneInstances->SetStaticMesh(CubeMesh.Object);
	}

	ThermalColumnInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ThermalColumnInstances"));
	ThermalColumnInstances->SetupAttachment(SceneRoot);
	ThermalColumnInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CylinderMesh.Succeeded())
	{
		ThermalColumnInstances->SetStaticMesh(CylinderMesh.Object);
	}

	ThermalCapInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ThermalCapInstances"));
	ThermalCapInstances->SetupAttachment(SceneRoot);
	ThermalCapInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (SphereMesh.Succeeded())
	{
		ThermalCapInstances->SetStaticMesh(SphereMesh.Object);
	}

	GateInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GateInstances"));
	GateInstances->SetupAttachment(SceneRoot);
	GateInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CubeMesh.Succeeded())
	{
		GateInstances->SetStaticMesh(CubeMesh.Object);
	}

	CliffFaceInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CliffFaceInstances"));
	CliffFaceInstances->SetupAttachment(SceneRoot);
	CliffFaceInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CubeMesh.Succeeded())
	{
		CliffFaceInstances->SetStaticMesh(CubeMesh.Object);
	}

	CliffSnowInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CliffSnowInstances"));
	CliffSnowInstances->SetupAttachment(SceneRoot);
	CliffSnowInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CubeMesh.Succeeded())
	{
		CliffSnowInstances->SetStaticMesh(CubeMesh.Object);
	}

	TreeTrunkInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TreeTrunkInstances"));
	TreeTrunkInstances->SetupAttachment(SceneRoot);
	TreeTrunkInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CylinderMesh.Succeeded())
	{
		TreeTrunkInstances->SetStaticMesh(CylinderMesh.Object);
	}

	TreeCanopyInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TreeCanopyInstances"));
	TreeCanopyInstances->SetupAttachment(SceneRoot);
	TreeCanopyInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (SphereMesh.Succeeded())
	{
		TreeCanopyInstances->SetStaticMesh(SphereMesh.Object);
	}

	CloudInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CloudInstances"));
	CloudInstances->SetupAttachment(SceneRoot);
	CloudInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (SphereMesh.Succeeded())
	{
		CloudInstances->SetStaticMesh(SphereMesh.Object);
	}

	WindMarkerInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WindMarkerInstances"));
	WindMarkerInstances->SetupAttachment(SceneRoot);
	WindMarkerInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CylinderMesh.Succeeded())
	{
		WindMarkerInstances->SetStaticMesh(CylinderMesh.Object);
	}

	MeadowStripeInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("MeadowStripeInstances"));
	MeadowStripeInstances->SetupAttachment(SceneRoot);
	MeadowStripeInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CubeMesh.Succeeded())
	{
		MeadowStripeInstances->SetStaticMesh(CubeMesh.Object);
	}

	RockSpineInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RockSpineInstances"));
	RockSpineInstances->SetupAttachment(SceneRoot);
	RockSpineInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CubeMesh.Succeeded())
	{
		RockSpineInstances->SetStaticMesh(CubeMesh.Object);
	}

	LaunchLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("LaunchLabel"));
	LaunchLabel->SetupAttachment(SceneRoot);
	LaunchLabel->SetWorldSize(LabelWorldSize);
	LaunchLabel->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	LaunchLabel->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	LaunchLabel->SetTextRenderColor(FColor(120, 230, 180));

	RidgeLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("RidgeLabel"));
	RidgeLabel->SetupAttachment(SceneRoot);
	RidgeLabel->SetWorldSize(LabelWorldSize);
	RidgeLabel->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	RidgeLabel->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	RidgeLabel->SetTextRenderColor(FColor(250, 210, 70));

	LandingLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("LandingLabel"));
	LandingLabel->SetupAttachment(SceneRoot);
	LandingLabel->SetWorldSize(LabelWorldSize);
	LandingLabel->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	LandingLabel->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	LandingLabel->SetTextRenderColor(FColor(255, 240, 120));

	ThermalLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ThermalLabel"));
	ThermalLabel->SetupAttachment(SceneRoot);
	ThermalLabel->SetWorldSize(LabelWorldSize);
	ThermalLabel->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	ThermalLabel->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	ThermalLabel->SetTextRenderColor(FColor(255, 160, 70));

	GateLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("GateLabel"));
	GateLabel->SetupAttachment(SceneRoot);
	GateLabel->SetWorldSize(LabelWorldSize);
	GateLabel->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	GateLabel->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	GateLabel->SetTextRenderColor(FColor(160, 220, 255));

	const auto Tint = [](UInstancedStaticMeshComponent* Component, const FLinearColor& Color)
	{
		if (Component == nullptr)
		{
			return;
		}

		const FVector ColorVector(Color.R, Color.G, Color.B);
		Component->SetVectorParameterValueOnMaterials(TEXT("Color"), ColorVector);
		Component->SetVectorParameterValueOnMaterials(TEXT("BaseColor"), ColorVector);
	};

	Tint(GroundDeckInstances, FLinearColor(0.20f, 0.18f, 0.14f, 1.0f));
	Tint(LaunchRampInstances, FLinearColor(0.58f, 0.42f, 0.24f, 1.0f));
	Tint(ParagliderCanopyInstances, FLinearColor(0.94f, 0.36f, 0.18f, 1.0f));
	Tint(ParagliderHarnessInstances, FLinearColor(0.13f, 0.16f, 0.20f, 1.0f));
	Tint(ParagliderLineInstances, FLinearColor(0.86f, 0.88f, 0.94f, 1.0f));
	Tint(RidgeInstances, FLinearColor(0.30f, 0.24f, 0.18f, 1.0f));
	Tint(LandingZoneInstances, FLinearColor(0.22f, 0.42f, 0.18f, 1.0f));
	Tint(ThermalColumnInstances, FLinearColor(1.0f, 0.48f, 0.10f, 1.0f));
	Tint(ThermalCapInstances, FLinearColor(1.0f, 0.72f, 0.22f, 1.0f));
	Tint(GateInstances, FLinearColor(0.55f, 0.78f, 1.0f, 1.0f));
	Tint(CliffFaceInstances, FLinearColor(0.26f, 0.22f, 0.20f, 1.0f));
	Tint(CliffSnowInstances, FLinearColor(0.86f, 0.90f, 0.96f, 1.0f));
	Tint(TreeTrunkInstances, FLinearColor(0.29f, 0.18f, 0.10f, 1.0f));
	Tint(TreeCanopyInstances, FLinearColor(0.17f, 0.30f, 0.14f, 1.0f));
	Tint(CloudInstances, FLinearColor(0.92f, 0.96f, 1.0f, 1.0f));
	Tint(WindMarkerInstances, FLinearColor(0.95f, 0.88f, 0.30f, 1.0f));
	Tint(MeadowStripeInstances, FLinearColor(0.24f, 0.50f, 0.20f, 1.0f));
	Tint(RockSpineInstances, FLinearColor(0.38f, 0.31f, 0.24f, 1.0f));
}

void AParaglidePrototypeWorld::BeginPlay()
{
	Super::BeginPlay();

	RecenterToActivePawn();
	RebuildPrototypeWorld();
}

void AParaglidePrototypeWorld::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bHasCenteredOnPawn || !bCenterOnActivePawn)
	{
		SetActorTickEnabled(false);
		return;
	}

	RecenterToActivePawn();
	if (bHasCenteredOnPawn)
	{
		RebuildPrototypeWorld();
		SetActorTickEnabled(false);
	}
}

void AParaglidePrototypeWorld::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildPrototypeWorld();
}

void AParaglidePrototypeWorld::RebuildPrototypeWorld()
{
	ClearInstances();
	BuildLaunchDeck();
	BuildRidgeReference();
	BuildCliffArticulation();
	BuildForestPatches();
	BuildLandingZone();
	BuildThermals();
	BuildGates();
	BuildClouds();
	BuildSkyMarkers();
	BuildLabels();
}

void AParaglidePrototypeWorld::RecenterToActivePawn()
{
	if (!bCenterOnActivePawn)
	{
		return;
	}

	APawn* ActivePawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (ActivePawn == nullptr)
	{
		return;
	}

	const FVector CenterLocation = ActivePawn->GetActorLocation() - FVector(0.0f, 0.0f, AnchorAglMeters * WorldScale);
	SetActorLocation(CenterLocation);
	bHasCenteredOnPawn = true;
}

void AParaglidePrototypeWorld::ClearInstances()
{
	GroundDeckInstances->ClearInstances();
	LaunchRampInstances->ClearInstances();
	ParagliderCanopyInstances->ClearInstances();
	ParagliderHarnessInstances->ClearInstances();
	ParagliderLineInstances->ClearInstances();
	RidgeInstances->ClearInstances();
	LandingZoneInstances->ClearInstances();
	ThermalColumnInstances->ClearInstances();
	ThermalCapInstances->ClearInstances();
	GateInstances->ClearInstances();
	CliffFaceInstances->ClearInstances();
	CliffSnowInstances->ClearInstances();
	TreeTrunkInstances->ClearInstances();
	TreeCanopyInstances->ClearInstances();
	CloudInstances->ClearInstances();
	WindMarkerInstances->ClearInstances();
	MeadowStripeInstances->ClearInstances();
	RockSpineInstances->ClearInstances();
}

void AParaglidePrototypeWorld::BuildLaunchDeck()
{
	AddBoxInstance(GroundDeckInstances, FVector(0.0f, 0.0f, -2.0f), FVector(40.0f, 14.0f, 0.7f));
	AddBoxInstance(GroundDeckInstances, FVector(0.0f, 36.0f, 5.0f), FVector(34.0f, 8.0f, 0.35f));
	AddBoxInstance(GroundDeckInstances, FVector(0.0f, 68.0f, 13.0f), FVector(26.0f, 7.0f, 0.3f));
	AddBoxInstance(LaunchRampInstances, FVector(0.0f, -20.0f, 2.0f), FVector(12.0f, 12.0f, 0.45f), FRotator(-12.0f, 0.0f, 0.0f));
	AddBoxInstance(LaunchRampInstances, FVector(0.0f, -6.0f, 5.5f), FVector(12.0f, 12.0f, 0.42f), FRotator(-9.0f, 0.0f, 0.0f));
	AddBoxInstance(LaunchRampInstances, FVector(0.0f, 10.0f, 9.5f), FVector(12.0f, 12.0f, 0.38f), FRotator(-5.0f, 0.0f, 0.0f));
	AddBoxInstance(LaunchRampInstances, FVector(0.0f, 26.0f, 13.5f), FVector(12.0f, 12.0f, 0.34f), FRotator(-3.0f, 0.0f, 0.0f));
	AddBoxInstance(LaunchRampInstances, FVector(-22.0f, 18.0f, 11.0f), FVector(4.0f, 18.0f, 4.0f), FRotator(0.0f, 0.0f, -6.0f));
	AddBoxInstance(LaunchRampInstances, FVector(22.0f, 18.0f, 11.0f), FVector(4.0f, 18.0f, 4.0f), FRotator(0.0f, 0.0f, 6.0f));
	AddBoxInstance(ParagliderCanopyInstances, FVector(0.0f, 16.0f, 31.0f), FVector(28.0f, 9.0f, 0.36f), FRotator(-9.0f, 0.0f, 0.0f));
	AddBoxInstance(ParagliderCanopyInstances, FVector(0.0f, 16.0f, 30.0f), FVector(16.0f, 6.0f, 0.18f), FRotator(8.0f, 0.0f, 0.0f));
	AddBoxInstance(ParagliderCanopyInstances, FVector(-6.5f, 16.0f, 29.5f), FVector(7.0f, 2.6f, 0.15f), FRotator(0.0f, 0.0f, -8.0f));
	AddBoxInstance(ParagliderCanopyInstances, FVector(6.5f, 16.0f, 29.5f), FVector(7.0f, 2.6f, 0.15f), FRotator(0.0f, 0.0f, 8.0f));
	AddBoxInstance(ParagliderHarnessInstances, FVector(0.0f, 8.0f, 18.0f), FVector(1.8f, 1.0f, 4.4f));
	AddBoxInstance(ParagliderHarnessInstances, FVector(0.0f, 8.0f, 14.0f), FVector(1.1f, 0.9f, 2.2f));
	AddBoxInstance(ParagliderHarnessInstances, FVector(-6.0f, 8.0f, 22.0f), FVector(2.2f, 0.5f, 0.5f));
	AddBoxInstance(ParagliderHarnessInstances, FVector(6.0f, 8.0f, 22.0f), FVector(2.2f, 0.5f, 0.5f));
	AddBoxInstance(ParagliderLineInstances, FVector(-8.0f, 12.0f, 27.0f), FVector(0.10f, 0.10f, 7.0f), FRotator(-20.0f, 0.0f, 18.0f));
	AddBoxInstance(ParagliderLineInstances, FVector(-3.0f, 12.0f, 26.0f), FVector(0.10f, 0.10f, 7.0f), FRotator(-20.0f, 0.0f, 8.0f));
	AddBoxInstance(ParagliderLineInstances, FVector(3.0f, 12.0f, 26.0f), FVector(0.10f, 0.10f, 7.0f), FRotator(-20.0f, 0.0f, -8.0f));
	AddBoxInstance(ParagliderLineInstances, FVector(8.0f, 12.0f, 27.0f), FVector(0.10f, 0.10f, 7.0f), FRotator(-20.0f, 0.0f, -18.0f));

	for (int32 FlagIndex = -2; FlagIndex <= 2; ++FlagIndex)
	{
		AddBoxInstance(WindMarkerInstances, FVector(FlagIndex * 14.0f, 26.0f, 12.0f), FVector(0.35f, 0.35f, 12.0f));
		AddBoxInstance(WindMarkerInstances, FVector(FlagIndex * 14.0f + 1.6f, 26.0f, 18.0f), FVector(2.8f, 0.35f, 0.42f), FRotator(0.0f, 0.0f, 8.0f * FlagIndex));
	}
}

void AParaglidePrototypeWorld::BuildRidgeReference()
{
	const float RidgeBaseY = RidgeDistanceMeters;
	const int32 RidgeSegmentCount = FMath::Max(12, FMath::RoundToInt(RidgeLengthMeters / 18.0f));

	for (int32 Index = 0; Index < RidgeSegmentCount; ++Index)
	{
		const float SegmentAlpha = RidgeSegmentCount > 1 ? static_cast<float>(Index) / static_cast<float>(RidgeSegmentCount - 1) : 0.0f;
		const float X = FMath::Lerp(-RidgeLengthMeters * 0.5f, RidgeLengthMeters * 0.5f, SegmentAlpha);
		const float CrestHeight = 16.0f + FMath::Sin(SegmentAlpha * PI * 2.0f) * 3.0f + FMath::Sin(SegmentAlpha * PI * 5.0f) * 1.2f;
		AddBoxInstance(RidgeInstances, FVector(X, RidgeBaseY, CrestHeight), FVector(12.0f, 8.0f, 10.0f));
		AddBoxInstance(RidgeInstances, FVector(X, RidgeBaseY - 18.0f, CrestHeight - 7.5f), FVector(8.0f, 7.0f, 4.8f));
		AddBoxInstance(CliffFaceInstances, FVector(X, RidgeBaseY - 44.0f, CrestHeight - 4.0f), FVector(10.0f, 16.0f, 18.0f), FRotator(0.0f, 0.0f, 4.0f * FMath::Sin(SegmentAlpha * PI * 3.0f)));
		AddBoxInstance(RockSpineInstances, FVector(X, RidgeBaseY + 10.0f, CrestHeight + 10.0f), FVector(4.0f, 5.5f, 9.0f), FRotator(0.0f, 0.0f, 10.0f * FMath::Sin(SegmentAlpha * PI * 4.0f)));
	}

	for (int32 Marker = -4; Marker <= 4; ++Marker)
	{
		const float X = Marker * 42.0f;
		AddBoxInstance(RidgeInstances, FVector(X, RidgeBaseY + 10.0f, 30.0f), FVector(2.0f, 2.0f, 6.0f));
	}

	for (int32 Peak = -2; Peak <= 2; ++Peak)
	{
		const float X = Peak * 68.0f;
		AddBoxInstance(CliffSnowInstances, FVector(X, RidgeBaseY + 4.0f, 34.0f + FMath::Abs(Peak) * 4.0f), FVector(7.0f, 5.0f, 1.4f));
	}
}

void AParaglidePrototypeWorld::BuildLandingZone()
{
	const FVector ZoneCenter(0.0f, LandingZoneDistanceMeters, 0.75f);
	AddBoxInstance(LandingZoneInstances, ZoneCenter, FVector(LandingZoneLengthMeters * 0.5f, LandingZoneWidthMeters * 0.5f, 0.25f));
	AddBoxInstance(LandingZoneInstances, ZoneCenter + FVector(0.0f, 0.0f, 1.6f), FVector(LandingZoneLengthMeters * 0.48f, LandingZoneWidthMeters * 0.48f, 0.05f));
	AddBoxInstance(LandingZoneInstances, ZoneCenter + FVector(0.0f, -LandingZoneLengthMeters * 0.33f, 3.8f), FVector(18.0f, 1.5f, 1.2f));
	AddBoxInstance(LandingZoneInstances, ZoneCenter + FVector(-LandingZoneLengthMeters * 0.25f, 0.0f, 2.2f), FVector(0.8f, LandingZoneWidthMeters * 0.52f, 0.35f));
	AddBoxInstance(LandingZoneInstances, ZoneCenter + FVector(LandingZoneLengthMeters * 0.25f, 0.0f, 2.2f), FVector(0.8f, LandingZoneWidthMeters * 0.52f, 0.35f));

	const FVector FrontLeft(-LandingZoneLengthMeters * 0.5f, LandingZoneDistanceMeters, 1.2f);
	const FVector FrontRight(LandingZoneLengthMeters * 0.5f, LandingZoneDistanceMeters, 1.2f);
	const FVector BackLeft(-LandingZoneLengthMeters * 0.5f, LandingZoneDistanceMeters + LandingZoneWidthMeters, 1.2f);
	const FVector BackRight(LandingZoneLengthMeters * 0.5f, LandingZoneDistanceMeters + LandingZoneWidthMeters, 1.2f);
	AddBoxInstance(LandingZoneInstances, FrontLeft, FVector(1.0f, LandingZoneWidthMeters * 0.5f, 0.4f));
	AddBoxInstance(LandingZoneInstances, FrontRight, FVector(1.0f, LandingZoneWidthMeters * 0.5f, 0.4f));
	AddBoxInstance(LandingZoneInstances, BackLeft, FVector(1.0f, LandingZoneWidthMeters * 0.5f, 0.4f));
	AddBoxInstance(LandingZoneInstances, BackRight, FVector(1.0f, LandingZoneWidthMeters * 0.5f, 0.4f));
	for (int32 Stripe = -3; Stripe <= 3; ++Stripe)
	{
		const float StripeX = Stripe * (LandingZoneLengthMeters * 0.12f);
		AddBoxInstance(MeadowStripeInstances, FVector(StripeX, LandingZoneDistanceMeters + 2.5f, 0.95f), FVector(0.65f, LandingZoneWidthMeters * 0.38f, 0.08f));
	}
	AddBoxInstance(WindMarkerInstances, FVector(-LandingZoneLengthMeters * 0.48f, LandingZoneDistanceMeters - 16.0f, 15.0f), FVector(0.35f, 0.35f, 11.0f));
	AddBoxInstance(WindMarkerInstances, FVector(-LandingZoneLengthMeters * 0.48f + 1.7f, LandingZoneDistanceMeters - 16.0f, 20.0f), FVector(2.4f, 0.32f, 0.4f), FRotator(0.0f, 0.0f, -12.0f));
	AddBoxInstance(WindMarkerInstances, FVector(LandingZoneLengthMeters * 0.48f, LandingZoneDistanceMeters + 18.0f, 15.0f), FVector(0.35f, 0.35f, 11.0f));
	AddBoxInstance(WindMarkerInstances, FVector(LandingZoneLengthMeters * 0.48f - 1.7f, LandingZoneDistanceMeters + 18.0f, 20.0f), FVector(2.4f, 0.32f, 0.4f), FRotator(0.0f, 0.0f, 12.0f));
}

void AParaglidePrototypeWorld::BuildThermals()
{
	const int32 ThermalTotal = FMath::Max(1, ThermalCount);
	for (int32 Index = 0; Index < ThermalTotal; ++Index)
	{
		const float Alpha = ThermalTotal > 1 ? static_cast<float>(Index) / static_cast<float>(ThermalTotal - 1) : 0.5f;
		const float X = FMath::Lerp(-180.0f, 220.0f, Alpha);
		const float Y = FMath::Lerp(420.0f, 1300.0f, Alpha);
		const float HeightMeters = 28.0f + Index * 7.0f;
		AddBoxInstance(ThermalColumnInstances, FVector(X, Y, HeightMeters * 0.5f), FVector(8.0f, 8.0f, HeightMeters * 0.5f));
		AddBoxInstance(ThermalCapInstances, FVector(X, Y, HeightMeters + 7.0f), FVector(5.5f, 5.5f, 5.5f));
		AddSphereInstance(CloudInstances, FVector(X + 4.0f, Y + 10.0f, HeightMeters + 20.0f), FVector(3.2f, 2.2f, 1.8f));
		AddSphereInstance(CloudInstances, FVector(X - 10.0f, Y - 6.0f, HeightMeters + 26.0f), FVector(4.0f, 2.6f, 2.0f));
	}
}

void AParaglidePrototypeWorld::BuildGates()
{
	const int32 GateTotal = FMath::Max(2, GateCount);
	for (int32 Index = 0; Index < GateTotal; ++Index)
	{
		const float GateY = 240.0f + Index * GateSpacingMeters;
		const float GateHalfWidth = GateWidthMeters * 0.5f;
		const float PostHeight = 18.0f + Index * 1.5f;
		const FVector LeftPost(-GateHalfWidth, GateY, PostHeight * 0.5f);
		const FVector RightPost(GateHalfWidth, GateY, PostHeight * 0.5f);
		const FVector TopBar(0.0f, GateY, PostHeight);

		AddBoxInstance(GateInstances, LeftPost, FVector(1.3f, 1.3f, PostHeight * 0.5f));
		AddBoxInstance(GateInstances, RightPost, FVector(1.3f, 1.3f, PostHeight * 0.5f));
		AddBoxInstance(GateInstances, TopBar, FVector(GateHalfWidth * 2.0f, 1.0f, 0.65f));
	}
}

void AParaglidePrototypeWorld::BuildLabels()
{
	AddTextLabel(LaunchLabel, FVector(0.0f, 35.0f, 20.0f), TEXT("LAUNCH"), FLinearColor(0.50f, 1.0f, 0.72f, 1.0f));
	AddTextLabel(RidgeLabel, FVector(0.0f, RidgeDistanceMeters + 45.0f, 42.0f), TEXT("RIDGE"), FLinearColor(1.0f, 0.90f, 0.35f, 1.0f));
	AddTextLabel(LandingLabel, FVector(0.0f, LandingZoneDistanceMeters + 50.0f, 24.0f), TEXT("LANDING"), FLinearColor(1.0f, 0.96f, 0.56f, 1.0f));
	AddTextLabel(ThermalLabel, FVector(160.0f, 900.0f, 76.0f), TEXT("THERMALS"), FLinearColor(1.0f, 0.62f, 0.28f, 1.0f));
	AddTextLabel(GateLabel, FVector(0.0f, 630.0f, 60.0f), TEXT("GATES"), FLinearColor(0.6f, 0.85f, 1.0f, 1.0f));
}

void AParaglidePrototypeWorld::BuildCliffArticulation()
{
	for (int32 Shelf = -3; Shelf <= 3; ++Shelf)
	{
		const float ShelfX = Shelf * 72.0f;
		const float ShelfHeight = 6.0f + FMath::Abs(Shelf) * 2.0f;
		AddBoxInstance(CliffFaceInstances, FVector(ShelfX, RidgeDistanceMeters - 88.0f, ShelfHeight), FVector(14.0f, 36.0f, 16.0f), FRotator(-6.0f, 0.0f, 0.0f));
		AddBoxInstance(CliffFaceInstances, FVector(ShelfX + 18.0f, RidgeDistanceMeters - 132.0f, ShelfHeight - 3.0f), FVector(10.0f, 20.0f, 12.0f), FRotator(4.0f, 0.0f, 8.0f));
		AddBoxInstance(RockSpineInstances, FVector(ShelfX - 28.0f, RidgeDistanceMeters - 40.0f, ShelfHeight + 22.0f), FVector(5.0f, 9.0f, 12.0f), FRotator(0.0f, 0.0f, 18.0f * Shelf));
	}

	for (int32 Spur = -4; Spur <= 4; ++Spur)
	{
		const float SpurX = Spur * 48.0f;
		AddBoxInstance(CliffSnowInstances, FVector(SpurX, RidgeDistanceMeters - 60.0f, 38.0f + FMath::Abs(Spur % 2) * 3.0f), FVector(5.2f, 4.0f, 1.0f));
	}
}

void AParaglidePrototypeWorld::BuildForestPatches()
{
	const float ForestBands[3] = {LandingZoneDistanceMeters - 220.0f, RidgeDistanceMeters + 150.0f, LandingZoneDistanceMeters + 160.0f};
	for (int32 BandIndex = 0; BandIndex < 3; ++BandIndex)
	{
		const float BaseY = ForestBands[BandIndex];
		for (int32 TreeIndex = -6; TreeIndex <= 6; ++TreeIndex)
		{
			const float TreeX = TreeIndex * 18.0f + (BandIndex % 2 == 0 ? 0.0f : 8.0f);
			const float TreeHeight = 6.0f + FMath::Abs(TreeIndex % 3) * 1.2f + BandIndex * 0.8f;
			AddCylinderInstance(TreeTrunkInstances, FVector(TreeX, BaseY, TreeHeight * 0.5f), FVector(0.65f, 0.65f, TreeHeight * 0.5f));
			AddSphereInstance(TreeCanopyInstances, FVector(TreeX, BaseY, TreeHeight + 5.2f), FVector(2.8f, 2.6f, 2.1f));
			if ((TreeIndex + BandIndex) % 3 == 0)
			{
				AddSphereInstance(TreeCanopyInstances, FVector(TreeX + 2.6f, BaseY + 2.1f, TreeHeight + 6.0f), FVector(2.0f, 1.8f, 1.6f));
			}
		}
	}
}

void AParaglidePrototypeWorld::BuildClouds()
{
	const FVector CloudCenters[] = {
		FVector(-220.0f, 280.0f, 160.0f),
		FVector(140.0f, 420.0f, 172.0f),
		FVector(-80.0f, 760.0f, 190.0f),
		FVector(210.0f, 980.0f, 182.0f),
		FVector(-160.0f, 1360.0f, 210.0f)
	};

	for (const FVector& Center : CloudCenters)
	{
		AddSphereInstance(CloudInstances, Center, FVector(7.0f, 4.2f, 2.2f));
		AddSphereInstance(CloudInstances, Center + FVector(18.0f, -8.0f, 4.0f), FVector(4.5f, 3.0f, 1.9f));
		AddSphereInstance(CloudInstances, Center + FVector(-20.0f, 6.0f, -2.0f), FVector(5.6f, 3.5f, 2.0f));
	}
}

void AParaglidePrototypeWorld::BuildSkyMarkers()
{
	for (int32 Marker = -5; Marker <= 5; ++Marker)
	{
		const float X = Marker * 22.0f;
		const float Y = RidgeDistanceMeters - 20.0f + FMath::Abs(Marker) * 10.0f;
		AddBoxInstance(WindMarkerInstances, FVector(X, Y, 34.0f + FMath::Abs(Marker) * 2.0f), FVector(0.25f, 0.25f, 8.0f));
		AddBoxInstance(WindMarkerInstances, FVector(X + 1.4f, Y, 38.0f + FMath::Abs(Marker) * 2.0f), FVector(1.8f, 0.25f, 0.3f), FRotator(0.0f, 0.0f, Marker * 4.0f));
	}
}

void AParaglidePrototypeWorld::AddBoxInstance(UInstancedStaticMeshComponent* Component, const FVector& LocationMeters, const FVector& Scale, const FRotator& Rotation) const
{
	if (Component == nullptr)
	{
		return;
	}

	const FTransform InstanceTransform(Rotation, MeterToWorld(LocationMeters, WorldScale), Scale);
	Component->AddInstance(InstanceTransform);
}

void AParaglidePrototypeWorld::AddSphereInstance(UInstancedStaticMeshComponent* Component, const FVector& LocationMeters, const FVector& Scale, const FRotator& Rotation) const
{
	AddBoxInstance(Component, LocationMeters, Scale, Rotation);
}

void AParaglidePrototypeWorld::AddCylinderInstance(UInstancedStaticMeshComponent* Component, const FVector& LocationMeters, const FVector& Scale, const FRotator& Rotation) const
{
	AddBoxInstance(Component, LocationMeters, Scale, Rotation);
}

void AParaglidePrototypeWorld::AddTextLabel(UTextRenderComponent* LabelComponent, const FVector& LocationMeters, const FString& Text, const FLinearColor& Color) const
{
	if (LabelComponent == nullptr)
	{
		return;
	}

	LabelComponent->SetWorldSize(LabelWorldSize);
	LabelComponent->SetText(FText::FromString(Text));
	LabelComponent->SetTextRenderColor(Color.ToFColor(true));
	LabelComponent->SetRelativeLocation(ToWorldLocation(LocationMeters));
}

FVector AParaglidePrototypeWorld::ToWorldLocation(const FVector& LocationMeters) const
{
	return MeterToWorld(LocationMeters, WorldScale);
}
