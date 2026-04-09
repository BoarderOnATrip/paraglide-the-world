// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Flight/ParaglideFlightTypes.h"
#include "ParaglideFlightComponent.generated.h"

UCLASS(ClassGroup=(Paraglide), meta=(BlueprintSpawnableComponent))
class PARAGLIDETHEWORLDUE_API UParaglideFlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UParaglideFlightComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ApplyControlInputs(const FParaglideControlState& ControlState);

	UFUNCTION(BlueprintCallable, Category = "Paraglide")
	void ResetScenario();

	UFUNCTION(BlueprintCallable, Category = "Paraglide")
	void SelectScenarioByIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Paraglide")
	void StepScenarioForward();

	UFUNCTION(BlueprintCallable, Category = "Paraglide")
	void StepScenarioBackward();

	const FParaglideFlightState& GetFlightState() const;
	const FParaglideControlState& GetControlState() const;
	const FParaglideFlightAssistProfile& GetAssistProfile() const;
	const FParaglideScenarioPreset& GetCurrentScenario() const;
	const TArray<FParaglideScenarioPreset>& GetScenarioPresets() const;

	FText GetCurrentScenarioName() const;
	FText GetCurrentScenarioSummary() const;
	FText GetCurrentScenarioRecommendedInputs() const;
	int32 GetCurrentScenarioIndex() const;
	int32 GetScenarioCount() const;

private:
	void ResetFromScenario();
	void ApplyOwnerTransform() const;
	void DrawScenarioDebug() const;

	FParaglideFlightState StepFlightState(const FParaglideFlightState& CurrentState, float DeltaSeconds) const;

private:
	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	FParaglideControlState Controls;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	FParaglideFlightAssistProfile AssistProfile;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	FParaglideAmbientAirState Atmosphere;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	FParaglideFlightState FlightState;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	int32 CurrentScenarioIndex = 0;

	TArray<FParaglideScenarioPreset> ScenarioPresets;
};
