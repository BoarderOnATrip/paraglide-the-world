// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/Destinations/ParaglideDestinationPack.h"
#include "World/Presentation/ParaglideWorldPresentationTypes.h"
#include "ParaglideWorldPresentationActor.generated.h"

class UParaglideWorldPresentationComponent;
class USceneComponent;

UCLASS()
class PARAGLIDETHEWORLDUE_API AParaglideWorldPresentationActor : public AActor
{
	GENERATED_BODY()

public:
	AParaglideWorldPresentationActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Paraglide|Presentation")
	void SetPresentationMode(EParaglideWorldPresentationMode NewMode);

	UFUNCTION(BlueprintPure, Category = "Paraglide|Presentation")
	EParaglideWorldPresentationMode GetPresentationMode() const;

	UFUNCTION(BlueprintCallable, Category = "Paraglide|Presentation")
	void SetDestinationAsset(TSoftObjectPtr<UParaglideDestinationPack> NewDestinationAsset);

	UFUNCTION(BlueprintPure, Category = "Paraglide|Presentation")
	const TSoftObjectPtr<UParaglideDestinationPack>& GetDestinationAsset() const;

	UFUNCTION(BlueprintPure, Category = "Paraglide|Presentation")
	UParaglideDestinationPack* GetLoadedDestinationAsset() const;

	UFUNCTION(BlueprintPure, Category = "Paraglide|Presentation")
	EParaglideWorldPresentationMode ResolveRequestedPresentationMode() const;

	UFUNCTION(BlueprintCallable, Category = "Paraglide|Presentation")
	void RefreshPresentation();

	UFUNCTION(BlueprintPure, Category = "Paraglide|Presentation")
	bool IsPresentationReady() const;

	UFUNCTION(BlueprintPure, Category = "Paraglide|Presentation")
	const FParaglideWorldPresentationReadiness& GetPresentationReadiness() const;

	UFUNCTION(BlueprintCallable, Category = "Paraglide|Presentation")
	void SelectProceduralFallback();

	UFUNCTION(BlueprintCallable, Category = "Paraglide|Presentation")
	void SelectGaussianSplatPlaceholder();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Presentation")
	EParaglideWorldPresentationMode PresentationMode = EParaglideWorldPresentationMode::Automatic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Presentation")
	TSoftObjectPtr<UParaglideDestinationPack> DestinationAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide|Presentation")
	TSubclassOf<AActor> ProceduralFallbackActorClass;

private:
	UPROPERTY(VisibleAnywhere, Category = "Paraglide|Presentation")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Paraglide|Presentation")
	TObjectPtr<UParaglideWorldPresentationComponent> PresentationComponent;

	UPROPERTY(Transient)
	TObjectPtr<UParaglideDestinationPack> LoadedDestinationAsset;
};
