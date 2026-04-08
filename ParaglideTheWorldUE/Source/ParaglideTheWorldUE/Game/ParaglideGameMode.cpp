#include "ParaglideGameMode.h"

#include "ParaglideTheWorldUE/Player/ParaglideFlightPawn.h"
#include "ParaglideTheWorldUE/Player/ParaglidePlayerController.h"
#include "ParaglideTheWorldUE/UI/ParaglideHUD.h"

AParaglideGameMode::AParaglideGameMode()
{
	DefaultPawnClass = AParaglideFlightPawn::StaticClass();
	PlayerControllerClass = AParaglidePlayerController::StaticClass();
	HUDClass = AParaglideHUD::StaticClass();
}
