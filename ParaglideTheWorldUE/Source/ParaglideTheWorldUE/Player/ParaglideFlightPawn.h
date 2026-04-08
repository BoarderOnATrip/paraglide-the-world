#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ParaglideTheWorldUE/Flight/ParaglideFlightTypes.h"
#include "ParaglideFlightPawn.generated.h"

class UCameraComponent;
class UArrowComponent;
class UParaglideFlightComponent;
class USpringArmComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class PARAGLIDETHEWORLDUE_API AParaglideFlightPawn : public APawn
{
	GENERATED_BODY()

public:
	AParaglideFlightPawn();
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;

	UFUNCTION(BlueprintCallable, Category="Paraglide")
	void SetFlightControls(const FParaglideControlState& ControlState);

	UFUNCTION(BlueprintCallable, Category="Paraglide")
	void ResetCurrentScenario();

	UFUNCTION(BlueprintCallable, Category="Paraglide")
	void SelectScenarioByIndex(int32 ScenarioIndex);

	UFUNCTION(BlueprintPure, Category="Paraglide")
	FParaglideFlightState GetFlightState() const;

	UFUNCTION(BlueprintPure, Category="Paraglide")
	FString GetCurrentScenarioName() const;

	UFUNCTION(BlueprintPure, Category="Paraglide")
	FString GetCurrentScenarioSummary() const;

	UFUNCTION(BlueprintPure, Category="Paraglide")
	int32 GetScenarioCount() const;

	UFUNCTION(BlueprintPure, Category="Paraglide")
	int32 GetCurrentScenarioIndex() const;

	UFUNCTION(BlueprintPure, Category="Paraglide")
	UParaglideFlightComponent* GetFlightComponent() const;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category="Paraglide")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category="Paraglide")
	TObjectPtr<UArrowComponent> FlightMarker;

	UPROPERTY(VisibleAnywhere, Category="Paraglide")
	TObjectPtr<UStaticMeshComponent> PilotBody;

	UPROPERTY(VisibleAnywhere, Category="Paraglide")
	TObjectPtr<UStaticMeshComponent> WingBody;

	UPROPERTY(VisibleAnywhere, Category="Paraglide")
	TObjectPtr<UStaticMeshComponent> CanopyBody;

	UPROPERTY(VisibleAnywhere, Category="Paraglide")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category="Paraglide")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, Category="Paraglide")
	TObjectPtr<UParaglideFlightComponent> FlightComponent;
};
