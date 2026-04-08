#include "ParaglideGameMode.h"

#include "EngineUtils.h"
#include "World/ParaglidePrototypeWorld.h"
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
}

void AParaglideGameMode::StartPlay()
{
	Super::StartPlay();
	SpawnPrototypeWorldIfNeeded();
}

void AParaglideGameMode::SpawnPrototypeWorldIfNeeded()
{
	if (SpawnedPrototypeWorld != nullptr)
	{
		return;
	}

	for (TActorIterator<AParaglidePrototypeWorld> It(GetWorld()); It; ++It)
	{
		SpawnedPrototypeWorld = *It;
		return;
	}

	if (PrototypeWorldClass == nullptr || GetWorld() == nullptr)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnedPrototypeWorld = GetWorld()->SpawnActor<AParaglidePrototypeWorld>(PrototypeWorldClass, FTransform::Identity, SpawnParameters);
}
