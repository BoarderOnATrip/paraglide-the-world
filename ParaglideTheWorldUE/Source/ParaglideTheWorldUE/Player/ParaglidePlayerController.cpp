#include "ParaglidePlayerController.h"

#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "ParaglideTheWorldUE/Player/ParaglideFlightPawn.h"
#include "ParaglideTheWorldUE/World/Presentation/ParaglideWorldPresentationActor.h"
#include "ParaglideTheWorldUE/World/Presentation/ParaglideWorldPresentationTypes.h"
#include "GameFramework/Pawn.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"

namespace
{
	constexpr float RearRiserCommand = 1.0f;
	constexpr float BrakePrimaryStage = 0.58f;
	constexpr float BrakeDeepStage = 0.42f;
	constexpr float SpeedBarStageOne = 0.45f;
	constexpr float SpeedBarStageTwo = 0.55f;
}

AParaglidePlayerController::AParaglidePlayerController()
{
	bAutoManageActiveCameraTarget = false;
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
}

void AParaglidePlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetInputMode(FInputModeGameOnly());
	PushControlState();
}

void AParaglidePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	check(InputComponent);

	InputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &ThisClass::HandleWeightLeftPressed);
	InputComponent->BindKey(EKeys::LeftShift, IE_Released, this, &ThisClass::HandleWeightLeftReleased);
	InputComponent->BindKey(EKeys::S, IE_Pressed, this, &ThisClass::HandleLeftRearRiserPressed);
	InputComponent->BindKey(EKeys::S, IE_Released, this, &ThisClass::HandleLeftRearRiserReleased);
	InputComponent->BindKey(EKeys::D, IE_Pressed, this, &ThisClass::HandleLeftBrakePrimaryPressed);
	InputComponent->BindKey(EKeys::D, IE_Released, this, &ThisClass::HandleLeftBrakePrimaryReleased);
	InputComponent->BindKey(EKeys::F, IE_Pressed, this, &ThisClass::HandleLeftBrakeDeepPressed);
	InputComponent->BindKey(EKeys::F, IE_Released, this, &ThisClass::HandleLeftBrakeDeepReleased);
	InputComponent->BindKey(EKeys::G, IE_Pressed, this, &ThisClass::HandleSpeedBarStageOnePressed);
	InputComponent->BindKey(EKeys::G, IE_Released, this, &ThisClass::HandleSpeedBarStageOneReleased);
	InputComponent->BindKey(EKeys::H, IE_Pressed, this, &ThisClass::HandleSpeedBarStageTwoPressed);
	InputComponent->BindKey(EKeys::H, IE_Released, this, &ThisClass::HandleSpeedBarStageTwoReleased);
	InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ThisClass::HandleSpeedBarFallbackPressed);
	InputComponent->BindKey(EKeys::SpaceBar, IE_Released, this, &ThisClass::HandleSpeedBarFallbackReleased);
	InputComponent->BindKey(EKeys::J, IE_Pressed, this, &ThisClass::HandleRightBrakeDeepPressed);
	InputComponent->BindKey(EKeys::J, IE_Released, this, &ThisClass::HandleRightBrakeDeepReleased);
	InputComponent->BindKey(EKeys::K, IE_Pressed, this, &ThisClass::HandleRightBrakePrimaryPressed);
	InputComponent->BindKey(EKeys::K, IE_Released, this, &ThisClass::HandleRightBrakePrimaryReleased);
	InputComponent->BindKey(EKeys::L, IE_Pressed, this, &ThisClass::HandleRightRearRiserPressed);
	InputComponent->BindKey(EKeys::L, IE_Released, this, &ThisClass::HandleRightRearRiserReleased);
	InputComponent->BindKey(EKeys::RightShift, IE_Pressed, this, &ThisClass::HandleWeightRightPressed);
	InputComponent->BindKey(EKeys::RightShift, IE_Released, this, &ThisClass::HandleWeightRightReleased);
	InputComponent->BindKey(EKeys::R, IE_Pressed, this, &ThisClass::HandleResetPressed);
	InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &ThisClass::HandleHudTogglePressed);
	InputComponent->BindKey(EKeys::One, IE_Pressed, this, &ThisClass::HandleScenario1Pressed);
	InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ThisClass::HandleScenario2Pressed);
	InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ThisClass::HandleScenario3Pressed);
	InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &ThisClass::HandleScenario4Pressed);
	InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &ThisClass::HandleScenario5Pressed);
	InputComponent->BindKey(EKeys::Six, IE_Pressed, this, &ThisClass::HandleScenario6Pressed);
	InputComponent->BindKey(EKeys::Seven, IE_Pressed, this, &ThisClass::HandleScenario7Pressed);
	InputComponent->BindKey(EKeys::Eight, IE_Pressed, this, &ThisClass::HandlePresentationProceduralPressed);
	InputComponent->BindKey(EKeys::Nine, IE_Pressed, this, &ThisClass::HandlePresentationHybridPressed);
	InputComponent->BindKey(EKeys::Zero, IE_Pressed, this, &ThisClass::HandlePresentationGaussianPressed);
}

void AParaglidePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	SetViewTarget(InPawn);
	PushControlState();

	if (GEngine && InPawn)
	{
		GEngine->AddOnScreenDebugMessage(
			102,
			8.0f,
			FColor::Yellow,
			FString::Printf(
				TEXT("Controller active: %s possessing %s"),
				*GetClass()->GetName(),
				*InPawn->GetClass()->GetName()));
	}
}

void AParaglidePlayerController::ToggleParaglideHUD()
{
	bParaglideHUDVisible = !bParaglideHUDVisible;
}

bool AParaglidePlayerController::IsParaglideHUDVisible() const
{
	return bParaglideHUDVisible;
}

void AParaglidePlayerController::PushControlState() const
{
	if (AParaglideFlightPawn* FlightPawn = GetParaglideFlightPawn())
	{
		FlightPawn->SetFlightControls(ControlState);
	}
}

AParaglideFlightPawn* AParaglidePlayerController::GetParaglideFlightPawn() const
{
	return Cast<AParaglideFlightPawn>(GetPawn());
}

void AParaglidePlayerController::SetInputFlag(bool& InputFlag, const bool bPressed)
{
	InputFlag = bPressed;
	UpdateDerivedControlState();
	PushControlState();
}

void AParaglidePlayerController::UpdateDerivedControlState()
{
	const AParaglideFlightPawn* FlightPawn = GetParaglideFlightPawn();
	const FParaglideFlightState FlightState = FlightPawn ? FlightPawn->GetFlightState() : FParaglideFlightState {};
	const bool bUseFrontRisers =
		FlightState.FlightPhase == EParaglideFlightPhase::Launch ||
		(FlightState.GroundClearanceMeters < 6.0f && FlightState.WingInflation < 0.95f);

	ControlState.bLeftFrontRiserPressed = bUseFrontRisers && bLeftOuterHomePressed;
	ControlState.bRightFrontRiserPressed = bUseFrontRisers && bRightOuterHomePressed;
	ControlState.bWeightLeftPressed = !bUseFrontRisers && bLeftOuterHomePressed;
	ControlState.bWeightRightPressed = !bUseFrontRisers && bRightOuterHomePressed;
	ControlState.bLeftBrakePressed =
		ControlState.bLeftBrakePrimaryPressed ||
		ControlState.bLeftBrakeDeepPressed;
	ControlState.bRightBrakePressed =
		ControlState.bRightBrakePrimaryPressed ||
		ControlState.bRightBrakeDeepPressed;
	ControlState.bSpeedBarPressed =
		ControlState.bSpeedBarStageOnePressed ||
		ControlState.bSpeedBarStageTwoPressed;

	ControlState.LeftFrontRiserTarget = ControlState.bLeftFrontRiserPressed ? RearRiserCommand : 0.0f;
	ControlState.LeftRearRiserTarget = ControlState.bLeftRearRiserPressed ? RearRiserCommand : 0.0f;
	ControlState.LeftBrakeTarget = FMath::Clamp(
		(ControlState.bLeftBrakePrimaryPressed ? BrakePrimaryStage : 0.0f) +
		(ControlState.bLeftBrakeDeepPressed ? BrakeDeepStage : 0.0f),
		0.0f,
		1.0f);
	ControlState.RightFrontRiserTarget = ControlState.bRightFrontRiserPressed ? RearRiserCommand : 0.0f;
	ControlState.RightRearRiserTarget = ControlState.bRightRearRiserPressed ? RearRiserCommand : 0.0f;
	ControlState.RightBrakeTarget = FMath::Clamp(
		(ControlState.bRightBrakePrimaryPressed ? BrakePrimaryStage : 0.0f) +
		(ControlState.bRightBrakeDeepPressed ? BrakeDeepStage : 0.0f),
		0.0f,
		1.0f);
	ControlState.SpeedBarTarget = FMath::Clamp(
		(ControlState.bSpeedBarStageOnePressed ? SpeedBarStageOne : 0.0f) +
		(ControlState.bSpeedBarStageTwoPressed ? SpeedBarStageTwo : 0.0f),
		0.0f,
		1.0f);

	if (ControlState.bWeightLeftPressed && !ControlState.bWeightRightPressed)
	{
		ControlState.WeightShiftTarget = -1.0f;
	}
	else if (ControlState.bWeightRightPressed && !ControlState.bWeightLeftPressed)
	{
		ControlState.WeightShiftTarget = 1.0f;
	}
	else
	{
		ControlState.WeightShiftTarget = 0.0f;
	}
}

