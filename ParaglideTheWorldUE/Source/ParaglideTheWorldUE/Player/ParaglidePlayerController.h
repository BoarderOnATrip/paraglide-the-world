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
	FParaglideControlState ControlState;

	void PushControlState() const;
	AParaglideFlightPawn* GetParaglideFlightPawn() const;

	void HandleWeightLeftPressed();
	void HandleWeightLeftReleased();
	void HandleLeftBrakePressed();
	void HandleLeftBrakeReleased();
	void HandleRightBrakePressed();
	void HandleRightBrakeReleased();
	void HandleWeightRightPressed();
	void HandleWeightRightReleased();
	void HandleSpeedBarPressed();
	void HandleSpeedBarReleased();
	void HandleResetPressed();
	void HandleHudTogglePressed();
	void HandleScenario1Pressed();
	void HandleScenario2Pressed();
	void HandleScenario3Pressed();
	void HandleScenario4Pressed();
	void HandleScenario5Pressed();
	void HandleScenarioIndexPressed(int32 ScenarioIndex);
	void UpdateDerivedControlState();
};
