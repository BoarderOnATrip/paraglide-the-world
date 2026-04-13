#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "World/Presentation/ParaglideWorldPresentationTypes.h"
#include "ParaglideGameMode.generated.h"

class AParaglidePrototypeWorld;
class UParaglideDestinationPack;

UCLASS(Config=Game, DefaultConfig)
class PARAGLIDETHEWORLDUE_API AParaglideGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AParaglideGameMode();
	virtual void StartPlay() override;

private:
	void SpawnRuntimeActorsIfNeeded();
	void SpawnRuntimeActorIfNeeded(TSubclassOf<AActor> ActorClass, TObjectPtr<AActor>& SpawnedActor);

	UPROPERTY(EditDefaultsOnly, Category = "Paraglide|World")
	TSubclassOf<AActor> PrototypeWorldClass;

	UPROPERTY(EditDefaultsOnly, Category = "Paraglide|World")
	TSubclassOf<AActor> DestinationActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Paraglide|World")
	TSubclassOf<AActor> PresentationActorClass;

	UPROPERTY(Config, EditDefaultsOnly, Category = "Paraglide|World")
	TSoftObjectPtr<UParaglideDestinationPack> DefaultDestinationPack;

	UPROPERTY(Config, EditDefaultsOnly, Category = "Paraglide|World")
	EParaglideWorldPresentationMode PreferredPresentationMode = EParaglideWorldPresentationMode::Automatic;

	UPROPERTY()
	TObjectPtr<AActor> SpawnedPrototypeWorld;

	UPROPERTY()
	TObjectPtr<AActor> SpawnedDestinationActor;

	UPROPERTY()
	TObjectPtr<AActor> SpawnedPresentationActor;
};
