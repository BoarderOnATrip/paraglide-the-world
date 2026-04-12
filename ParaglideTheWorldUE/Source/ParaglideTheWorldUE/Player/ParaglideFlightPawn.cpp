#include "ParaglideFlightPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "Materials/MaterialInterface.h"
#include "ParaglideTheWorldUE/Flight/ParaglideFlightComponent.h"
#include "ParaglideTheWorldUE/Player/ParaglidePlayerController.h"
#include "ProceduralMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr int32 CanopyCellCount = 13;
	constexpr int32 SuspensionLineCount = 18;
}

AParaglideFlightPawn::AParaglideFlightPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> VertexColorMaterialAsset(TEXT("/Engine/EngineDebugMaterials/VertexColorMaterial.VertexColorMaterial"));

	UStaticMesh* CubeMesh = CubeMeshAsset.Succeeded() ? CubeMeshAsset.Object : nullptr;
	UStaticMesh* CylinderMesh = CylinderMeshAsset.Succeeded() ? CylinderMeshAsset.Object : nullptr;
	UStaticMesh* SphereMesh = SphereMeshAsset.Succeeded() ? SphereMeshAsset.Object : nullptr;
	VertexColorMaterial = VertexColorMaterialAsset.Succeeded() ? VertexColorMaterialAsset.Object : nullptr;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	FlightMarker = CreateDefaultSubobject<UArrowComponent>(TEXT("FlightMarker"));
	FlightMarker->SetupAttachment(SceneRoot);
	FlightMarker->ArrowColor = FColor(80, 220, 255);
	FlightMarker->ArrowSize = 2.7f;
	FlightMarker->SetRelativeLocation(FVector(-90.0f, 0.0f, -120.0f));
	FlightMarker->SetHiddenInGame(true);

	HarnessRig = CreateDefaultSubobject<USceneComponent>(TEXT("HarnessRig"));
	HarnessRig->SetupAttachment(SceneRoot);

	PilotRig = CreateDefaultSubobject<USceneComponent>(TEXT("PilotRig"));
	PilotRig->SetupAttachment(HarnessRig);

	CanopyRig = CreateDefaultSubobject<USceneComponent>(TEXT("CanopyRig"));
	CanopyRig->SetupAttachment(SceneRoot);

	CanopyTopSurface = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("CanopyTopSurface"));
	CanopyTopSurface->SetupAttachment(SceneRoot);
	CanopyTopSurface->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CanopyTopSurface->SetMobility(EComponentMobility::Movable);
	CanopyTopSurface->SetCastShadow(true);

	CanopyBottomSurface = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("CanopyBottomSurface"));
	CanopyBottomSurface->SetupAttachment(SceneRoot);
	CanopyBottomSurface->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CanopyBottomSurface->SetMobility(EComponentMobility::Movable);
	CanopyBottomSurface->SetCastShadow(true);

	CanopyEdgeSurface = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("CanopyEdgeSurface"));
	CanopyEdgeSurface->SetupAttachment(SceneRoot);
	CanopyEdgeSurface->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CanopyEdgeSurface->SetMobility(EComponentMobility::Movable);
	CanopyEdgeSurface->SetCastShadow(true);

	if (VertexColorMaterial != nullptr)
	{
		CanopyTopSurface->SetMaterial(0, VertexColorMaterial);
		CanopyBottomSurface->SetMaterial(0, VertexColorMaterial);
		CanopyEdgeSurface->SetMaterial(0, VertexColorMaterial);
	}

	HarnessSeat = CreateVisualMesh(TEXT("HarnessSeat"), HarnessRig, CubeMesh, FVector(-34.0f, 0.0f, -20.0f), FVector(0.78f, 0.42f, 0.22f), FRotator(10.0f, 0.0f, 0.0f));
	PilotTorso = CreateVisualMesh(TEXT("PilotTorso"), PilotRig, CubeMesh, FVector(4.0f, 0.0f, 42.0f), FVector(0.22f, 0.20f, 0.55f), FRotator(4.0f, 0.0f, 0.0f));
	PilotHead = CreateVisualMesh(TEXT("PilotHead"), PilotRig, SphereMesh, FVector(18.0f, 0.0f, 102.0f), FVector(0.18f, 0.18f, 0.18f));
	PilotLeftArm = CreateVisualMesh(TEXT("PilotLeftArm"), PilotRig, CubeMesh, FVector(-2.0f, -26.0f, 44.0f), FVector(0.10f, 0.09f, 0.42f), FRotator(-14.0f, 0.0f, -20.0f));
	PilotRightArm = CreateVisualMesh(TEXT("PilotRightArm"), PilotRig, CubeMesh, FVector(-2.0f, 26.0f, 44.0f), FVector(0.10f, 0.09f, 0.42f), FRotator(-14.0f, 0.0f, 20.0f));
	PilotLeftLeg = CreateVisualMesh(TEXT("PilotLeftLeg"), PilotRig, CubeMesh, FVector(-38.0f, -12.0f, -20.0f), FVector(0.13f, 0.10f, 0.50f), FRotator(-26.0f, 0.0f, -6.0f));
	PilotRightLeg = CreateVisualMesh(TEXT("PilotRightLeg"), PilotRig, CubeMesh, FVector(-38.0f, 12.0f, -20.0f), FVector(0.13f, 0.10f, 0.50f), FRotator(-26.0f, 0.0f, 6.0f));
	PilotHelmet = CreateVisualMesh(TEXT("PilotHelmet"), PilotRig, SphereMesh, FVector(24.0f, 0.0f, 108.0f), FVector(0.20f, 0.20f, 0.20f));
	PilotReservePack = CreateVisualMesh(TEXT("PilotReservePack"), PilotRig, CubeMesh, FVector(-24.0f, 0.0f, 16.0f), FVector(0.18f, 0.28f, 0.26f), FRotator(18.0f, 0.0f, 0.0f));
	HarnessChest = CreateVisualMesh(TEXT("HarnessChest"), HarnessRig, CubeMesh, FVector(-8.0f, 0.0f, 24.0f), FVector(0.24f, 0.34f, 0.14f), FRotator(-12.0f, 0.0f, 0.0f));

	SetVisualColor(HarnessSeat, FLinearColor(0.12f, 0.13f, 0.18f, 1.0f));
	SetVisualColor(PilotTorso, FLinearColor(0.16f, 0.22f, 0.28f, 1.0f));
	SetVisualColor(PilotHead, FLinearColor(0.93f, 0.78f, 0.67f, 1.0f));
	SetVisualColor(PilotLeftArm, FLinearColor(0.19f, 0.23f, 0.30f, 1.0f));
	SetVisualColor(PilotRightArm, FLinearColor(0.19f, 0.23f, 0.30f, 1.0f));
	SetVisualColor(PilotLeftLeg, FLinearColor(0.12f, 0.13f, 0.18f, 1.0f));
	SetVisualColor(PilotRightLeg, FLinearColor(0.12f, 0.13f, 0.18f, 1.0f));
	SetVisualColor(PilotHelmet, FLinearColor(0.18f, 0.22f, 0.28f, 1.0f));
	SetVisualColor(PilotReservePack, FLinearColor(0.50f, 0.20f, 0.12f, 1.0f));
	SetVisualColor(HarnessChest, FLinearColor(0.11f, 0.12f, 0.15f, 1.0f));

	LeadingEdgeCells.Reserve(CanopyCellCount);
	for (int32 CellIndex = 0; CellIndex < CanopyCellCount; ++CellIndex)
	{
		UStaticMeshComponent* Cell = CreateVisualMesh(
			FName(*FString::Printf(TEXT("LeadingEdgeCell%d"), CellIndex)),
			CanopyRig,
			CubeMesh,
			FVector::ZeroVector,
			FVector(0.20f, 0.82f, 0.08f));
		const float Blend = static_cast<float>(CellIndex) / static_cast<float>(CanopyCellCount - 1);
		SetVisualColor(Cell, FLinearColor(1.0f - Blend * 0.20f, 0.42f + Blend * 0.18f, 0.16f + Blend * 0.14f, 1.0f));
		Cell->SetHiddenInGame(true);
		LeadingEdgeCells.Add(Cell);
	}

	CanopyCells.Reserve(CanopyCellCount);
	for (int32 CellIndex = 0; CellIndex < CanopyCellCount; ++CellIndex)
	{
		UStaticMeshComponent* Cell = CreateVisualMesh(
			FName(*FString::Printf(TEXT("CanopyCell%d"), CellIndex)),
			CanopyRig,
			CubeMesh,
			FVector::ZeroVector,
			FVector(0.15f, 0.74f, 0.05f));
		const float Blend = static_cast<float>(CellIndex) / static_cast<float>(CanopyCellCount - 1);
		SetVisualColor(Cell, FLinearColor(0.92f - Blend * 0.22f, 0.30f + Blend * 0.28f, 0.18f + Blend * 0.20f, 1.0f));
		Cell->SetHiddenInGame(true);
		CanopyCells.Add(Cell);
	}

	TrailingEdgeCells.Reserve(CanopyCellCount);
	for (int32 CellIndex = 0; CellIndex < CanopyCellCount; ++CellIndex)
	{
		UStaticMeshComponent* Cell = CreateVisualMesh(
			FName(*FString::Printf(TEXT("TrailingEdgeCell%d"), CellIndex)),
			CanopyRig,
			CubeMesh,
			FVector::ZeroVector,
			FVector(0.10f, 0.66f, 0.03f));
		const float Blend = static_cast<float>(CellIndex) / static_cast<float>(CanopyCellCount - 1);
		SetVisualColor(Cell, FLinearColor(0.74f - Blend * 0.08f, 0.22f + Blend * 0.10f, 0.12f + Blend * 0.08f, 1.0f));
		Cell->SetHiddenInGame(true);
		TrailingEdgeCells.Add(Cell);
	}

	SuspensionLines.Reserve(SuspensionLineCount);
	for (int32 LineIndex = 0; LineIndex < SuspensionLineCount; ++LineIndex)
	{
		UStaticMeshComponent* Line = CreateVisualMesh(
			FName(*FString::Printf(TEXT("SuspensionLine%d"), LineIndex)),
			SceneRoot,
			CylinderMesh,
			FVector::ZeroVector,
			FVector(0.03f, 0.03f, 1.0f));
		SetVisualColor(Line, FLinearColor(0.82f, 0.88f, 0.96f, 1.0f));
		SuspensionLines.Add(Line);
	}

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(SceneRoot);
	SpringArm->TargetArmLength = 1820.0f;
	SpringArm->SocketOffset = FVector(0.0f, 0.0f, 260.0f);
	SpringArm->TargetOffset = FVector(-280.0f, 0.0f, 130.0f);
	SpringArm->bDoCollisionTest = false;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = true;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 4.8f;
	SpringArm->CameraRotationLagSpeed = 8.6f;
	SpringArm->SetRelativeRotation(FRotator(-18.0f, 0.0f, 0.0f));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 74.0f;

	FlightComponent = CreateDefaultSubobject<UParaglideFlightComponent>(TEXT("ParaglideFlightComponent"));

	UpdateVisualRig(0.0f);
}

