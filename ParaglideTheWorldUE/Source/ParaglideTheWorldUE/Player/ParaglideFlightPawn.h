#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ParaglideTheWorldUE/Flight/ParaglideFlightTypes.h"
#include "ParaglideFlightPawn.generated.h"

class UArrowComponent;
class UCameraComponent;
class UParaglideFlightComponent;
class USceneComponent;
class USpringArmComponent;
class UStaticMesh;
class UStaticMeshComponent;

UCLASS()
class PARAGLIDETHEWORLDUE_API AParaglideFlightPawn : public APawn
{
	GENERATED_BODY()

public:
	AParaglideFlightPawn();

	virtual void Tick(float DeltaSeconds) override;
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;

	UFUNCTION(BlueprintCallable, Category = "Paraglide")
	void SetFlightControls(const FParaglideControlState& ControlState);

	UFUNCTION(BlueprintCallable, Category = "Paraglide")
	void ResetCurrentScenario();

	UFUNCTION(BlueprintCallable, Category = "Paraglide")
	void SelectScenarioByIndex(int32 ScenarioIndex);

	UFUNCTION(BlueprintPure, Category = "Paraglide")
	FParaglideFlightState GetFlightState() const;

	UFUNCTION(BlueprintPure, Category = "Paraglide")
	FString GetCurrentScenarioName() const;

	UFUNCTION(BlueprintPure, Category = "Paraglide")
	FString GetCurrentScenarioSummary() const;

	UFUNCTION(BlueprintPure, Category = "Paraglide")
	int32 GetScenarioCount() const;

	UFUNCTION(BlueprintPure, Category = "Paraglide")
	int32 GetCurrentScenarioIndex() const;

	UFUNCTION(BlueprintPure, Category = "Paraglide")
	UParaglideFlightComponent* GetFlightComponent() const;

protected:
	virtual void BeginPlay() override;

private:
	UStaticMeshComponent* CreateVisualMesh(
		const FName ComponentName,
		USceneComponent* Parent,
		UStaticMesh* Mesh,
		const FVector& RelativeLocation,
		const FVector& RelativeScale,
		const FRotator& RelativeRotation = FRotator::ZeroRotator,
		bool bStartHiddenInGame = false);

	void SetVisualColor(UStaticMeshComponent* MeshComponent, const FLinearColor& Color) const;
	void SetLineMeshTransform(UStaticMeshComponent* MeshComponent, const FVector& Start, const FVector& End, float Thickness) const;
	void UpdateVisualRig(float DeltaSeconds);

private:
	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UArrowComponent> FlightMarker;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<USceneComponent> HarnessRig;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<USceneComponent> PilotRig;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<USceneComponent> CanopyRig;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UStaticMeshComponent> HarnessSeat;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UStaticMeshComponent> PilotTorso;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UStaticMeshComponent> PilotHead;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UStaticMeshComponent> PilotLeftArm;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UStaticMeshComponent> PilotRightArm;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UStaticMeshComponent> PilotLeftLeg;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UStaticMeshComponent> PilotRightLeg;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TArray<TObjectPtr<UStaticMeshComponent>> CanopyCells;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TArray<TObjectPtr<UStaticMeshComponent>> SuspensionLines;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide")
	TObjectPtr<UParaglideFlightComponent> FlightComponent;
};
