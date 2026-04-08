#include "ParaglidePlayerController.h"

#include "Engine/Engine.h"
#include "ParaglideTheWorldUE/Player/ParaglideFlightPawn.h"
#include "GameFramework/Pawn.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"

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

	InputComponent->BindKey(EKeys::A, IE_Pressed, this, &ThisClass::HandleWeightLeftPressed);
	InputComponent->BindKey(EKeys::A, IE_Released, this, &ThisClass::HandleWeightLeftReleased);
	InputComponent->BindKey(EKeys::F, IE_Pressed, this, &ThisClass::HandleLeftBrakePressed);
	InputComponent->BindKey(EKeys::F, IE_Released, this, &ThisClass::HandleLeftBrakeReleased);
	InputComponent->BindKey(EKeys::J, IE_Pressed, this, &ThisClass::HandleRightBrakePressed);
	InputComponent->BindKey(EKeys::J, IE_Released, this, &ThisClass::HandleRightBrakeReleased);
	InputComponent->BindKey(EKeys::Semicolon, IE_Pressed, this, &ThisClass::HandleWeightRightPressed);
	InputComponent->BindKey(EKeys::Semicolon, IE_Released, this, &ThisClass::HandleWeightRightReleased);
	InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ThisClass::HandleSpeedBarPressed);
	InputComponent->BindKey(EKeys::SpaceBar, IE_Released, this, &ThisClass::HandleSpeedBarReleased);
	InputComponent->BindKey(EKeys::R, IE_Pressed, this, &ThisClass::HandleResetPressed);
	InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &ThisClass::HandleHudTogglePressed);
	InputComponent->BindKey(EKeys::One, IE_Pressed, this, &ThisClass::HandleScenario1Pressed);
	InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ThisClass::HandleScenario2Pressed);
	InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ThisClass::HandleScenario3Pressed);
	InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &ThisClass::HandleScenario4Pressed);
	InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &ThisClass::HandleScenario5Pressed);
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

void AParaglidePlayerController::UpdateDerivedControlState()
{
	ControlState.LeftBrakeTravel = ControlState.bLeftBrakePressed ? 1.0f : 0.0f;
	ControlState.RightBrakeTravel = ControlState.bRightBrakePressed ? 1.0f : 0.0f;
	ControlState.SpeedBarTravel = ControlState.bSpeedBarPressed ? 1.0f : 0.0f;

	if (ControlState.bWeightLeftPressed && !ControlState.bWeightRightPressed)
	{
		ControlState.WeightShiftPosition = -1.0f;
	}
	else if (ControlState.bWeightRightPressed && !ControlState.bWeightLeftPressed)
	{
		ControlState.WeightShiftPosition = 1.0f;
	}
	else
	{
		ControlState.WeightShiftPosition = 0.0f;
	}
}

void AParaglidePlayerController::HandleWeightLeftPressed()
{
	ControlState.bWeightLeftPressed = true;
	UpdateDerivedControlState();
	PushControlState();
}

void AParaglidePlayerController::HandleWeightLeftReleased()
{
	ControlState.bWeightLeftPressed = false;
	UpdateDerivedControlState();
	PushControlState();
}

void AParaglidePlayerController::HandleLeftBrakePressed()
{
	ControlState.bLeftBrakePressed = true;
	UpdateDerivedControlState();
	PushControlState();
}

void AParaglidePlayerController::HandleLeftBrakeReleased()
{
	ControlState.bLeftBrakePressed = false;
	UpdateDerivedControlState();
	PushControlState();
}

void AParaglidePlayerController::HandleRightBrakePressed()
{
	ControlState.bRightBrakePressed = true;
	UpdateDerivedControlState();
	PushControlState();
}

void AParaglidePlayerController::HandleRightBrakeReleased()
{
	ControlState.bRightBrakePressed = false;
	UpdateDerivedControlState();
	PushControlState();
}

void AParaglidePlayerController::HandleWeightRightPressed()
{
	ControlState.bWeightRightPressed = true;
	UpdateDerivedControlState();
	PushControlState();
}

void AParaglidePlayerController::HandleWeightRightReleased()
{
	ControlState.bWeightRightPressed = false;
	UpdateDerivedControlState();
	PushControlState();
}

void AParaglidePlayerController::HandleSpeedBarPressed()
{
	ControlState.bSpeedBarPressed = true;
	UpdateDerivedControlState();
	PushControlState();
}

void AParaglidePlayerController::HandleSpeedBarReleased()
{
	ControlState.bSpeedBarPressed = false;
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

void AParaglidePlayerController::HandleScenarioIndexPressed(int32 ScenarioIndex)
{
	if (AParaglideFlightPawn* FlightPawn = GetParaglideFlightPawn())
	{
		FlightPawn->SelectScenarioByIndex(ScenarioIndex);
	}
}