UStaticMeshComponent* AParaglideFlightPawn::CreateVisualMesh(
	const FName ComponentName,
	USceneComponent* Parent,
	UStaticMesh* Mesh,
	const FVector& RelativeLocation,
	const FVector& RelativeScale,
	const FRotator& RelativeRotation,
	const bool bStartHiddenInGame)
{
	UStaticMeshComponent* MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(ComponentName);
	MeshComponent->SetupAttachment(Parent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->SetRelativeLocation(RelativeLocation);
	MeshComponent->SetRelativeRotation(RelativeRotation);
	MeshComponent->SetRelativeScale3D(RelativeScale);
	MeshComponent->SetHiddenInGame(bStartHiddenInGame);

	if (Mesh != nullptr)
	{
		MeshComponent->SetStaticMesh(Mesh);
	}

	return MeshComponent;
}

void AParaglideFlightPawn::SetVisualColor(UStaticMeshComponent* MeshComponent, const FLinearColor& Color) const
{
	if (MeshComponent == nullptr)
	{
		return;
	}

	const FVector ColorVector(Color.R, Color.G, Color.B);
	MeshComponent->SetVectorParameterValueOnMaterials(TEXT("Color"), ColorVector);
	MeshComponent->SetVectorParameterValueOnMaterials(TEXT("BaseColor"), ColorVector);
}

void AParaglideFlightPawn::SetLineMeshTransform(UStaticMeshComponent* MeshComponent, const FVector& Start, const FVector& End, const float Thickness) const
{
	if (MeshComponent == nullptr)
	{
		return;
	}

	const FVector Delta = End - Start;
	const float Length = Delta.Size();
	if (Length <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	MeshComponent->SetRelativeLocation((Start + End) * 0.5f);
	MeshComponent->SetRelativeRotation(FRotationMatrix::MakeFromZ(Delta.GetSafeNormal()).Rotator());
	MeshComponent->SetRelativeScale3D(FVector(
		FMath::Max(Thickness * 0.02f, 0.02f),
		FMath::Max(Thickness * 0.02f, 0.02f),
		FMath::Max(Length / 200.0f, 0.01f)));
}

void AParaglideFlightPawn::UpdateCanopyMesh(
	const TArray<FVector>& LeadingEdgeAnchors,
	const TArray<FVector>& MidChordAnchors,
	const TArray<FVector>& TrailingEdgeAnchors,
	const float Pressure,
	const float Inflation,
	const float LeftCollapse,
	const float RightCollapse)
{
	if (CanopyTopSurface == nullptr || CanopyBottomSurface == nullptr || CanopyEdgeSurface == nullptr)
	{
		return;
	}

	const int32 SpanCount = LeadingEdgeAnchors.Num();
	if (SpanCount < 2 || MidChordAnchors.Num() != SpanCount || TrailingEdgeAnchors.Num() != SpanCount)
	{
		CanopyTopSurface->ClearAllMeshSections();
		CanopyBottomSurface->ClearAllMeshSections();
		CanopyEdgeSurface->ClearAllMeshSections();
		return;
	}

	constexpr int32 ChordSamples = 7;
	constexpr float Pi = 3.14159265358979323846f;
	TArray<FVector> TopCurveVertices;
	TArray<FVector> BottomCurveVertices;
	TopCurveVertices.Reserve(SpanCount * ChordSamples);
	BottomCurveVertices.Reserve(SpanCount * ChordSamples);

	auto BuildSurfaceSection = [&](UProceduralMeshComponent* MeshComponent, const bool bBottomSurface)
	{
		TArray<FVector> Vertices;
		TArray<int32> Triangles;
		TArray<FVector2D> UVs;
		TArray<FLinearColor> Colors;
		TArray<FProcMeshTangent> Tangents;
		TArray<FVector> Normals;

		Vertices.Reserve(SpanCount * ChordSamples);
		UVs.Reserve(SpanCount * ChordSamples);
		Colors.Reserve(SpanCount * ChordSamples);

		for (int32 SpanIndex = 0; SpanIndex < SpanCount; ++SpanIndex)
		{
			const float SpanAlpha = SpanCount > 1 ? static_cast<float>(SpanIndex) / static_cast<float>(SpanCount - 1) : 0.0f;
			const float SpanSigned = SpanAlpha * 2.0f - 1.0f;
			const float LocalCollapse = SpanSigned < 0.0f ? LeftCollapse : RightCollapse;
			const float LocalPressure = FMath::Clamp(Pressure * (1.0f - LocalCollapse * 0.58f), 0.05f, 1.25f);
			const float LocalInflation = FMath::Clamp(Inflation * (1.0f - LocalCollapse * 0.68f), 0.08f, 1.0f);

			for (int32 ChordIndex = 0; ChordIndex < ChordSamples; ++ChordIndex)
			{
				const float T = static_cast<float>(ChordIndex) / static_cast<float>(ChordSamples - 1);
				FVector TopPoint =
					FMath::Square(1.0f - T) * LeadingEdgeAnchors[SpanIndex] +
					2.0f * (1.0f - T) * T * MidChordAnchors[SpanIndex] +
					FMath::Square(T) * TrailingEdgeAnchors[SpanIndex];
				const float ChordArch = FMath::Sin(T * Pi);
				const float Billow = ChordArch * (7.0f + LocalPressure * 8.5f + LocalInflation * 5.2f);
				const float LeadingNose = FMath::Clamp(1.0f - T * 2.2f, 0.0f, 1.0f) * (10.0f + LocalInflation * 6.0f);
				const float TrailingCurl = FMath::Clamp((T - 0.72f) / 0.28f, 0.0f, 1.0f) * (3.0f + LocalCollapse * 6.0f);
				TopPoint.Z += Billow + LeadingNose - TrailingCurl;
				const FVector BottomPoint = TopPoint - FVector(0.0f, 0.0f, 8.0f + ChordArch * (9.0f + LocalPressure * 6.0f + LocalInflation * 3.0f));
				const FVector Point = bBottomSurface ? BottomPoint : TopPoint;
				const float Stripe = 0.5f + 0.5f * FMath::Sin(SpanAlpha * Pi * 7.0f + T * Pi * 1.2f);
				const float LeadingHighlight = FMath::Clamp((0.28f - T) / 0.28f, 0.0f, 1.0f);
				const FLinearColor TopColor(
					0.96f - T * 0.10f - Stripe * 0.08f,
					0.24f + SpanAlpha * 0.20f + Stripe * 0.18f,
					0.14f + LocalPressure * 0.10f + LeadingHighlight * 0.12f,
					1.0f);
				const FLinearColor BottomColor(
					0.70f + LeadingHighlight * 0.12f,
					0.16f + SpanAlpha * 0.08f,
					0.12f + LocalPressure * 0.06f,
					1.0f);

				Vertices.Add(Point);
				UVs.Add(FVector2D(T, SpanAlpha));
				Colors.Add(bBottomSurface ? BottomColor : TopColor);
				if (bBottomSurface)
				{
					BottomCurveVertices.Add(BottomPoint);
				}
				else
				{
					TopCurveVertices.Add(TopPoint);
				}
			}
		}

		for (int32 SpanIndex = 0; SpanIndex < SpanCount - 1; ++SpanIndex)
		{
			for (int32 ChordIndex = 0; ChordIndex < ChordSamples - 1; ++ChordIndex)
			{
				const int32 A = SpanIndex * ChordSamples + ChordIndex;
				const int32 B = A + 1;
				const int32 C = A + ChordSamples;
				const int32 D = C + 1;

				if (bBottomSurface)
				{
					Triangles.Add(A);
					Triangles.Add(C);
					Triangles.Add(B);
					Triangles.Add(B);
					Triangles.Add(C);
					Triangles.Add(D);
				}
				else
				{
					Triangles.Add(A);
					Triangles.Add(B);
					Triangles.Add(C);
					Triangles.Add(B);
					Triangles.Add(D);
					Triangles.Add(C);
				}
			}
		}

		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UVs, Normals, Tangents);
		MeshComponent->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, Colors, Tangents, false);
	};

	BuildSurfaceSection(CanopyTopSurface, false);
	BuildSurfaceSection(CanopyBottomSurface, true);

	TArray<FVector> EdgeVertices;
	TArray<int32> EdgeTriangles;
	TArray<FVector2D> EdgeUVs;
	TArray<FLinearColor> EdgeColors;
	TArray<FVector> EdgeNormals;
	TArray<FProcMeshTangent> EdgeTangents;
	EdgeVertices.Reserve(SpanCount * 8 + ChordSamples * 8);
	EdgeUVs.Reserve(SpanCount * 8 + ChordSamples * 8);
	EdgeColors.Reserve(SpanCount * 8 + ChordSamples * 8);

	auto AppendQuadStrip = [&](const TArray<FVector>& StripA, const TArray<FVector>& StripB, const FLinearColor& ColorA, const FLinearColor& ColorB)
	{
		if (StripA.Num() != StripB.Num() || StripA.Num() < 2)
		{
			return;
		}

		const int32 BaseIndex = EdgeVertices.Num();
		for (int32 Index = 0; Index < StripA.Num(); ++Index)
		{
			const float Alpha = StripA.Num() > 1 ? static_cast<float>(Index) / static_cast<float>(StripA.Num() - 1) : 0.0f;
			EdgeVertices.Add(StripA[Index]);
			EdgeVertices.Add(StripB[Index]);
			EdgeUVs.Add(FVector2D(0.0f, Alpha));
			EdgeUVs.Add(FVector2D(1.0f, Alpha));
			EdgeColors.Add(ColorA);
			EdgeColors.Add(ColorB);
		}

		for (int32 Index = 0; Index < StripA.Num() - 1; ++Index)
		{
			const int32 A = BaseIndex + Index * 2;
			const int32 B = A + 1;
			const int32 C = A + 2;
			const int32 D = C + 1;
			EdgeTriangles.Add(A);
			EdgeTriangles.Add(B);
			EdgeTriangles.Add(C);
			EdgeTriangles.Add(B);
			EdgeTriangles.Add(D);
			EdgeTriangles.Add(C);
		}
	};

	TArray<FVector> LeadingTopStrip;
	TArray<FVector> LeadingBottomStrip;
	TArray<FVector> TrailingTopStrip;
	TArray<FVector> TrailingBottomStrip;
	TArray<FVector> LeftTipStrip;
	TArray<FVector> LeftBottomStrip;
	TArray<FVector> RightTipStrip;
	TArray<FVector> RightBottomStrip;
	LeadingTopStrip.Reserve(SpanCount);
	LeadingBottomStrip.Reserve(SpanCount);
	TrailingTopStrip.Reserve(SpanCount);
	TrailingBottomStrip.Reserve(SpanCount);
	LeftTipStrip.Reserve(ChordSamples);
	LeftBottomStrip.Reserve(ChordSamples);
	RightTipStrip.Reserve(ChordSamples);
	RightBottomStrip.Reserve(ChordSamples);

	for (int32 SpanIndex = 0; SpanIndex < SpanCount; ++SpanIndex)
	{
		const int32 LeadingIndex = SpanIndex * ChordSamples;
		const int32 TrailingIndex = LeadingIndex + (ChordSamples - 1);
		LeadingTopStrip.Add(TopCurveVertices[LeadingIndex]);
		LeadingBottomStrip.Add(BottomCurveVertices[LeadingIndex]);
		TrailingTopStrip.Add(TopCurveVertices[TrailingIndex]);
		TrailingBottomStrip.Add(BottomCurveVertices[TrailingIndex]);
	}

	for (int32 ChordIndex = 0; ChordIndex < ChordSamples; ++ChordIndex)
	{
		LeftTipStrip.Add(TopCurveVertices[ChordIndex]);
		LeftBottomStrip.Add(BottomCurveVertices[ChordIndex]);

		const int32 RightBaseIndex = (SpanCount - 1) * ChordSamples + ChordIndex;
		RightTipStrip.Add(TopCurveVertices[RightBaseIndex]);
		RightBottomStrip.Add(BottomCurveVertices[RightBaseIndex]);
	}

	AppendQuadStrip(LeadingTopStrip, LeadingBottomStrip, FLinearColor(0.98f, 0.86f, 0.54f, 1.0f), FLinearColor(0.78f, 0.36f, 0.16f, 1.0f));
	AppendQuadStrip(TrailingTopStrip, TrailingBottomStrip, FLinearColor(0.60f, 0.16f, 0.14f, 1.0f), FLinearColor(0.34f, 0.08f, 0.08f, 1.0f));
	AppendQuadStrip(LeftTipStrip, LeftBottomStrip, FLinearColor(0.86f, 0.30f, 0.16f, 1.0f), FLinearColor(0.44f, 0.10f, 0.10f, 1.0f));
	AppendQuadStrip(RightTipStrip, RightBottomStrip, FLinearColor(0.86f, 0.30f, 0.16f, 1.0f), FLinearColor(0.44f, 0.10f, 0.10f, 1.0f));

	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(EdgeVertices, EdgeTriangles, EdgeUVs, EdgeNormals, EdgeTangents);
	CanopyEdgeSurface->CreateMeshSection_LinearColor(0, EdgeVertices, EdgeTriangles, EdgeNormals, EdgeUVs, EdgeColors, EdgeTangents, false);
}

