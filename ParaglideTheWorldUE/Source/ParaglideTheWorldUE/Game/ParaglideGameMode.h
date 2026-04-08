#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ParaglideGameMode.generated.h"

class AParaglidePrototypeWorld;

UCLASS()
class PARAGLIDETHEWORLDUE_API AParaglideGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AParaglideGameMode();
	virtual void StartPlay() override;

private:
	void SpawnPrototypeWorldIfNeeded();

	UPROPERTY(EditDefaultsOnly, Category = "Paraglide|World")
	TSubclassOf<AParaglidePrototypeWorld> PrototypeWorldClass;

	UPROPERTY()
	TObjectPtr<AParaglidePrototypeWorld> SpawnedPrototypeWorld;
};