void AParaglidePlayerController::HandleWeightLeftPressed()
{
	bLeftOuterHomePressed = true;
	UpdateDerivedControlState();
	PushControlState();
}

void AParaglidePlayerController::HandleWeightLeftReleased()
{
	bLeftOuterHomePressed = false;
	UpdateDerivedControlState();
	PushControlState();
}

void AParaglidePlayerController::HandleLeftRearRiserPressed()
{
	SetInputFlag(ControlState.bLeftRearRiserPressed, true);
}

void AParaglidePlayerController::HandleLeftRearRiserReleased()
{
	SetInputFlag(ControlState.bLeftRearRiserPressed, false);
}

void AParaglidePlayerController::HandleLeftBrakePrimaryPressed()
{
	SetInputFlag(ControlState.bLeftBrakePrimaryPressed, true);
}

void AParaglidePlayerController::HandleLeftBrakePrimaryReleased()
{
	SetInputFlag(ControlState.bLeftBrakePrimaryPressed, false);
}

void AParaglidePlayerController::HandleLeftBrakeDeepPressed()
{
	SetInputFlag(ControlState.bLeftBrakeDeepPressed, true);
}

void AParaglidePlayerController::HandleLeftBrakeDeepReleased()
{
	SetInputFlag(ControlState.bLeftBrakeDeepPressed, false);
}

void AParaglidePlayerController::HandleSpeedBarStageOnePressed()
{
	SetInputFlag(ControlState.bSpeedBarStageOnePressed, true);
}

void AParaglidePlayerController::HandleSpeedBarStageOneReleased()
{
	SetInputFlag(ControlState.bSpeedBarStageOnePressed, false);
}

void AParaglidePlayerController::HandleSpeedBarStageTwoPressed()
{
	SetInputFlag(ControlState.bSpeedBarStageTwoPressed, true);
}

void AParaglidePlayerController::HandleSpeedBarStageTwoReleased()
{
	SetInputFlag(ControlState.bSpeedBarStageTwoPressed, false);
}

void AParaglidePlayerController::HandleSpeedBarFallbackPressed()
{
	ControlState.bSpeedBarStageOnePressed = true;
	ControlState.bSpeedBarStageTwoPressed = true;
	UpdateDerivedControlState();
	PushControlState();
}

void AParaglidePlayerController::HandleSpeedBarFallbackReleased()
{
	ControlState.bSpeedBarStageOnePressed = false;
	ControlState.bSpeedBarStageTwoPressed = false;
	UpdateDerivedControlState();
	PushControlState();
}

void AParaglidePlayerController::HandleRightBrakePrimaryPressed()
{
	SetInputFlag(ControlState.bRightBrakePrimaryPressed, true);
}

void AParaglidePlayerController::HandleRightBrakePrimaryReleased()
{
	SetInputFlag(ControlState.bRightBrakePrimaryPressed, false);
}

void AParaglidePlayerController::HandleRightRearRiserPressed()
{
	SetInputFlag(ControlState.bRightRearRiserPressed, true);
}

