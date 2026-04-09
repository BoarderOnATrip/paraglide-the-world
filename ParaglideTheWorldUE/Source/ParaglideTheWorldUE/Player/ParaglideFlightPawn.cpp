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

	UStaticMesh* CubeMesh = CubeMeshAsset.Succeeded() ? CubeMeshAsset.Object : nullptr;
	UStaticMesh* CylinderMesh = CylinderMeshAsset.Succeeded() ? CylinderMeshAsset.Object : nullptr;
	UStaticMesh* SphereMesh = SphereMeshAsset.Succeeded() ? SphereMeshAsset.Object : nullptr;

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

	HarnessSeat = CreateVisualMesh(TEXT("HarnessSeat"), HarnessRig, CubeMesh, FVector(-34.0f, 0.0f, -20.0f), FVector(0.78f, 0.42f, 0.22f), FRotator(10.0f, 0.0f, 0.0f));
	PilotTorso = CreateVisualMesh(TEXT("PilotTorso"), PilotRig, CubeMesh, FVector(4.0f, 0.0f, 42.0f), FVector(0.22f, 0.20f, 0.55f), FRotator(4.0f, 0.0f, 0.0f));
	PilotHead = CreateVisualMesh(TEXT("PilotHead"), PilotRig, SphereMesh, FVector(18.0f, 0.0f, 102.0f), FVector(0.18f, 0.18f, 0.18f));
	PilotLeftArm = CreateVisualMesh(TEXT("PilotLeftArm"), PilotRig, CubeMesh, FVector(-2.0f, -26.0f, 44.0f), FVector(0.10f, 0.09f, 0.42f), FRotator(-14.0f, 0.0f, -20.0f));
	PilotRightArm = CreateVisualMesh(TEXT("PilotRightArm"), PilotRig, CubeMesh, FVector(-2.0f, 26.0f, 44.0f), FVector(0.10f, 0.09f, 0.42f), FRotator(-14.0f, 0.0f, 20.0f));
	PilotLeftLeg = CreateVisualMesh(TEXT("PilotLeftLeg"), PilotRig, CubeMesh, FVector(-38.0f, -12.0f, -20.0f), FVector(0.13f, 0.10f, 0.50f), FRotator(-26.0f, 0.0f, -6.0f));
	PilotRightLeg = CreateVisualMesh(TEXT("PilotRightLeg"), PilotRig, CubeMesh, FVector(-38.0f, 12.0f, -20.0f), FVector(0.13f, 0.10f, 0.50f), FRotator(-26.0f, 0.0f, 6.0f));

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
			FVector(0.18f, 0.70f, 0.05f));
		const float Blend = static_cast<float>(CellIndex) / static_cast<float>(CanopyCellCount - 1);
		SetVisualColor(Cell, FLinearColor(0.98f - Blend * 0.24f, 0.24f + Blend * 0.32f, 0.12f + Blend * 0.26f, 1.0f));
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
	SpringArm->TargetArmLength = 1680.0f;
	SpringArm->SocketOffset = FVector(0.0f, 0.0f, 220.0f);
	SpringArm->TargetOffset = FVector(-240.0f, 0.0f, 110.0f);
	SpringArm->bDoCollisionTest = false;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = true;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 4.4f;
	SpringArm->CameraRotationLagSpeed = 8.6f;
	SpringArm->SetRelativeRotation(FRotator(-16.0f, 0.0f, 0.0f));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 78.0f;

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
	const float SymmetricFrontRiser = (ControlState.LeftFrontRiserTravel + ControlState.RightFrontRiserTravel) * 0.5f;
	const float SymmetricRearRiser = (ControlState.LeftRearRiserTravel + ControlState.RightRearRiserTravel) * 0.5f;
	const float SymmetricBrake = (ControlState.LeftBrakeTravel + ControlState.RightBrakeTravel) * 0.5f;
	const float FrontRiserDifferential = ControlState.RightFrontRiserTravel - ControlState.LeftFrontRiserTravel;
	const float RearRiserDifferential = ControlState.RightRearRiserTravel - ControlState.LeftRearRiserTravel;
	const float BrakeDifferential = ControlState.RightBrakeTravel - ControlState.LeftBrakeTravel;
	const float Inflation = FMath::Clamp(State.WingInflation, 0.0f, 1.0f);
	const float Pressure = FMath::Clamp(State.CanopyPressure, 0.0f, 1.2f);
	const float LeftCollapse = FMath::Clamp(State.LeftCollapseAmount, 0.0f, 1.0f);
	const float RightCollapse = FMath::Clamp(State.RightCollapseAmount, 0.0f, 1.0f);
	const float CollapseAverage = (LeftCollapse + RightCollapse) * 0.5f;
	const float CollapseDifferential = RightCollapse - LeftCollapse;
	const float AngleOfAttack = State.AngleOfAttackDeg;
	const float WingSurge = State.WingSurgeDeg;
	const float DiveAmount = State.DiveEnergy;
	const float TumbleAmount = State.TumbleAmount;
	const float LoadFactor = FMath::Clamp(State.LoadFactor, 1.0f, 2.8f);
	const float PressurePulse = Pressure * 6.0f + Inflation * 4.0f;
	const float WingPulse = FMath::Sin(State.ElapsedSeconds * (1.2f + DiveAmount * 0.7f) + DeltaSeconds) * (2.0f + TumbleAmount * 4.0f + PressurePulse * 0.25f);

	HarnessRig->SetRelativeLocation(FVector(
		-158.0f - DiveAmount * 42.0f - SymmetricFrontRiser * 16.0f + SymmetricRearRiser * 10.0f + WingSurge * 1.8f,
		ControlState.WeightShiftPosition * 18.0f + BrakeDifferential * 12.0f + RearRiserDifferential * 18.0f + FrontRiserDifferential * 10.0f,
		-176.0f - (LoadFactor - 1.0f) * 15.0f - SymmetricRearRiser * 5.0f + TumbleAmount * 18.0f + CollapseAverage * 16.0f));
	HarnessRig->SetRelativeRotation(FRotator(
		DiveAmount * 12.0f + SymmetricBrake * 10.0f + AngleOfAttack * 0.4f - SymmetricFrontRiser * 8.0f - State.FlareEffectiveness * 18.0f,
		0.0f,
		-State.BankDeg * 0.12f + ControlState.WeightShiftPosition * 8.0f + BrakeDifferential * 7.0f + RearRiserDifferential * 8.0f + CollapseDifferential * 6.0f));

	PilotRig->SetRelativeLocation(FVector(-20.0f, 0.0f, -30.0f + TumbleAmount * 10.0f + CollapseAverage * 8.0f));
	PilotRig->SetRelativeRotation(FRotator(
		12.0f + DiveAmount * 16.0f + SymmetricBrake * 16.0f + AngleOfAttack * 0.5f - SymmetricFrontRiser * 6.0f + TumbleAmount * 18.0f,
		0.0f,
		-State.BankDeg * 0.18f - ControlState.WeightShiftPosition * 10.0f - BrakeDifferential * 12.0f + RearRiserDifferential * 7.0f + CollapseDifferential * 8.0f));

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

	CanopyRig->SetRelativeLocation(FVector(
		144.0f + DiveAmount * 26.0f - SymmetricFrontRiser * 28.0f - SymmetricRearRiser * 10.0f + WingSurge * 3.2f,
		ControlState.WeightShiftPosition * 16.0f + BrakeDifferential * 18.0f + RearRiserDifferential * 24.0f + FrontRiserDifferential * 12.0f + CollapseDifferential * 26.0f,
		340.0f + Inflation * 22.0f + Pressure * 12.0f + (LoadFactor - 1.0f) * 14.0f + SymmetricRearRiser * 9.0f + WingPulse - CollapseAverage * 28.0f));
	CanopyRig->SetRelativeRotation(FRotator(
		-2.0f - DiveAmount * 12.0f - SymmetricFrontRiser * 14.0f + SymmetricBrake * 10.0f + AngleOfAttack * 0.35f + TumbleAmount * 10.0f - WingSurge * 0.7f,
		0.0f,
		-State.BankDeg * 0.14f - ControlState.WeightShiftPosition * 6.0f - BrakeDifferential * 4.0f + RearRiserDifferential * 5.0f + CollapseDifferential * 9.0f));

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

	const FVector LeftHarnessAnchor = HarnessRig->GetRelativeTransform().TransformPosition(FVector(16.0f, -24.0f, 18.0f));
	const FVector RightHarnessAnchor = HarnessRig->GetRelativeTransform().TransformPosition(FVector(16.0f, 24.0f, 18.0f));
	const FVector LeftRiserAnchor = HarnessRig->GetRelativeTransform().TransformPosition(FVector(52.0f, -38.0f, 142.0f));
	const FVector RightRiserAnchor = HarnessRig->GetRelativeTransform().TransformPosition(FVector(52.0f, 38.0f, 142.0f));
	const FVector CenterRiserAnchor = HarnessRig->GetRelativeTransform().TransformPosition(FVector(54.0f, 0.0f, 150.0f));
	const int32 LeftIndices[6] = {0, 1, 2, 3, 4, 5};
	const int32 RightIndices[6] = {7, 8, 9, 10, 11, 12};
	const int32 CenterIndices[2] = {5, 7};

	for (int32 LineIndex = 0; LineIndex < SuspensionLines.Num(); ++LineIndex)
	{
		UStaticMeshComponent* Line = SuspensionLines[LineIndex];
		if (Line == nullptr)
		{
			continue;
		}

		if (LineIndex < 6)
		{
			const int32 CellIndex = LeftIndices[LineIndex];
			if (!CellAnchorPoints.IsValidIndex(CellIndex))
			{
				continue;
			}

			SetLineMeshTransform(Line, CellAnchorPoints[CellIndex], LeftRiserAnchor, 1.1f);
			continue;
		}

		if (LineIndex < 12)
		{
			const int32 CellIndex = RightIndices[LineIndex - 6];
			if (!CellAnchorPoints.IsValidIndex(CellIndex))
			{
				continue;
			}

			SetLineMeshTransform(Line, CellAnchorPoints[CellIndex], RightRiserAnchor, 1.1f);
			continue;
		}

		if (LineIndex < 14)
		{
			const int32 CellIndex = CenterIndices[LineIndex - 12];
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
			SetLineMeshTransform(Line, CenterRiserAnchor, LeftHarnessAnchor, 1.4f);
			continue;
		}

		SetLineMeshTransform(Line, CenterRiserAnchor, RightHarnessAnchor, 1.4f);
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
