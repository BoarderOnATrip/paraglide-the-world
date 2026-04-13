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
	virtual bool BuildGaussianSplatPlaceholderPresentation(int32& OutLoadedChunkCount);
	virtual void ReleaseProceduralFallbackPresentation();
	virtual void ReleaseGaussianPresentation();

	AParaglideWorldPresentationActor* GetPresentationOwner() const;
	bool SpawnGaussianActorFromClassPath(const FSoftClassPath& ActorClassPath, const FTransform& LocalTransform);
	bool SpawnGaussianActorFromAssetPath(const FSoftObjectPath& AssetPath, const FTransform& LocalTransform);
	UClass* ResolveGaussianActorClass(const FSoftObjectPath& AssetPath) const;
	void UpdateReadinessFlags(
		EParaglideWorldPresentationMode RequestedMode,
		EParaglideWorldPresentationMode RuntimeMode,
		bool bProceduralReady,
		bool bGaussianPlaceholderReady,
		bool bSupportsSelectedMode,
		int32 TotalChunkCount,
		int32 DeclaredGaussianChunkCount,
		int32 LiveGaussianActorCount,
		EParaglideGaussianPresentationProvider PrimaryGaussianProvider,
		int32 LoadedChunkCount);

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Paraglide|Presentation")
	EParaglideWorldPresentationMode PresentationMode = EParaglideWorldPresentationMode::Automatic;

	UPROPERTY(VisibleInstanceOnly, Category = "Paraglide|Presentation")
	FParaglideWorldPresentationReadiness Readiness;

	UPROPERTY(Transient)
	TObjectPtr<AActor> SpawnedFallbackActor;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> SpawnedGaussianActors;
};
