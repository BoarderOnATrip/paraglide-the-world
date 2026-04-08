#include "ParaglideFlightPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ParaglideTheWorldUE/Flight/ParaglideFlightComponent.h"
#include "ParaglideTheWorldUE/Player/ParaglidePlayerController.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr int32 CanopyCellCount = 7;
	constexpr int32 SuspensionLineCount = 6;
}

AParaglideFlightPawn::AParaglideFlightPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	UStaticMesh* CubeMesh = CubeMeshAsset.Succeeded() ? CubeMeshAsset.Object : nullptr;
	UStaticMesh* CylinderMesh = CylinderMeshAsset.Succeeded() ? CylinderMeshAsset.Object : nullptr;
	UStaticMesh* SphereMesh = SphereMeshAsset.Succeeded() ? SphereMeshAsset.Object : nullptr;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	FlightMarker = CreateDefaultSubobject<UArrowComponent>(TEXT("FlightMarker"));
	FlightMarker->SetupAttachment(SceneRoot);
	FlightMarker->ArrowColor = FColor(80, 220, 255);
	FlightMarker->ArrowSize = 2.3f;
	FlightMarker->SetRelativeLocation(FVector(-60.0f, 0.0f, -110.0f));
	FlightMarker->SetHiddenInGame(true);

	HarnessRig = CreateDefaultSubobject<USceneComponent>(TEXT("HarnessRig"));
	HarnessRig->SetupAttachment(SceneRoot);

	PilotRig = CreateDefaultSubobject<USceneComponent>(TEXT("PilotRig"));
	PilotRig->SetupAttachment(HarnessRig);

	CanopyRig = CreateDefaultSubobject<USceneComponent>(TEXT("CanopyRig"));
	CanopyRig->SetupAttachment(SceneRoot);

	HarnessSeat = CreateVisualMesh(TEXT("HarnessSeat"), HarnessRig, CubeMesh, FVector(-20.0f, 0.0f, -18.0f), FVector(0.42f, 0.55f, 0.16f));
	PilotTorso = CreateVisualMesh(TEXT("PilotTorso"), PilotRig, CubeMesh, FVector(6.0f, 0.0f, 36.0f), FVector(0.20f, 0.18f, 0.44f));
	PilotHead = CreateVisualMesh(TEXT("PilotHead"), PilotRig, SphereMesh, FVector(14.0f, 0.0f, 84.0f), FVector(0.15f, 0.15f, 0.15f));
	PilotLeftArm = CreateVisualMesh(TEXT("PilotLeftArm"), PilotRig, CubeMesh, FVector(-8.0f, -22.0f, 34.0f), FVector(0.12f, 0.10f, 0.34f), FRotator(-8.0f, 0.0f, -16.0f));
	PilotRightArm = CreateVisualMesh(TEXT("PilotRightArm"), PilotRig, CubeMesh, FVector(-8.0f, 22.0f, 34.0f), FVector(0.12f, 0.10f, 0.34f), FRotator(-8.0f, 0.0f, 16.0f));
	PilotLeftLeg = CreateVisualMesh(TEXT("PilotLeftLeg"), PilotRig, CubeMesh, FVector(-22.0f, -10.0f, -18.0f), FVector(0.14f, 0.11f, 0.42f), FRotator(-16.0f, 0.0f, -8.0f));
	PilotRightLeg = CreateVisualMesh(TEXT("PilotRightLeg"), PilotRig, CubeMesh, FVector(-22.0f, 10.0f, -18.0f), FVector(0.14f, 0.11f, 0.42f), FRotator(-16.0f, 0.0f, 8.0f));

	SetVisualColor(HarnessSeat, FLinearColor(0.12f, 0.13f, 0.18f, 1.0f));
	SetVisualColor(PilotTorso, FLinearColor(0.16f, 0.22f, 0.28f, 1.0f));
	SetVisualColor(PilotHead, FLinearColor(0.93f, 0.78f, 0.67f, 1.0f));
	SetVisualColor(PilotLeftArm, FLinearColor(0.19f, 0.23f, 0.30f, 1.0f));
	SetVisualColor(PilotRightArm, FLinearColor(0.19f, 0.23f, 0.30f, 1.0f));
	SetVisualColor(PilotLeftLeg, FLinearColor(0.12f, 0.13f, 0.18f, 1.0f));
	SetVisualColor(PilotRightLeg, FLinearColor(0.12f, 0.13f, 0.18f, 1.0f));

	CanopyCells.Reserve(CanopyCellCount);
	for (int32 CellIndex = 0; CellIndex < CanopyCellCount; ++CellIndex)
	{
		UStaticMeshComponent* Cell = CreateVisualMesh(
			FName(*FString::Printf(TEXT("CanopyCell%d"), CellIndex)),
			CanopyRig,
			CubeMesh,
			FVector::ZeroVector,
			FVector(0.15f, 1.1f, 0.04f));
		const float Blend = static_cast<float>(CellIndex) / static_cast<float>(CanopyCellCount - 1);
		SetVisualColor(Cell, FLinearColor(0.92f - Blend * 0.18f, 0.42f + Blend * 0.16f, 0.18f + Blend * 0.10f, 1.0f));
		CanopyCells.Add(Cell);
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
	SpringArm->TargetArmLength = 1520.0f;
	SpringArm->SocketOffset = FVector(0.0f, 0.0f, 170.0f);
	SpringArm->TargetOffset = FVector(-170.0f, 0.0f, 80.0f);
	SpringArm->bDoCollisionTest = false;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = true;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 3.8f;
	SpringArm->CameraRotationLagSpeed = 8.0f;
	SpringArm->SetRelativeRotation(FRotator(-12.0f, 0.0f, 0.0f));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 82.0f;

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
	const float SymmetricBrake = (ControlState.LeftBrakeTravel + ControlState.RightBrakeTravel) * 0.5f;
	const float BrakeDifferential = ControlState.RightBrakeTravel - ControlState.LeftBrakeTravel;
	const float DiveAmount = State.DiveEnergy;
	const float TumbleAmount = State.TumbleAmount;
	const float LoadFactor = FMath::Clamp(State.LoadFactor, 1.0f, 2.8f);
	const float WingPulse = FMath::Sin(State.ElapsedSeconds * (1.2f + DiveAmount * 0.7f) + DeltaSeconds) * (2.0f + TumbleAmount * 4.0f);

	HarnessRig->SetRelativeLocation(FVector(
		-126.0f - DiveAmount * 38.0f,
		BrakeDifferential * 10.0f,
		-148.0f - (LoadFactor - 1.0f) * 14.0f + TumbleAmount * 16.0f));
	HarnessRig->SetRelativeRotation(FRotator(
		DiveAmount * 12.0f + SymmetricBrake * 10.0f - State.FlareEffectiveness * 18.0f,
		0.0f,
		-State.BankDeg * 0.12f + BrakeDifferential * 7.0f));

	PilotRig->SetRelativeLocation(FVector(-14.0f, 0.0f, -26.0f + TumbleAmount * 10.0f));
	PilotRig->SetRelativeRotation(FRotator(
		10.0f + DiveAmount * 14.0f + SymmetricBrake * 14.0f + TumbleAmount * 18.0f,
		0.0f,
		-State.BankDeg * 0.18f - BrakeDifferential * 12.0f));

	if (PilotLeftArm)
	{
		PilotLeftArm->SetRelativeRotation(FRotator(12.0f + ControlState.LeftBrakeTravel * 65.0f, 0.0f, -18.0f - BrakeDifferential * 10.0f));
	}

	if (PilotRightArm)
	{
		PilotRightArm->SetRelativeRotation(FRotator(12.0f + ControlState.RightBrakeTravel * 65.0f, 0.0f, 18.0f - BrakeDifferential * 10.0f));
	}

	if (PilotLeftLeg)
	{
		PilotLeftLeg->SetRelativeRotation(FRotator(-18.0f - DiveAmount * 18.0f + SymmetricBrake * 12.0f, 0.0f, -7.0f));
	}

	if (PilotRightLeg)
	{
		PilotRightLeg->SetRelativeRotation(FRotator(-18.0f - DiveAmount * 18.0f + SymmetricBrake * 12.0f, 0.0f, 7.0f));
	}

	CanopyRig->SetRelativeLocation(FVector(
		96.0f + DiveAmount * 24.0f,
		BrakeDifferential * 16.0f,
		292.0f + (LoadFactor - 1.0f) * 12.0f + WingPulse));
	CanopyRig->SetRelativeRotation(FRotator(
		-6.0f - DiveAmount * 14.0f + SymmetricBrake * 18.0f + TumbleAmount * 10.0f,
		0.0f,
		-State.BankDeg * 0.14f - BrakeDifferential * 4.0f));

	TArray<FVector, TInlineAllocator<CanopyCellCount>> CellAnchorPoints;
	CellAnchorPoints.Reserve(CanopyCells.Num());

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
		const float SpanOffset = SpanAlpha * 760.0f;
		const float ArcHeight = 112.0f - FMath::Square(SpanAlpha) * 96.0f - SymmetricBrake * (34.0f + (1.0f - SpanAbs) * 18.0f);
		const float BackShift = -SpanAbs * 28.0f - SymmetricBrake * (42.0f + (1.0f - SpanAbs) * 18.0f) + DiveAmount * 10.0f;
		const float CellPitch = -3.0f - DiveAmount * 10.0f + SymmetricBrake * (14.0f + SpanAbs * 7.0f) +
			TumbleAmount * FMath::Sin(State.ElapsedSeconds * 7.5f + SpanAlpha * 1.8f) * 10.0f;
		const float CellRoll = -SpanAlpha * (20.0f + DiveAmount * 8.0f) - BrakeDifferential * SpanAlpha * 14.0f;

		Cell->SetRelativeLocation(FVector(BackShift, SpanOffset, ArcHeight));
		Cell->SetRelativeRotation(FRotator(CellPitch, 0.0f, CellRoll));
		Cell->SetRelativeScale3D(FVector(0.15f - SpanAbs * 0.02f, 1.24f - SpanAbs * 0.14f, 0.035f + (1.0f - SpanAbs) * 0.01f));

		CellAnchorPoints.Add(CanopyRig->GetRelativeTransform().TransformPosition(Cell->GetRelativeLocation() + FVector(8.0f, 0.0f, -18.0f)));
	}

	const FVector LeftHarnessAnchor = HarnessRig->GetRelativeTransform().TransformPosition(FVector(18.0f, -28.0f, 12.0f));
	const FVector RightHarnessAnchor = HarnessRig->GetRelativeTransform().TransformPosition(FVector(18.0f, 28.0f, 12.0f));
	const int32 LeftIndices[3] = {0, 1, 2};
	const int32 RightIndices[3] = {4, 5, 6};

	for (int32 LineIndex = 0; LineIndex < SuspensionLines.Num(); ++LineIndex)
	{
		UStaticMeshComponent* Line = SuspensionLines[LineIndex];
		if (Line == nullptr)
		{
			continue;
		}

		const bool bLeftSide = LineIndex < 3;
		const int32 CellIndex = bLeftSide ? LeftIndices[LineIndex] : RightIndices[LineIndex - 3];
		if (!CellAnchorPoints.IsValidIndex(CellIndex))
		{
			continue;
		}

		SetLineMeshTransform(Line, CellAnchorPoints[CellIndex], bLeftSide ? LeftHarnessAnchor : RightHarnessAnchor, 1.6f);
	}
}

void AParaglideFlightPawn::SetFlightControls(const FParaglideControlState& ControlState)
{
	if (FlightComponent)
	{
		FlightComponent->SetWeightLeftPressed(ControlState.bWeightLeftPressed);
		FlightComponent->SetLeftBrakePressed(ControlState.bLeftBrakePressed);
		FlightComponent->SetRightBrakePressed(ControlState.bRightBrakePressed);
		FlightComponent->SetWeightRightPressed(ControlState.bWeightRightPressed);
		FlightComponent->SetSpeedBarPressed(ControlState.bSpeedBarPressed);
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
