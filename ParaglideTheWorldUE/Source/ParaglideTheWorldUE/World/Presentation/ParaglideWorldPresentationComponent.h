// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "World/Presentation/ParaglideWorldPresentationTypes.h"
#include "ParaglideWorldPresentationComponent.generated.h"

class AParaglideWorldPresentationActor;
class AActor;

UCLASS(ClassGroup = (Paraglide), meta = (BlueprintSpawnableComponent))
class PARAGLIDETHEWORLDUE_API UParaglideWorldPresentationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UParaglideWorldPresentationComponent();

	UFUNCTION(BlueprintCallable, Category = "Paraglide|Presentation")
	void SetPresentationMode(EParaglideWorldPresentationMode NewMode);

	UFUNCTION(BlueprintPure, Category = "Paraglide|Presentation")
	EParaglideWorldPresentationMode GetPresentationMode() const;

	UFUNCTION(BlueprintCallable, Category = "Paraglide|Presentation")
	bool ActivateProceduralFallbackPath();

	UFUNCTION(BlueprintCallable, Category = "Paraglide|Presentation")
	bool ActivateHybridPath();

	UFUNCTION(BlueprintCallable, Category = "Paraglide|Presentation")
	bool ActivateGaussianSplatPlaceholderPath();

	UFUNCTION(BlueprintCallable, Category = "Paraglide|Presentation")
	void RefreshPresentationPath();

	UFUNCTION(BlueprintPure, Category = "Paraglide|Presentation")
	const FParaglideWorldPresentationReadiness& GetReadiness() const;

	UFUNCTION(BlueprintPure, Category = "Paraglide|Presentation")
	bool IsPresentationReady() const;

	UFUNCTION(BlueprintPure, Category = "Paraglide|Presentation")
	bool CanUseGaussianSplatPlaceholder() const;

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;

	virtual bool BuildProceduralFallbackPresentation();
	virtual bool BuildGaussianSplatPlaceholderPresentation();
	virtual void ReleaseProceduralFallbackPresentation();

	AParaglideWorldPresentationActor* GetPresentationOwner() const;
	void UpdateReadinessFlags(
		EParaglideWorldPresentationMode RequestedMode,
		EParaglideWorldPresentationMode RuntimeMode,
		bool bProceduralReady,
		bool bGaussianPlaceholderReady,
		bool bSupportsSelectedMode,
		int32 TotalChunkCount,
		int32 DeclaredGaussianChunkCount);

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Paraglide|Presentation")
	EParaglideWorldPresentationMode PresentationMode = EParaglideWorldPresentationMode::Automatic;

	UPROPERTY(VisibleInstanceOnly, Category = "Paraglide|Presentation")
	FParaglideWorldPresentationReadiness Readiness;

	UPROPERTY(Transient)
	TObjectPtr<AActor> SpawnedFallbackActor;
};
