// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ParaglideFlightTypes.generated.h"

UENUM(BlueprintType)
enum class EParaglideFlightPhase : uint8
{
	Launch,
	Soaring,
	Approach,
	Flare,
	Landed,
	Crashed,
};

UENUM(BlueprintType)
enum class EParaglideLandingRating : uint8
{
	None,
	Smooth,
	Firm,
	Hard,
	Crash,
};

USTRUCT(BlueprintType)
struct FParaglideAmbientAirState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float WindHeadingDeg = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float WindSpeedKmh = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float Turbulence = 0.16f;
};

USTRUCT(BlueprintType)
struct FParaglideFlightDebugTelemetry
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float BaseSinkMetersPerSecond = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float InducedTurnSinkMetersPerSecond = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float BrakeSinkMetersPerSecond = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float StallSinkMetersPerSecond = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float DiveSinkMetersPerSecond = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float SpiralSinkMetersPerSecond = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float TumbleSinkMetersPerSecond = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float TurbulenceLiftMetersPerSecond = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float FlareLiftMetersPerSecond = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float TotalSinkMetersPerSecond = 0.0f;
};

USTRUCT(BlueprintType)
struct FParaglideFlightState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	FVector2D PositionMeters = FVector2D::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float AltitudeMeters = 120.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float TerrainHeightMeters = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float HeadingDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float BankDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float PitchDeg = -6.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float AirspeedKmh = 38.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float GroundSpeedKmh = 38.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float VerticalSpeedMetersPerSecond = 0.1f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float RidgeLiftMetersPerSecond = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float ThermalLiftMetersPerSecond = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float AirMassSinkMetersPerSecond = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float GroundClearanceMeters = 120.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float DistanceKm = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float ElapsedSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float TurnRateDegPerSecond = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float StallWarning = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float FlareEffectiveness = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float WingInflation = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float CanopyPressure = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float AngleOfAttackDeg = 6.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float LeftCollapseAmount = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float RightCollapseAmount = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float WingSurgeDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float DiveEnergy = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float TumbleAmount = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float SpinRateDegPerSecond = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float LoadFactor = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float LandingZoneDistanceMeters = -1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float LandingApproachErrorDeg = -1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	EParaglideLandingRating LandingRating = EParaglideLandingRating::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	EParaglideFlightPhase FlightPhase = EParaglideFlightPhase::Launch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	FParaglideFlightDebugTelemetry Debug;
};

USTRUCT(BlueprintType)
struct FParaglideControlState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bWeightLeftPressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bLeftBrakePressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bRightBrakePressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bWeightRightPressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bSpeedBarPressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bLeftFrontRiserPressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bLeftRearRiserPressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bLeftBrakePrimaryPressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bLeftBrakeDeepPressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bRightBrakeDeepPressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bRightBrakePrimaryPressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bRightRearRiserPressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bRightFrontRiserPressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bSpeedBarStageOnePressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	bool bSpeedBarStageTwoPressed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float LeftFrontRiserTarget = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float LeftRearRiserTarget = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float LeftBrakeTarget = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float RightFrontRiserTarget = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float RightRearRiserTarget = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float RightBrakeTarget = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float WeightShiftTarget = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float SpeedBarTarget = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float LeftFrontRiserTravel = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float LeftRearRiserTravel = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float LeftBrakeTravel = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float RightFrontRiserTravel = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float RightRearRiserTravel = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float RightBrakeTravel = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float WeightShiftPosition = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paraglide")
	float SpeedBarTravel = 0.0f;
};

USTRUCT(BlueprintType)
struct FParaglideFlightAssistProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float InputResponsiveness = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float CoordinationAssist = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float TurbulenceDamping = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float RecoveryAssist = 0.0f;
};

USTRUCT(BlueprintType)
struct FParaglideThermalSource
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FName Id = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FVector2D PositionMeters = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float RadiusMeters = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float CoreRadiusMeters = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float LiftMetersPerSecond = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float SinkRingMetersPerSecond = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float DriftFactor = 0.28f;
};

USTRUCT(BlueprintType)
struct FParaglideRidgeBand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	bool bEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FVector2D PositionMeters = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float AxisHeadingDeg = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float LengthMeters = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float WindwardDepthMeters = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float LeeDepthMeters = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float PeakLiftMetersPerSecond = 1.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float LeeSinkMetersPerSecond = 1.0f;
};

USTRUCT(BlueprintType)
struct FParaglideLandingZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	bool bEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FVector2D PositionMeters = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float HeadingDeg = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float LengthMeters = 240.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float WidthMeters = 80.0f;
};

USTRUCT(BlueprintType)
struct FParaglideFlightSite
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FName Id = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FText Region;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FText Country;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float TerrainHeightMeters = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float SpawnAglMeters = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float PrevailingWindHeadingDeg = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float WindSpeedKmh = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float BaseRidgeLiftMetersPerSecond = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	float RouteLengthKm = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FVector2D LaunchPositionMeters = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FParaglideRidgeBand Ridge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FParaglideLandingZone LandingZone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	TArray<FParaglideThermalSource> Thermals;
};

USTRUCT(BlueprintType)
struct FParaglideScenarioPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FName Id = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FText Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FText Setup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FText RecommendedInputs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	TArray<FString> KeyOutputs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FParaglideFlightSite Site;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FParaglideAmbientAirState Atmosphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paraglide")
	FParaglideFlightState InitialFlightState;
};
