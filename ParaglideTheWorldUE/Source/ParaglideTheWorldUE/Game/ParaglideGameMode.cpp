#include "ParaglideGameMode.h"

#include "EngineUtils.h"
#include "World/ParaglidePrototypeWorld.h"
#include "World/Presentation/ParaglideWorldPresentationActor.h"
#include "ParaglideTheWorldUE/Player/ParaglideFlightPawn.h"
#include "ParaglideTheWorldUE/Player/ParaglidePlayerController.h"
#include "ParaglideTheWorldUE/UI/ParaglideHUD.h"
#include "Engine/World.h"

AParaglideGameMode::AParaglideGameMode()
{
	DefaultPawnClass = AParaglideFlightPawn::StaticClass();
	PlayerControllerClass = AParaglidePlayerController::StaticClass();
	HUDClass = AParaglideHUD::StaticClass();
	PrototypeWorldClass = AParaglidePrototypeWorld::StaticClass();
	PresentationActorClass = AParaglideWorldPresentationActor::StaticClass();
}

void AParaglideGameMode::StartPlay()
{
	Super::StartPlay();
	SpawnRuntimeActorsIfNeeded();
}

void AParaglideGameMode::SpawnRuntimeActorsIfNeeded()
{
	SpawnRuntimeActorIfNeeded(DestinationActorClass, SpawnedDestinationActor);
	SpawnRuntimeActorIfNeeded(PresentationActorClass, SpawnedPresentationActor);

	if (AParaglideWorldPresentationActor* PresentationActor = Cast<AParaglideWorldPresentationActor>(SpawnedPresentationActor))
	{
		PresentationActor->SetDestinationAsset(DefaultDestinationPack);
		PresentationActor->SetPresentationMode(PreferredPresentationMode);
		PresentationActor->RefreshPresentation();
		return;
	}

	SpawnRuntimeActorIfNeeded(PrototypeWorldClass, SpawnedPrototypeWorld);
}

void AParaglideGameMode::SpawnRuntimeActorIfNeeded(TSubclassOf<AActor> ActorClass, TObjectPtr<AActor>& SpawnedActor)
{
	if (SpawnedActor != nullptr || ActorClass == nullptr || GetWorld() == nullptr)
	{
		return;
	}

	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->IsA(ActorClass))
		{
			SpawnedActor = *It;
			return;
		}
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorClass, FTransform::Identity, SpawnParameters);
}