void AParaglideFlightPawn::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateVisualRig(DeltaSeconds);
}

void AParaglideFlightPawn::CalcCamera(const float DeltaTime, FMinimalViewInfo& OutResult)
{
	if (Camera)
	{
		Camera->GetCameraView(DeltaTime, OutResult);
		return;
	}

	Super::CalcCamera(DeltaTime, OutResult);
}

void AParaglideFlightPawn::BeginPlay()
{
	Super::BeginPlay();

	if (Camera)
	{
		Camera->SetActive(true);
	}

	if (FlightComponent)
	{
		FlightComponent->ResetScenario();
	}

	if (AParaglidePlayerController* ParaglideController = Cast<AParaglidePlayerController>(GetController()))
	{
		ParaglideController->SetViewTarget(this);
	}

	UpdateVisualRig(0.0f);
}

void AParaglideFlightPawn::UpdateVisualRig(const float DeltaSeconds)
{
	if (!HarnessRig || !PilotRig || !CanopyRig || !FlightComponent)
	{
		return;
	}

	const FParaglideFlightState& State = FlightComponent->GetFlightState();
	const FParaglideControlState& ControlState = FlightComponent->GetControlState();
	const float SymmetricFrontRiser = (ControlState.LeftFrontRiserTravel + ControlState.RightFrontRiserTravel) * 0.5f;
	const float SymmetricRearRiser = (ControlState.LeftRearRiserTravel + ControlState.RightRearRiserTravel) * 0.5f;
	const float SymmetricBrake = (ControlState.LeftBrakeTravel + ControlState.RightBrakeTravel) * 0.5f;
	const float FrontRiserDifferential = ControlState.RightFrontRiserTravel - ControlState.LeftFrontRiserTravel;
	const float RearRiserDifferential = ControlState.RightRearRiserTravel - ControlState.LeftRearRiserTravel;
	const float BrakeDifferential = ControlState.RightBrakeTravel - ControlState.LeftBrakeTravel;
	const float SpeedBarTravel = ControlState.SpeedBarTravel;
	const float Inflation = FMath::Clamp(State.WingInflation, 0.0f, 1.0f);
	const float Pressure = FMath::Clamp(State.CanopyPressure, 0.0f, 1.2f);
	const float LeftCollapse = FMath::Clamp(State.LeftCollapseAmount, 0.0f, 1.0f);
	const float RightCollapse = FMath::Clamp(State.RightCollapseAmount, 0.0f, 1.0f);
	const float CollapseAverage = (LeftCollapse + RightCollapse) * 0.5f;
	const float CollapseDifferential = RightCollapse - LeftCollapse;
	const float AngleOfAttack = State.AngleOfAttackDeg;
	const float WingSurge = State.WingSurgeDeg;
	const float DiveAmount = State.DiveEnergy;
	const float ManeuverEnergy = State.ManeuverEnergy;
	const float TumbleAmount = State.TumbleAmount;
	const float LoadFactor = FMath::Clamp(State.LoadFactor, 1.0f, 2.8f);
	const float GroundRush = FMath::Clamp((18.0f - State.GroundClearanceMeters) / 18.0f, 0.0f, 1.0f);
	const float PressurePulse = Pressure * 6.0f + Inflation * 4.0f;
	const float WingPulse = FMath::Sin(State.ElapsedSeconds * (1.2f + DiveAmount * 0.7f + ManeuverEnergy * 0.5f) + DeltaSeconds) * (2.0f + TumbleAmount * 4.0f + PressurePulse * 0.25f);

	HarnessRig->SetRelativeLocation(FVector(
		-158.0f - DiveAmount * 42.0f - ManeuverEnergy * 18.0f - SymmetricFrontRiser * 16.0f + SymmetricRearRiser * 10.0f + SpeedBarTravel * 26.0f + WingSurge * 1.8f,
		ControlState.WeightShiftPosition * 18.0f + BrakeDifferential * 12.0f + RearRiserDifferential * 18.0f + FrontRiserDifferential * 10.0f,
		-176.0f - (LoadFactor - 1.0f) * 15.0f - SymmetricRearRiser * 5.0f + TumbleAmount * 18.0f + CollapseAverage * 16.0f));
	HarnessRig->SetRelativeRotation(FRotator(
		DiveAmount * 12.0f + ManeuverEnergy * 8.0f + SymmetricBrake * 10.0f + AngleOfAttack * 0.4f - SymmetricFrontRiser * 8.0f + SpeedBarTravel * 12.0f - State.FlareEffectiveness * 18.0f,
		0.0f,
		-State.BankDeg * 0.12f + ControlState.WeightShiftPosition * 8.0f + BrakeDifferential * 7.0f + RearRiserDifferential * 8.0f + CollapseDifferential * 6.0f));

	PilotRig->SetRelativeLocation(FVector(-20.0f - SpeedBarTravel * 14.0f - ManeuverEnergy * 8.0f, 0.0f, -30.0f + TumbleAmount * 10.0f + CollapseAverage * 8.0f));
	PilotRig->SetRelativeRotation(FRotator(
		12.0f + DiveAmount * 16.0f + ManeuverEnergy * 10.0f + SymmetricBrake * 16.0f + AngleOfAttack * 0.5f - SymmetricFrontRiser * 6.0f + SpeedBarTravel * 14.0f + TumbleAmount * 18.0f,
		0.0f,
		-State.BankDeg * 0.18f - ControlState.WeightShiftPosition * 10.0f - BrakeDifferential * 12.0f + RearRiserDifferential * 7.0f + CollapseDifferential * 8.0f - ManeuverEnergy * 6.0f * FMath::Sign(BrakeDifferential + RearRiserDifferential + ControlState.WeightShiftPosition)));

	if (PilotHelmet)
	{
		PilotHelmet->SetRelativeLocation(FVector(24.0f + WingSurge * 0.4f, 0.0f, 108.0f + TumbleAmount * 8.0f));
		PilotHelmet->SetRelativeScale3D(FVector(0.20f, 0.20f, 0.20f));
	}

	if (PilotReservePack)
	{
		PilotReservePack->SetRelativeLocation(FVector(-24.0f - SymmetricRearRiser * 6.0f, 0.0f, 16.0f - DiveAmount * 6.0f));
		PilotReservePack->SetRelativeRotation(FRotator(18.0f + SymmetricBrake * 8.0f, 0.0f, 0.0f));
	}

	if (HarnessChest)
	{
		HarnessChest->SetRelativeLocation(FVector(-8.0f - SymmetricFrontRiser * 8.0f, 0.0f, 24.0f + SymmetricBrake * 4.0f));
		HarnessChest->SetRelativeRotation(FRotator(-12.0f + AngleOfAttack * 0.4f, 0.0f, 0.0f));
	}

	if (PilotLeftArm)
	{
		PilotLeftArm->SetRelativeRotation(FRotator(
			-6.0f + ControlState.LeftFrontRiserTravel * 78.0f + ControlState.LeftBrakeTravel * 64.0f - ControlState.LeftRearRiserTravel * 18.0f,
			0.0f,
			-18.0f - ControlState.LeftFrontRiserTravel * 8.0f - BrakeDifferential * 10.0f - ControlState.LeftRearRiserTravel * 7.0f));
	}

	if (PilotRightArm)
	{
		PilotRightArm->SetRelativeRotation(FRotator(
			-6.0f + ControlState.RightFrontRiserTravel * 78.0f + ControlState.RightBrakeTravel * 64.0f - ControlState.RightRearRiserTravel * 18.0f,
			0.0f,
			18.0f + ControlState.RightFrontRiserTravel * 8.0f - BrakeDifferential * 10.0f + ControlState.RightRearRiserTravel * 7.0f));
	}

	if (PilotLeftLeg)
	{
		PilotLeftLeg->SetRelativeRotation(FRotator(-18.0f - DiveAmount * 18.0f + SymmetricBrake * 12.0f, 0.0f, -7.0f));
	}

	if (PilotRightLeg)
	{
		PilotRightLeg->SetRelativeRotation(FRotator(-18.0f - DiveAmount * 18.0f + SymmetricBrake * 12.0f, 0.0f, 7.0f));
	}

	if (Camera)
	{
		Camera->SetFieldOfView(74.0f + DiveAmount * 10.0f + ManeuverEnergy * 7.0f + SpeedBarTravel * 7.0f + GroundRush * 4.0f - SymmetricBrake * 3.0f + TumbleAmount * 4.0f);
	}

	if (SpringArm)
	{
		SpringArm->TargetArmLength = 1780.0f + DiveAmount * 180.0f + ManeuverEnergy * 120.0f + SpeedBarTravel * 130.0f - SymmetricBrake * 40.0f - GroundRush * 150.0f;
		SpringArm->SocketOffset = FVector(0.0f, 0.0f, 250.0f + DiveAmount * 28.0f - GroundRush * 18.0f);
		SpringArm->TargetOffset = FVector(-280.0f - DiveAmount * 85.0f - ManeuverEnergy * 32.0f, 0.0f, 130.0f - SymmetricBrake * 18.0f - GroundRush * 30.0f);
		SpringArm->SetRelativeRotation(FRotator(-18.0f - DiveAmount * 8.0f - ManeuverEnergy * 4.0f - GroundRush * 5.0f + State.StallWarning * 3.0f + TumbleAmount * 6.0f, 0.0f, 0.0f));
	}

	CanopyRig->SetRelativeLocation(FVector(
		144.0f + DiveAmount * 26.0f + ManeuverEnergy * 12.0f - SymmetricFrontRiser * 28.0f - SymmetricRearRiser * 10.0f - SpeedBarTravel * 36.0f + WingSurge * 3.2f,
		ControlState.WeightShiftPosition * 16.0f + BrakeDifferential * 18.0f + RearRiserDifferential * 24.0f + FrontRiserDifferential * 12.0f + CollapseDifferential * 26.0f,
		340.0f + Inflation * 22.0f + Pressure * 12.0f + (LoadFactor - 1.0f) * 14.0f + SymmetricRearRiser * 9.0f + WingPulse - CollapseAverage * 28.0f + SpeedBarTravel * 20.0f + ManeuverEnergy * 10.0f));
	CanopyRig->SetRelativeRotation(FRotator(
		-2.0f - DiveAmount * 12.0f - ManeuverEnergy * 10.0f - SymmetricFrontRiser * 14.0f + SymmetricBrake * 10.0f + AngleOfAttack * 0.35f + SpeedBarTravel * 12.0f + TumbleAmount * 10.0f - WingSurge * 0.7f,
		0.0f,
		-State.BankDeg * 0.14f - ControlState.WeightShiftPosition * 6.0f - BrakeDifferential * 4.0f + RearRiserDifferential * 5.0f + CollapseDifferential * 9.0f - ManeuverEnergy * 8.0f * FMath::Sign(BrakeDifferential + RearRiserDifferential)));

	TArray<FVector> CellAnchorPoints;
	TArray<FVector> LeadingEdgeAnchors;
	TArray<FVector> TrailingEdgeAnchors;
	CellAnchorPoints.Reserve(CanopyCells.Num());
	LeadingEdgeAnchors.Reserve(LeadingEdgeCells.Num());
	TrailingEdgeAnchors.Reserve(TrailingEdgeCells.Num());

	for (int32 CellIndex = 0; CellIndex < CanopyCells.Num(); ++CellIndex)
	{
		UStaticMeshComponent* Cell = CanopyCells[CellIndex];
		if (Cell == nullptr)
		{
			continue;
		}

		const float SpanAlpha = CanopyCells.Num() > 1
			? (static_cast<float>(CellIndex) / static_cast<float>(CanopyCells.Num() - 1)) * 2.0f - 1.0f
			: 0.0f;
		const float SpanAbs = FMath::Abs(SpanAlpha);
		const float CenterBlend = 1.0f - SpanAbs;
		const float LocalCollapse = SpanAlpha < 0.0f ? LeftCollapse : RightCollapse;
		const float LocalInflation = FMath::Clamp(Inflation * (1.0f - LocalCollapse * 0.82f), 0.08f, 1.0f);
		const float LocalPressure = FMath::Clamp(Pressure * (1.0f - LocalCollapse * 0.66f), 0.05f, 1.2f);
		const float SpanOffset = SpanAlpha * 980.0f;
		const float ArcHeight =
			72.0f +
			LocalInflation * 78.0f -
			FMath::Square(SpanAlpha) * (72.0f + LocalCollapse * 28.0f) -
			SymmetricBrake * (24.0f + CenterBlend * 18.0f) -
			LocalCollapse * (54.0f + CenterBlend * 38.0f) +
			LocalPressure * 18.0f;
		const float BackShift =
			-SpanAbs * 42.0f -
			SymmetricBrake * (26.0f + CenterBlend * 16.0f) +
			DiveAmount * 10.0f +
			WingSurge * 1.4f -
			SymmetricFrontRiser * 18.0f -
			LocalCollapse * 42.0f;
		const float CellPitch =
			-1.0f -
			DiveAmount * 8.0f -
			ManeuverEnergy * 5.0f -
			SymmetricFrontRiser * 10.0f +
			SymmetricBrake * (9.0f + SpanAbs * 4.0f) +
			AngleOfAttack * 0.32f +
			LocalCollapse * 16.0f +
			TumbleAmount * FMath::Sin(State.ElapsedSeconds * 7.5f + SpanAlpha * 1.8f) * 10.0f;
		const float CellRoll =
			-SpanAlpha * (26.0f + DiveAmount * 8.0f + CollapseAverage * 10.0f) -
			BrakeDifferential * SpanAlpha * 14.0f +
			CollapseDifferential * SpanAbs * 16.0f;

		Cell->SetRelativeLocation(FVector(BackShift, SpanOffset, ArcHeight));
		Cell->SetRelativeRotation(FRotator(CellPitch, 0.0f, CellRoll));
		Cell->SetRelativeScale3D(FVector(
			0.13f + CenterBlend * 0.06f + LocalPressure * 0.02f,
			0.98f - SpanAbs * 0.18f + CenterBlend * 0.12f,
			0.028f + LocalPressure * 0.02f + LocalInflation * 0.012f));

		const float ColorBlend = static_cast<float>(CellIndex) / static_cast<float>(FMath::Max(CanopyCells.Num() - 1, 1));
		SetVisualColor(
			Cell,
			FLinearColor(
				(0.99f - ColorBlend * 0.24f) * (1.0f - LocalCollapse * 0.52f),
				(0.24f + ColorBlend * 0.32f) * (0.92f + LocalPressure * 0.08f),
				(0.12f + ColorBlend * 0.26f) * (0.92f + LocalInflation * 0.08f),
				1.0f));

		CellAnchorPoints.Add(CanopyRig->GetRelativeTransform().TransformPosition(Cell->GetRelativeLocation() + FVector(18.0f, 0.0f, -24.0f)));
	}

	for (int32 CellIndex = 0; CellIndex < LeadingEdgeCells.Num(); ++CellIndex)
	{
		UStaticMeshComponent* Cell = LeadingEdgeCells[CellIndex];
		if (Cell == nullptr)
		{
			continue;
		}

		const float SpanAlpha = LeadingEdgeCells.Num() > 1
			? (static_cast<float>(CellIndex) / static_cast<float>(LeadingEdgeCells.Num() - 1)) * 2.0f - 1.0f
			: 0.0f;
		const float SpanAbs = FMath::Abs(SpanAlpha);
		const float LocalCollapse = SpanAlpha < 0.0f ? LeftCollapse : RightCollapse;
		const float LocalInflation = FMath::Clamp(Inflation * (1.0f - LocalCollapse * 0.72f), 0.1f, 1.0f);
		const float LocalPressure = FMath::Clamp(Pressure * (1.0f - LocalCollapse * 0.45f), 0.08f, 1.2f);
		const float SpanOffset = SpanAlpha * 1080.0f;
		const float ArcHeight = 132.0f + LocalInflation * 92.0f - FMath::Square(SpanAlpha) * (58.0f + LocalCollapse * 26.0f) - SymmetricFrontRiser * 10.0f;
		const float BackShift = -28.0f - SpanAbs * 16.0f - SymmetricFrontRiser * 20.0f - LocalCollapse * 28.0f + WingSurge * 0.8f;
		const float CellPitch = -10.0f - DiveAmount * 10.0f + AngleOfAttack * 0.22f + LocalCollapse * 10.0f;
		const float CellRoll = -SpanAlpha * (16.0f + CollapseDifferential * 10.0f);

		Cell->SetRelativeLocation(FVector(BackShift, SpanOffset, ArcHeight));
		Cell->SetRelativeRotation(FRotator(CellPitch, 0.0f, CellRoll));
		Cell->SetRelativeScale3D(FVector(
			0.15f + LocalPressure * 0.05f,
			1.08f - SpanAbs * 0.18f,
			0.036f + LocalPressure * 0.012f));
		LeadingEdgeAnchors.Add(CanopyRig->GetRelativeTransform().TransformPosition(Cell->GetRelativeLocation() + FVector(36.0f, 0.0f, -20.0f)));
	}

	for (int32 CellIndex = 0; CellIndex < TrailingEdgeCells.Num(); ++CellIndex)
	{
		UStaticMeshComponent* Cell = TrailingEdgeCells[CellIndex];
		if (Cell == nullptr)
		{
			continue;
		}

		const float SpanAlpha = TrailingEdgeCells.Num() > 1
			? (static_cast<float>(CellIndex) / static_cast<float>(TrailingEdgeCells.Num() - 1)) * 2.0f - 1.0f
			: 0.0f;
		const float SpanAbs = FMath::Abs(SpanAlpha);
		const float LocalCollapse = SpanAlpha < 0.0f ? LeftCollapse : RightCollapse;
		const float LocalPressure = FMath::Clamp(Pressure * (1.0f - LocalCollapse * 0.58f), 0.04f, 1.0f);
		const float SpanOffset = SpanAlpha * 920.0f;
		const float ArcHeight = 42.0f + Inflation * 42.0f - FMath::Square(SpanAlpha) * (36.0f + LocalCollapse * 18.0f) - SymmetricBrake * 14.0f;
		const float BackShift = -96.0f - SpanAbs * 22.0f - SymmetricBrake * 12.0f - LocalCollapse * 18.0f;
		const float CellPitch = 6.0f + SymmetricBrake * 12.0f + AngleOfAttack * 0.18f + LocalCollapse * 14.0f;
		const float CellRoll = -SpanAlpha * (12.0f + CollapseDifferential * 12.0f);

		Cell->SetRelativeLocation(FVector(BackShift, SpanOffset, ArcHeight));
		Cell->SetRelativeRotation(FRotator(CellPitch, 0.0f, CellRoll));
		Cell->SetRelativeScale3D(FVector(
			0.10f + LocalPressure * 0.02f,
			0.80f - SpanAbs * 0.14f,
			0.022f + LocalPressure * 0.01f));
		TrailingEdgeAnchors.Add(CanopyRig->GetRelativeTransform().TransformPosition(Cell->GetRelativeLocation() + FVector(-12.0f, 0.0f, -18.0f)));
	}

	UpdateCanopyMesh(LeadingEdgeAnchors, CellAnchorPoints, TrailingEdgeAnchors, Pressure, Inflation, LeftCollapse, RightCollapse);

	const FVector LeftHarnessAnchor = HarnessRig->GetRelativeTransform().TransformPosition(FVector(16.0f, -24.0f, 18.0f));
	const FVector RightHarnessAnchor = HarnessRig->GetRelativeTransform().TransformPosition(FVector(16.0f, 24.0f, 18.0f));
	const FVector LeftRiserAnchor = HarnessRig->GetRelativeTransform().TransformPosition(FVector(52.0f, -38.0f, 142.0f));
	const FVector RightRiserAnchor = HarnessRig->GetRelativeTransform().TransformPosition(FVector(52.0f, 38.0f, 142.0f));
	const FVector CenterRiserAnchor = HarnessRig->GetRelativeTransform().TransformPosition(FVector(54.0f, 0.0f, 150.0f));
	for (int32 LineIndex = 0; LineIndex < SuspensionLines.Num(); ++LineIndex)
	{
		UStaticMeshComponent* Line = SuspensionLines[LineIndex];
		if (Line == nullptr)
		{
			continue;
		}

		if (LineIndex < 6)
		{
			if (!LeadingEdgeAnchors.IsValidIndex(LineIndex))
			{
				continue;
			}

			SetLineMeshTransform(Line, LeadingEdgeAnchors[LineIndex], LeftRiserAnchor, 1.0f);
			continue;
		}

		if (LineIndex < 12)
		{
			const int32 CellIndex = 6 + (LineIndex - 6);
			if (!LeadingEdgeAnchors.IsValidIndex(CellIndex))
			{
				continue;
			}

			SetLineMeshTransform(Line, LeadingEdgeAnchors[CellIndex], RightRiserAnchor, 1.0f);
			continue;
		}

		if (LineIndex < 14)
		{
			const int32 CellIndex = LineIndex - 12;
			if (!CellAnchorPoints.IsValidIndex(CellIndex))
			{
				continue;
			}

			SetLineMeshTransform(Line, CellAnchorPoints[CellIndex], CenterRiserAnchor, 1.0f);
			continue;
		}

		if (LineIndex == 14)
		{
			SetLineMeshTransform(Line, LeftRiserAnchor, LeftHarnessAnchor, 1.9f);
			continue;
		}

		if (LineIndex == 15)
		{
			SetLineMeshTransform(Line, RightRiserAnchor, RightHarnessAnchor, 1.9f);
			continue;
		}

		if (LineIndex == 16)
		{
			if (TrailingEdgeAnchors.IsValidIndex(4))
			{
				SetLineMeshTransform(Line, TrailingEdgeAnchors[4], LeftHarnessAnchor, 1.3f);
			}
			continue;
		}

		if (TrailingEdgeAnchors.IsValidIndex(8))
		{
			SetLineMeshTransform(Line, TrailingEdgeAnchors[8], RightHarnessAnchor, 1.3f);
		}
	}
}