void AParaglidePlayerController::HandleRightRearRiserReleased()
{
	SetInputFlag(ControlState.bRightRearRiserPressed, false);
}

void AParaglidePlayerController::HandleRightBrakeDeepPressed()
{
	SetInputFlag(ControlState.bRightBrakeDeepPressed, true);
}

void AParaglidePlayerController::HandleRightBrakeDeepReleased()
{
	SetInputFlag(ControlState.bRightBrakeDeepPressed, false);
}

void AParaglidePlayerController::HandleWeightRightPressed()
{
	bRightOuterHomePressed = true;
	UpdateDerivedControlState();
	PushControlState();
}

void AParaglidePlayerController::HandleWeightRightReleased()
{
	bRightOuterHomePressed = false;
	UpdateDerivedControlState();
	PushControlState();
}

void AParaglidePlayerController::HandleResetPressed()
{
	if (AParaglideFlightPawn* FlightPawn = GetParaglideFlightPawn())
	{
		FlightPawn->ResetCurrentScenario();
	}
}

void AParaglidePlayerController::HandleHudTogglePressed()
{
	ToggleParaglideHUD();
}

void AParaglidePlayerController::HandleScenario1Pressed()
{
	HandleScenarioIndexPressed(0);
}

void AParaglidePlayerController::HandleScenario2Pressed()
{
	HandleScenarioIndexPressed(1);
}

void AParaglidePlayerController::HandleScenario3Pressed()
{
	HandleScenarioIndexPressed(2);
}

void AParaglidePlayerController::HandleScenario4Pressed()
{
	HandleScenarioIndexPressed(3);
}

void AParaglidePlayerController::HandleScenario5Pressed()
{
	HandleScenarioIndexPressed(4);
}

void AParaglidePlayerController::HandleScenario6Pressed()
{
	HandleScenarioIndexPressed(5);
}

void AParaglidePlayerController::HandleScenario7Pressed()
{
	HandleScenarioIndexPressed(6);
}

void AParaglidePlayerController::HandlePresentationProceduralPressed()
{
	SetPresentationMode(EParaglideWorldPresentationMode::ProceduralFallback);
}

void AParaglidePlayerController::HandlePresentationHybridPressed()
{
	SetPresentationMode(EParaglideWorldPresentationMode::Hybrid);
}

void AParaglidePlayerController::HandlePresentationGaussianPressed()
{
	SetPresentationMode(EParaglideWorldPresentationMode::GaussianSplatPlaceholder);
}

void AParaglidePlayerController::HandleScenarioIndexPressed(int32 ScenarioIndex)
{
	if (AParaglideFlightPawn* FlightPawn = GetParaglideFlightPawn())
	{
		FlightPawn->SelectScenarioByIndex(ScenarioIndex);
	}
}

AParaglideWorldPresentationActor* AParaglidePlayerController::GetPresentationActor() const
{
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AParaglideWorldPresentationActor> It(World); It; ++It)
		{
			return *It;
		}
	}

	return nullptr;
}

void AParaglidePlayerController::SetPresentationMode(const EParaglideWorldPresentationMode NewMode)
{
	if (AParaglideWorldPresentationActor* PresentationActor = GetPresentationActor())
	{
		PresentationActor->SetPresentationMode(NewMode);
		PresentationActor->RefreshPresentation();

		if (GEngine != nullptr)
		{
			const TCHAR* ModeLabel = TEXT("Procedural");
			switch (NewMode)
			{
			case EParaglideWorldPresentationMode::Hybrid:
				ModeLabel = TEXT("Hybrid");
				break;
			case EParaglideWorldPresentationMode::GaussianSplatPlaceholder:
				ModeLabel = TEXT("Gaussian");
				break;
			case EParaglideWorldPresentationMode::ProceduralFallback:
			case EParaglideWorldPresentationMode::Automatic:
			default:
				ModeLabel = TEXT("Procedural");
				break;
			}

			GEngine->AddOnScreenDebugMessage(
				104,
				5.0f,
				FColor(120, 220, 255),
				FString::Printf(TEXT("World mode: %s"), ModeLabel));
		}
	}
}
