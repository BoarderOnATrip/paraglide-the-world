#include "ParaglideFlightPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "ParaglideTheWorldUE/Flight/ParaglideFlightComponent.h"
#include "ParaglideTheWorldUE/Player/ParaglidePlayerController.h"
#include "GameFramework/SpringArmComponent.h"

AParaglideFlightPawn::AParaglideFlightPawn()
{
	PrimaryActorTick.bCanEverTick = false;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	FlightMarker = CreateDefaultSubobject<UArrowComponent>(TEXT("FlightMarker"));
	FlightMarker->SetupAttachment(SceneRoot);
	FlightMarker->ArrowColor = FColor(80, 220, 255);
	FlightMarker->ArrowSize = 3.0f;
	FlightMarker->SetRelativeLocation(FVector(0.0f, 0.0f, -40.0f));
	FlightMarker->SetHiddenInGame(false);

	PilotBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PilotBody"));
	PilotBody->SetupAttachment(SceneRoot);
	PilotBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PilotBody->SetRelativeLocation(FVector(-55.0f, 0.0f, -35.0f));
	PilotBody->SetRelativeScale3D(FVector(0.75f, 0.5f, 0.22f));

	WingBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WingBody"));
	WingBody->SetupAttachment(SceneRoot);
	WingBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WingBody->SetRelativeLocation(FVector(10.0f, 0.0f, 45.0f));
	WingBody->SetRelativeScale3D(FVector(0.18f, 4.8f, 0.035f));

	CanopyBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CanopyBody"));
	CanopyBody->SetupAttachment(SceneRoot);
	CanopyBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CanopyBody->SetRelativeLocation(FVector(30.0f, 0.0f, 95.0f));
	CanopyBody->SetRelativeScale3D(FVector(0.1f, 6.8f, 0.03f));

	if (CubeMeshAsset.Succeeded())
	{
		PilotBody->SetStaticMesh(CubeMeshAsset.Object);
		WingBody->SetStaticMesh(CubeMeshAsset.Object);
		CanopyBody->SetStaticMesh(CubeMeshAsset.Object);
	}

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(SceneRoot);
	SpringArm->TargetArmLength = 1650.0f;
	SpringArm->SocketOffset = FVector(0.0f, 0.0f, 260.0f);
	SpringArm->TargetOffset = FVector(0.0f, 0.0f, 120.0f);
	SpringArm->bDoCollisionTest = false;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = true;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 3.2f;
	SpringArm->CameraRotationLagSpeed = 7.0f;
	SpringArm->SetRelativeRotation(FRotator(-18.0f, 0.0f, 0.0f));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 84.0f;

	FlightComponent = CreateDefaultSubobject<UParaglideFlightComponent>(TEXT("ParaglideFlightComponent"));
}

void AParaglideFlightPawn::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	if (Camera)
	{
		Camera->GetCameraView(DeltaTime, OutResult);
		return;
	}

	Super::CalcCamera(DeltaTime, OutResult);
}

void AParaglideFlightPawn::BeginPlay()
{
	Super::BeginPlay();

	if (Camera)
	{
		Camera->SetActive(true);
	}

	if (FlightComponent)
	{
		FlightComponent->ResetScenario();
	}

	if (AParaglidePlayerController* ParaglideController = Cast<AParaglidePlayerController>(GetController()))
	{
		ParaglideController->SetViewTarget(this);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			101,
			8.0f,
			FColor::Cyan,
			FString::Printf(
				TEXT("Pawn active: %s at (%.0f, %.0f, %.0f)"),
				*GetClass()->GetName(),
				GetActorLocation().X,
				GetActorLocation().Y,
				GetActorLocation().Z));
	}
}

void AParaglideFlightPawn::SetFlightControls(const FParaglideControlState& ControlState)
{
	if (FlightComponent)
	{
		FlightComponent->SetWeightLeftPressed(ControlState.bWeightLeftPressed);
		FlightComponent->SetLeftBrakePressed(ControlState.bLeftBrakePressed);
		FlightComponent->SetRightBrakePressed(ControlState.bRightBrakePressed);
		FlightComponent->SetWeightRightPressed(ControlState.bWeightRightPressed);
		FlightComponent->SetSpeedBarPressed(ControlState.bSpeedBarPressed);
	}
}

void AParaglideFlightPawn::ResetCurrentScenario()
{
	if (FlightComponent)
	{
		FlightComponent->ResetScenario();
	}
}

void AParaglideFlightPawn::SelectScenarioByIndex(int32 ScenarioIndex)
{
	if (FlightComponent)
	{
		FlightComponent->SelectScenarioByIndex(ScenarioIndex);
	}
}

FParaglideFlightState AParaglideFlightPawn::GetFlightState() const
{
	return FlightComponent ? FlightComponent->GetFlightState() : FParaglideFlightState{};
}

FString AParaglideFlightPawn::GetCurrentScenarioName() const
{
	return FlightComponent ? FlightComponent->GetCurrentScenarioName().ToString() : FString(TEXT("Free Flight"));
}

FString AParaglideFlightPawn::GetCurrentScenarioSummary() const
{
	return FlightComponent ? FlightComponent->GetCurrentScenarioSummary().ToString() : FString(TEXT("No scenario is active."));
}

int32 AParaglideFlightPawn::GetScenarioCount() const
{
	return FlightComponent ? FlightComponent->GetScenarioCount() : 0;
}

int32 AParaglideFlightPawn::GetCurrentScenarioIndex() const
{
	return FlightComponent ? FlightComponent->GetCurrentScenarioIndex() : INDEX_NONE;
}

UParaglideFlightComponent* AParaglideFlightPawn::GetFlightComponent() const
{
	return FlightComponent;
}
