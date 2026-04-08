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
	BuildLandingZone();
	BuildThermals();
	BuildGates();
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
}

void AParaglidePrototypeWorld::BuildLaunchDeck()
{
	AddBoxInstance(GroundDeckInstances, FVector(0.0f, 0.0f, -2.0f), FVector(34.0f, 12.0f, 0.5f));
	AddBoxInstance(GroundDeckInstances, FVector(0.0f, 28.0f, 4.0f), FVector(30.0f, 6.0f, 0.25f));
	AddBoxInstance(LaunchRampInstances, FVector(0.0f, -18.0f, 2.0f), FVector(10.0f, 10.0f, 0.35f), FRotator(-10.0f, 0.0f, 0.0f));
	AddBoxInstance(LaunchRampInstances, FVector(0.0f, -6.0f, 5.0f), FVector(10.0f, 10.0f, 0.35f), FRotator(-7.0f, 0.0f, 0.0f));
	AddBoxInstance(LaunchRampInstances, FVector(0.0f, 8.0f, 9.0f), FVector(10.0f, 10.0f, 0.35f), FRotator(-4.0f, 0.0f, 0.0f));
	AddBoxInstance(LaunchRampInstances, FVector(0.0f, 22.0f, 13.0f), FVector(10.0f, 10.0f, 0.35f), FRotator(-2.0f, 0.0f, 0.0f));
	AddBoxInstance(ParagliderCanopyInstances, FVector(0.0f, 16.0f, 31.0f), FVector(22.0f, 7.5f, 0.3f), FRotator(-8.0f, 0.0f, 0.0f));
	AddBoxInstance(ParagliderCanopyInstances, FVector(0.0f, 16.0f, 30.0f), FVector(12.0f, 5.2f, 0.15f), FRotator(8.0f, 0.0f, 0.0f));
	AddBoxInstance(ParagliderHarnessInstances, FVector(0.0f, 8.0f, 18.0f), FVector(1.6f, 1.0f, 4.0f));
	AddBoxInstance(ParagliderHarnessInstances, FVector(0.0f, 8.0f, 14.0f), FVector(1.0f, 0.8f, 1.8f));
	AddBoxInstance(ParagliderLineInstances, FVector(-4.5f, 12.0f, 26.0f), FVector(0.12f, 0.12f, 6.0f), FRotator(-20.0f, 0.0f, 16.0f));
	AddBoxInstance(ParagliderLineInstances, FVector(4.5f, 12.0f, 26.0f), FVector(0.12f, 0.12f, 6.0f), FRotator(-20.0f, 0.0f, -16.0f));
}

void AParaglidePrototypeWorld::BuildRidgeReference()
{
	const float RidgeBaseY = RidgeDistanceMeters;
	const int32 RidgeSegmentCount = FMath::Max(8, FMath::RoundToInt(RidgeLengthMeters / 26.0f));

	for (int32 Index = 0; Index < RidgeSegmentCount; ++Index)
	{
		const float SegmentAlpha = RidgeSegmentCount > 1 ? static_cast<float>(Index) / static_cast<float>(RidgeSegmentCount - 1) : 0.0f;
		const float X = FMath::Lerp(-RidgeLengthMeters * 0.5f, RidgeLengthMeters * 0.5f, SegmentAlpha);
		const float CrestHeight = 16.0f + FMath::Sin(SegmentAlpha * PI * 2.0f) * 3.0f;
		AddBoxInstance(RidgeInstances, FVector(X, RidgeBaseY, CrestHeight), FVector(12.0f, 8.0f, 9.0f));
		AddBoxInstance(RidgeInstances, FVector(X, RidgeBaseY - 18.0f, CrestHeight - 8.0f), FVector(8.0f, 6.0f, 4.0f));
	}

	for (int32 Marker = -3; Marker <= 3; ++Marker)
	{
		const float X = Marker * 42.0f;
		AddBoxInstance(RidgeInstances, FVector(X, RidgeBaseY + 10.0f, 30.0f), FVector(2.0f, 2.0f, 6.0f));
	}
}

void AParaglidePrototypeWorld::BuildLandingZone()
{
	const FVector ZoneCenter(0.0f, LandingZoneDistanceMeters, 0.75f);
	AddBoxInstance(LandingZoneInstances, ZoneCenter, FVector(LandingZoneLengthMeters * 0.5f, LandingZoneWidthMeters * 0.5f, 0.25f));
	AddBoxInstance(LandingZoneInstances, ZoneCenter + FVector(0.0f, 0.0f, 1.6f), FVector(LandingZoneLengthMeters * 0.48f, LandingZoneWidthMeters * 0.48f, 0.05f));
	AddBoxInstance(LandingZoneInstances, ZoneCenter + FVector(0.0f, -LandingZoneLengthMeters * 0.33f, 3.8f), FVector(18.0f, 1.5f, 1.2f));

	const FVector FrontLeft(-LandingZoneLengthMeters * 0.5f, LandingZoneDistanceMeters, 1.2f);
	const FVector FrontRight(LandingZoneLengthMeters * 0.5f, LandingZoneDistanceMeters, 1.2f);
	const FVector BackLeft(-LandingZoneLengthMeters * 0.5f, LandingZoneDistanceMeters + LandingZoneWidthMeters, 1.2f);
	const FVector BackRight(LandingZoneLengthMeters * 0.5f, LandingZoneDistanceMeters + LandingZoneWidthMeters, 1.2f);
	AddBoxInstance(LandingZoneInstances, FrontLeft, FVector(1.0f, LandingZoneWidthMeters * 0.5f, 0.4f));
	AddBoxInstance(LandingZoneInstances, FrontRight, FVector(1.0f, LandingZoneWidthMeters * 0.5f, 0.4f));
	AddBoxInstance(LandingZoneInstances, BackLeft, FVector(1.0f, LandingZoneWidthMeters * 0.5f, 0.4f));
	AddBoxInstance(LandingZoneInstances, BackRight, FVector(1.0f, LandingZoneWidthMeters * 0.5f, 0.4f));
}

void AParaglidePrototypeWorld::BuildThermals()
{
	const int32 ThermalTotal = FMath::Max(1, ThermalCount);
	for (int32 Index = 0; Index < ThermalTotal; ++Index)
	{
		const float Alpha = ThermalTotal > 1 ? static_cast<float>(Index) / static_cast<float>(ThermalTotal - 1) : 0.5f;
		const float X = FMath::Lerp(-140.0f, 180.0f, Alpha);
		const float Y = FMath::Lerp(450.0f, 1250.0f, Alpha);
		const float HeightMeters = 26.0f + Index * 6.0f;
		AddBoxInstance(ThermalColumnInstances, FVector(X, Y, HeightMeters * 0.5f), FVector(8.0f, 8.0f, HeightMeters * 0.5f));
		AddBoxInstance(ThermalCapInstances, FVector(X, Y, HeightMeters + 7.0f), FVector(5.5f, 5.5f, 5.5f));
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

void AParaglidePrototypeWorld::AddBoxInstance(UInstancedStaticMeshComponent* Component, const FVector& LocationMeters, const FVector& Scale, const FRotator& Rotation) const
{
	if (Component == nullptr)
	{
		return;
	}

	const FTransform InstanceTransform(Rotation, MeterToWorld(LocationMeters, WorldScale), Scale);
	Component->AddInstance(InstanceTransform);
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
