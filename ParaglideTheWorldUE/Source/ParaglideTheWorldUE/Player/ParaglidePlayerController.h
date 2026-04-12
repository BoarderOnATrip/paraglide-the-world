#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ParaglideTheWorldUE/Flight/ParaglideFlightTypes.h"
#include "ParaglidePlayerController.generated.h"

class AParaglideFlightPawn;

UCLASS()
class PARAGLIDETHEWORLDUE_API AParaglidePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AParaglidePlayerController();

	UFUNCTION(BlueprintCallable, Category="Paraglide")
	void ToggleParaglideHUD();

	UFUNCTION(BlueprintPure, Category="Paraglide")
	bool IsParaglideHUDVisible() const;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	UPROPERTY(VisibleInstanceOnly, Category="Paraglide")
	bool bParaglideHUDVisible = true;

	UPROPERTY(VisibleInstanceOnly, Category="Paraglide")
	bool bLeftOuterHomePressed = false;

	UPROPERTY(VisibleInstanceOnly, Category="Paraglide")
	bool bRightOuterHomePressed = false;

	UPROPERTY(VisibleInstanceOnly, Category="Paraglide")
	FParaglideControlState ControlState;

	void PushControlState() const;
	AParaglideFlightPawn* GetParaglideFlightPawn() const;
	void SetInputFlag(bool& InputFlag, bool bPressed);

	void HandleWeightLeftPressed();
	void HandleWeightLeftReleased();
	void HandleLeftRearRiserPressed();
	void HandleLeftRearRiserReleased();
	void HandleLeftBrakePrimaryPressed();
	void HandleLeftBrakePrimaryReleased();
	void HandleLeftBrakeDeepPressed();
	void HandleLeftBrakeDeepReleased();
	void HandleSpeedBarStageOnePressed();
	void HandleSpeedBarStageOneReleased();
	void HandleSpeedBarStageTwoPressed();
	void HandleSpeedBarStageTwoReleased();
	void HandleSpeedBarFallbackPressed();
	void HandleSpeedBarFallbackReleased();
	void HandleRightBrakeDeepPressed();
	void HandleRightBrakeDeepReleased();
	void HandleRightBrakePrimaryPressed();
	void HandleRightBrakePrimaryReleased();
	void HandleRightRearRiserPressed();
	void HandleRightRearRiserReleased();
	void HandleWeightRightPressed();
	void HandleWeightRightReleased();
	void HandleResetPressed();
	void HandleHudTogglePressed();
	void HandleScenario1Pressed();
	void HandleScenario2Pressed();
	void HandleScenario3Pressed();
	void HandleScenario4Pressed();
	void HandleScenario5Pressed();
	void HandleScenario6Pressed();
	void HandleScenario7Pressed();
	void HandleScenarioIndexPressed(int32 ScenarioIndex);
	void UpdateDerivedControlState();
};