void AParaglideFlightPawn::SetFlightControls(const FParaglideControlState& ControlState)
{
	if (FlightComponent)
	{
		FlightComponent->ApplyControlInputs(ControlState);
	}
}

void AParaglideFlightPawn::ResetCurrentScenario()
{
	if (FlightComponent)
	{
		FlightComponent->ResetScenario();
	}
}

void AParaglideFlightPawn::SelectScenarioByIndex(const int32 ScenarioIndex)
{
	if (FlightComponent)
	{
		FlightComponent->SelectScenarioByIndex(ScenarioIndex);
	}
}

FParaglideFlightState AParaglideFlightPawn::GetFlightState() const
{
	return FlightComponent ? FlightComponent->GetFlightState() : FParaglideFlightState {};
}

FString AParaglideFlightPawn::GetCurrentScenarioName() const
{
	return FlightComponent ? FlightComponent->GetCurrentScenarioName().ToString() : FString(TEXT("Free Flight"));
}

FString AParaglideFlightPawn::GetCurrentScenarioSummary() const
{
	return FlightComponent ? FlightComponent->GetCurrentScenarioSummary().ToString() : FString(TEXT("No scenario is active."));
}

int32 AParaglideFlightPawn::GetScenarioCount() const
{
	return FlightComponent ? FlightComponent->GetScenarioCount() : 0;
}

int32 AParaglideFlightPawn::GetCurrentScenarioIndex() const
{
	return FlightComponent ? FlightComponent->GetCurrentScenarioIndex() : INDEX_NONE;
}

UParaglideFlightComponent* AParaglideFlightPawn::GetFlightComponent() const
{
	return FlightComponent;
}
