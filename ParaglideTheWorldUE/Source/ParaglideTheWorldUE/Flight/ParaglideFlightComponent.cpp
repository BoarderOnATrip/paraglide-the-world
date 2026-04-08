// Copyright Epic Games, Inc. All Rights Reserved.

#include "Flight/ParaglideFlightComponent.h"

#include "DrawDebugHelpers.h"
#include "Flight/ParaglideFlightTuning.h"
#include "Flight/ParaglideScenarioLibrary.h"
#include "GameFramework/Actor.h"

namespace
{
	struct FDerivedControls
	{
		float LeftBrakeTravel = 0.0f;
		float RightBrakeTravel = 0.0f;
		float SymmetricBrake = 0.0f;
		float BrakeDifferential = 0.0f;
		float WeightShift = 0.0f;
		float SpeedBarTravel = 0.0f;
	};

	struct FLandingZoneMetrics
	{
		bool bValid = false;
		float DistanceMeters = 0.0f;
		float ApproachErrorDeg = 0.0f;
		bool bIsInside = false;
	};

	float ApproachValue(const float Current, const float Target, const float MaxStep)
	{
		if (Current < Target)
		{
			return FMath::Min(Current + MaxStep, Target);
		}

		return FMath::Max(Current - MaxStep, Target);
	}

	float WrapDegrees(const float Value)
	{
		return FRotator::ClampAxis(Value);
	}

	float ProjectOffsetMeters(const FVector2D& OffsetMeters, const float HeadingDeg)
	{
		const float HeadingRadians = FMath::DegreesToRadians(HeadingDeg);
		return OffsetMeters.X * FMath::Sin(HeadingRadians) + OffsetMeters.Y * FMath::Cos(HeadingRadians);
	}

	FVector2D HeadingVectorMeters(const float HeadingDeg)
	{
		const float HeadingRadians = FMath::DegreesToRadians(HeadingDeg);
		return FVector2D(FMath::Sin(HeadingRadians), FMath::Cos(HeadingRadians));
	}

	FVector2D OffsetByHeading(const FVector2D& OriginMeters, const float HeadingDeg, const float DistanceMeters)
	{
		return OriginMeters + HeadingVectorMeters(HeadingDeg) * DistanceMeters;
	}

	FVector ToWorldLocation(const FVector2D& PositionMeters, const float HeightMeters)
	{
		return FVector(
			PositionMeters.Y * 100.0f,
			PositionMeters.X * 100.0f,
			HeightMeters * 100.0f);
	}

	float GetWindwardNormalHeadingDeg(const FParaglideRidgeBand& Ridge, const float WindHeadingDeg)
	{
		const float NormalA = WrapDegrees(Ridge.AxisHeadingDeg + 90.0f);
		const float NormalB = WrapDegrees(Ridge.AxisHeadingDeg - 90.0f);
		const float UpwindHeadingDeg = WrapDegrees(WindHeadingDeg + 180.0f);

		return FMath::Abs(FMath::FindDeltaAngleDegrees(NormalA, UpwindHeadingDeg)) <=
			FMath::Abs(FMath::FindDeltaAngleDegrees(NormalB, UpwindHeadingDeg))
			? NormalA
			: NormalB;
	}

	float GetTargetAirspeedKmh(const FDerivedControls& Controls, const FParaglideFlightAssistProfile& Assist, const FParaglideFlightTuning& Tuning)
	{
		const float TargetAirspeed =
			Tuning.BaselineWing.TrimAirspeedKmh +
			Controls.SpeedBarTravel * Tuning.Controls.SpeedBarAirspeedGainKmh -
			Controls.SymmetricBrake * Tuning.Controls.SymmetricBrakeAirspeedLossKmh -
			FMath::Square(Controls.SymmetricBrake) * Tuning.Controls.DeepBrakeAirspeedLossKmh +
			(Assist.InputResponsiveness - 0.9f) * Tuning.Controls.ResponsivenessAirspeedGainKmh;

		return FMath::Clamp(
			TargetAirspeed,
			Tuning.BaselineWing.MinControllableAirspeedKmh,
			Tuning.BaselineWing.MaxAirspeedKmh);
	}

	float GetTurnRateDegPerSecond(const float BankDeg, const float AirspeedKmh, const FParaglideFlightTuning& Tuning)
	{
		const float AirspeedMetersPerSecond = FMath::Max(
			AirspeedKmh / 3.6f,
			Tuning.BaselineWing.TurnRateMinAirspeedMetersPerSecond);
		const float BankRadians = FMath::DegreesToRadians(BankDeg);

		return FMath::RadiansToDegrees((9.81f * FMath::Tan(BankRadians)) / AirspeedMetersPerSecond);
	}

	EParaglideFlightPhase GetFlightPhase(
		const float ElapsedSeconds,
		const float GroundClearanceMeters,
		const float FlareEffectiveness,
		const FParaglideFlightTuning& Tuning)
	{
		EParaglideFlightPhase FlightPhase =
			ElapsedSeconds < Tuning.PhaseModel.LaunchDurationSeconds
			? EParaglideFlightPhase::Launch
			: EParaglideFlightPhase::Soaring;

		if (GroundClearanceMeters < Tuning.PhaseModel.ApproachHeightMeters)
		{
			FlightPhase = EParaglideFlightPhase::Approach;
		}

		if (FlareEffectiveness > Tuning.PhaseModel.FlareEffectivenessThreshold &&
			GroundClearanceMeters < Tuning.PhaseModel.FlarePhaseHeightMeters)
		{
			FlightPhase = EParaglideFlightPhase::Flare;
		}

		return FlightPhase;
	}

	FDerivedControls DeriveControls(const FParaglideControlState& Controls)
	{
		FDerivedControls Derived;
		Derived.LeftBrakeTravel = Controls.LeftBrakeTravel;
		Derived.RightBrakeTravel = Controls.RightBrakeTravel;
		Derived.SymmetricBrake = (Controls.LeftBrakeTravel + Controls.RightBrakeTravel) * 0.5f;
		Derived.BrakeDifferential = Controls.RightBrakeTravel - Controls.LeftBrakeTravel;
		Derived.WeightShift = Controls.WeightShiftPosition;
		Derived.SpeedBarTravel = Controls.SpeedBarTravel;
		return Derived;
	}

	void StepControls(FParaglideControlState& Controls, const float DeltaSeconds)
	{
		const float LeftBrakeTarget = Controls.bLeftBrakePressed ? 1.0f : 0.0f;
		const float RightBrakeTarget = Controls.bRightBrakePressed ? 1.0f : 0.0f;
		const float WeightTarget = (Controls.bWeightRightPressed ? 1.0f : 0.0f) - (Controls.bWeightLeftPressed ? 1.0f : 0.0f);
		const float SpeedBarTarget = Controls.bSpeedBarPressed ? 1.0f : 0.0f;
		const float BrakeStep = DeltaSeconds * 2.8f;
		const float BrakeReleaseStep = DeltaSeconds * 4.8f;
		const float WeightStep = DeltaSeconds * 4.2f;
		const float WeightReleaseStep = DeltaSeconds * 6.0f;
		const float SpeedBarStep = DeltaSeconds * 2.0f;
		const float SpeedBarReleaseStep = DeltaSeconds * 3.0f;

		Controls.LeftBrakeTravel = FMath::Clamp(
			ApproachValue(Controls.LeftBrakeTravel, LeftBrakeTarget, Controls.bLeftBrakePressed ? BrakeStep : BrakeReleaseStep),
			0.0f,
			1.0f);
		Controls.RightBrakeTravel = FMath::Clamp(
			ApproachValue(Controls.RightBrakeTravel, RightBrakeTarget, Controls.bRightBrakePressed ? BrakeStep : BrakeReleaseStep),
			0.0f,
			1.0f);
		Controls.WeightShiftPosition = FMath::Clamp(
			ApproachValue(Controls.WeightShiftPosition, WeightTarget, WeightTarget == 0.0f ? WeightReleaseStep : WeightStep),
			-1.0f,
			1.0f);
		Controls.SpeedBarTravel = FMath::Clamp(
			ApproachValue(Controls.SpeedBarTravel, SpeedBarTarget, Controls.bSpeedBarPressed ? SpeedBarStep : SpeedBarReleaseStep),
			0.0f,
			1.0f);
	}

	void GetRidgeAirMass(
		const FVector2D& PositionMeters,
		const float WindHeadingDeg,
		const float WindSpeedKmh,
		const FParaglideFlightSite& Site,
		const FParaglideFlightTuning& Tuning,
		float& OutRidgeLiftMetersPerSecond,
		float& OutLeeSinkMetersPerSecond)
	{
		OutRidgeLiftMetersPerSecond = 0.0f;
		OutLeeSinkMetersPerSecond = 0.0f;

		if (!Site.Ridge.bEnabled)
		{
			const float DistanceFromLaunchMeters = FVector2D::Distance(PositionMeters, Site.LaunchPositionMeters);
			const float RidgeAlignment = FMath::Cos(FMath::DegreesToRadians(WindHeadingDeg - Site.PrevailingWindHeadingDeg));
			const float TerrainWindow = FMath::Clamp(
				1.0f - DistanceFromLaunchMeters / Tuning.LiftSources.FallbackRidgeTerrainWindowMeters,
				0.0f,
				1.0f);
			const float WindFactor = FMath::Clamp(
				(WindSpeedKmh - Tuning.LiftSources.FallbackRidgeWindStartKmh) / Tuning.LiftSources.FallbackRidgeWindRangeKmh,
				0.0f,
				Tuning.LiftSources.FallbackRidgeWindCap);

			OutRidgeLiftMetersPerSecond = FMath::Max(0.0f, RidgeAlignment) * Site.BaseRidgeLiftMetersPerSecond * TerrainWindow * WindFactor;
			return;
		}

		const float WindFactor = FMath::Clamp(
			(WindSpeedKmh - Tuning.LiftSources.RidgeWindStartKmh) / Tuning.LiftSources.RidgeWindRangeKmh,
			0.0f,
			Tuning.LiftSources.RidgeWindCap);
		const FVector2D OffsetMeters = PositionMeters - Site.Ridge.PositionMeters;
		const float AlongMeters = ProjectOffsetMeters(OffsetMeters, Site.Ridge.AxisHeadingDeg);
		const float WindwardNormalHeadingDeg = GetWindwardNormalHeadingDeg(Site.Ridge, WindHeadingDeg);
		const float CrossWindwardMeters = ProjectOffsetMeters(OffsetMeters, WindwardNormalHeadingDeg);
		const float AlongFactor = FMath::Clamp(
			1.0f - FMath::Abs(AlongMeters) / (Site.Ridge.LengthMeters * 0.5f),
			0.0f,
			1.0f);

		if (AlongFactor <= 0.0f)
		{
			return;
		}

		if (CrossWindwardMeters >= 0.0f)
		{
			const float DepthFactor = FMath::Clamp(
				1.0f - CrossWindwardMeters / Site.Ridge.WindwardDepthMeters,
				0.0f,
				1.0f);
			OutRidgeLiftMetersPerSecond = Site.Ridge.PeakLiftMetersPerSecond * AlongFactor * DepthFactor * WindFactor;
			return;
		}

		const float LeeFactor = FMath::Clamp(
			1.0f - FMath::Abs(CrossWindwardMeters) / Site.Ridge.LeeDepthMeters,
			0.0f,
			1.0f);
		OutLeeSinkMetersPerSecond = Site.Ridge.LeeSinkMetersPerSecond * AlongFactor * LeeFactor * WindFactor;
	}

	FVector2D GetDriftedThermalCenter(const FParaglideThermalSource& Thermal, const FParaglideAmbientAirState& Atmosphere, const float ElapsedSeconds)
	{
		const float DriftMeters = (Atmosphere.WindSpeedKmh / 3.6f) * ElapsedSeconds * Thermal.DriftFactor;
		const float HeadingRadians = FMath::DegreesToRadians(Atmosphere.WindHeadingDeg);
		return FVector2D(
			Thermal.PositionMeters.X + FMath::Sin(HeadingRadians) * DriftMeters,
			Thermal.PositionMeters.Y + FMath::Cos(HeadingRadians) * DriftMeters);
	}

	void GetThermalAirMass(
		const FVector2D& PositionMeters,
		const float ElapsedSeconds,
		const FParaglideAmbientAirState& Atmosphere,
		const FParaglideFlightSite& Site,
		const FParaglideFlightTuning& Tuning,
		float& OutLiftMetersPerSecond,
		float& OutSinkMetersPerSecond)
	{
		OutLiftMetersPerSecond = 0.0f;
		OutSinkMetersPerSecond = 0.0f;

		for (const FParaglideThermalSource& Thermal : Site.Thermals)
		{
			const FVector2D ThermalCenter = GetDriftedThermalCenter(Thermal, Atmosphere, ElapsedSeconds);
			const float DistanceMeters = FVector2D::Distance(PositionMeters, ThermalCenter);

			if (DistanceMeters <= Thermal.CoreRadiusMeters)
			{
				const float CoreFactor = FMath::Clamp(1.0f - DistanceMeters / Thermal.CoreRadiusMeters, 0.0f, 1.0f);
				OutLiftMetersPerSecond += Thermal.LiftMetersPerSecond * (Tuning.LiftSources.ThermalCoreBaseFraction + CoreFactor * Tuning.LiftSources.ThermalCoreCenteringBonus);
				continue;
			}

			if (DistanceMeters <= Thermal.RadiusMeters)
			{
				const float EdgeFactor = FMath::Clamp(
					1.0f - (DistanceMeters - Thermal.CoreRadiusMeters) / FMath::Max(Thermal.RadiusMeters - Thermal.CoreRadiusMeters, 1.0f),
					0.0f,
					1.0f);
				OutLiftMetersPerSecond += Thermal.LiftMetersPerSecond * EdgeFactor * EdgeFactor * Tuning.LiftSources.ThermalEdgeLiftFraction;
				continue;
			}

			const float SinkRingRadiusMeters = Thermal.RadiusMeters * Tuning.LiftSources.ThermalSinkRingRadiusMultiplier;
			if (Thermal.SinkRingMetersPerSecond <= 0.0f || DistanceMeters > SinkRingRadiusMeters)
			{
				continue;
			}

			const float SinkFactor = FMath::Clamp(
				1.0f - (DistanceMeters - Thermal.RadiusMeters) / FMath::Max(SinkRingRadiusMeters - Thermal.RadiusMeters, 1.0f),
				0.0f,
				1.0f);
			OutSinkMetersPerSecond += Thermal.SinkRingMetersPerSecond * SinkFactor;
		}
	}

	float GetTurbulenceVerticalGust(
		const FVector2D& PositionMeters,
		const float ElapsedSeconds,
		const float Turbulence,
		const FParaglideFlightTuning& Tuning)
	{
		const float Phase = ElapsedSeconds * 1.55f + PositionMeters.X * 0.034f + PositionMeters.Y * 0.041f;
		const float SecondaryPhase = ElapsedSeconds * 2.7f + PositionMeters.X * 0.012f - PositionMeters.Y * 0.019f;
		return (FMath::Sin(Phase) * Tuning.LiftSources.TurbulencePrimaryMix +
			FMath::Sin(SecondaryPhase) * Tuning.LiftSources.TurbulenceSecondaryMix) *
			Turbulence * Tuning.LiftSources.TurbulenceVerticalGustScale;
	}

	FLandingZoneMetrics GetLandingZoneMetrics(
		const FVector2D& PositionMeters,
		const float HeadingDeg,
		const FParaglideLandingZone& LandingZone)
	{
		FLandingZoneMetrics Metrics;

		if (!LandingZone.bEnabled)
		{
			return Metrics;
		}

		const FVector2D OffsetMeters = PositionMeters - LandingZone.PositionMeters;
		const float AlongMeters = ProjectOffsetMeters(OffsetMeters, LandingZone.HeadingDeg);
		const float CrossMeters = ProjectOffsetMeters(OffsetMeters, LandingZone.HeadingDeg + 90.0f);
		const float OutsideAlongMeters = FMath::Max(0.0f, FMath::Abs(AlongMeters) - LandingZone.LengthMeters * 0.5f);
		const float OutsideCrossMeters = FMath::Max(0.0f, FMath::Abs(CrossMeters) - LandingZone.WidthMeters * 0.5f);

		Metrics.bValid = true;
		Metrics.DistanceMeters = FMath::Sqrt(FMath::Square(OutsideAlongMeters) + FMath::Square(OutsideCrossMeters));
		Metrics.ApproachErrorDeg = FMath::Abs(FMath::FindDeltaAngleDegrees(HeadingDeg, LandingZone.HeadingDeg));
		Metrics.bIsInside = FMath::Abs(AlongMeters) <= LandingZone.LengthMeters * 0.5f && FMath::Abs(CrossMeters) <= LandingZone.WidthMeters * 0.5f;
		return Metrics;
	}

	EParaglideLandingRating GetLandingRating(
		const float VerticalSpeedMetersPerSecond,
		const float GroundSpeedKmh,
		const float BankDeg,
		const FLandingZoneMetrics& LandingMetrics,
		const FParaglideFlightTuning& Tuning)
	{
		const float SeverityScore =
			FMath::Abs(VerticalSpeedMetersPerSecond) * Tuning.Landing.VerticalSpeedSeverityMultiplier +
			FMath::Max(0.0f, GroundSpeedKmh - Tuning.Landing.GroundSpeedSeverityOffsetKmh) / Tuning.Landing.GroundSpeedSeverityDivisor +
			FMath::Abs(BankDeg) / Tuning.Landing.BankSeverityDivisorDeg +
			(LandingMetrics.bValid ? LandingMetrics.ApproachErrorDeg / Tuning.Landing.ApproachErrorSeverityDivisorDeg : 0.0f) +
			(LandingMetrics.bValid ? LandingMetrics.DistanceMeters / Tuning.Landing.LandingDistanceSeverityDivisorMeters : 0.0f);

		if (SeverityScore < Tuning.Landing.SmoothSeverityThreshold)
		{
			return EParaglideLandingRating::Smooth;
		}

		if (SeverityScore < Tuning.Landing.FirmSeverityThreshold)
		{
			return EParaglideLandingRating::Firm;
		}

		if (SeverityScore < Tuning.Landing.HardSeverityThreshold)
		{
			return EParaglideLandingRating::Hard;
		}

		return EParaglideLandingRating::Crash;
	}
}

UParaglideFlightComponent::UParaglideFlightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	ScenarioPresets = FParaglideScenarioLibrary::GetScenarioPresets();
	ResetFromScenario();
}

void UParaglideFlightComponent::BeginPlay()
{
	Super::BeginPlay();
	ApplyOwnerTransform();
}

void UParaglideFlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ScenarioPresets.IsEmpty())
	{
		return;
	}

	const float ClampedDeltaSeconds = FMath::Clamp(DeltaTime, 0.0f, 0.05f);
	StepControls(Controls, ClampedDeltaSeconds);
	FlightState = StepFlightState(FlightState, ClampedDeltaSeconds);
	ApplyOwnerTransform();
	DrawScenarioDebug();
}

void UParaglideFlightComponent::SetWeightLeftPressed(const bool bPressed)
{
	Controls.bWeightLeftPressed = bPressed;
}

void UParaglideFlightComponent::SetLeftBrakePressed(const bool bPressed)
{
	Controls.bLeftBrakePressed = bPressed;
}

void UParaglideFlightComponent::SetRightBrakePressed(const bool bPressed)
{
	Controls.bRightBrakePressed = bPressed;
}

void UParaglideFlightComponent::SetWeightRightPressed(const bool bPressed)
{
	Controls.bWeightRightPressed = bPressed;
}

void UParaglideFlightComponent::SetSpeedBarPressed(const bool bPressed)
{
	Controls.bSpeedBarPressed = bPressed;
}

void UParaglideFlightComponent::ResetScenario()
{
	ResetFromScenario();
	ApplyOwnerTransform();
}

void UParaglideFlightComponent::SelectScenarioByIndex(const int32 Index)
{
	if (!ScenarioPresets.IsValidIndex(Index))
	{
		return;
	}

	CurrentScenarioIndex = Index;
	ResetScenario();
}

void UParaglideFlightComponent::StepScenarioForward()
{
	if (ScenarioPresets.IsEmpty())
	{
		return;
	}

	CurrentScenarioIndex = (CurrentScenarioIndex + 1) % ScenarioPresets.Num();
	ResetScenario();
}

void UParaglideFlightComponent::StepScenarioBackward()
{
	if (ScenarioPresets.IsEmpty())
	{
		return;
	}

	CurrentScenarioIndex = (CurrentScenarioIndex - 1 + ScenarioPresets.Num()) % ScenarioPresets.Num();
	ResetScenario();
}

const FParaglideFlightState& UParaglideFlightComponent::GetFlightState() const
{
	return FlightState;
}

const FParaglideControlState& UParaglideFlightComponent::GetControlState() const
{
	return Controls;
}

const FParaglideFlightAssistProfile& UParaglideFlightComponent::GetAssistProfile() const
{
	return AssistProfile;
}

const FParaglideScenarioPreset& UParaglideFlightComponent::GetCurrentScenario() const
{
	return ScenarioPresets[CurrentScenarioIndex];
}

const TArray<FParaglideScenarioPreset>& UParaglideFlightComponent::GetScenarioPresets() const
{
	return ScenarioPresets;
}

FText UParaglideFlightComponent::GetCurrentScenarioName() const
{
	return GetCurrentScenario().Name;
}

FText UParaglideFlightComponent::GetCurrentScenarioSummary() const
{
	return GetCurrentScenario().Summary;
}

FText UParaglideFlightComponent::GetCurrentScenarioRecommendedInputs() const
{
	return GetCurrentScenario().RecommendedInputs;
}

int32 UParaglideFlightComponent::GetCurrentScenarioIndex() const
{
	return CurrentScenarioIndex;
}

int32 UParaglideFlightComponent::GetScenarioCount() const
{
	return ScenarioPresets.Num();
}

void UParaglideFlightComponent::ResetFromScenario()
{
	if (!ScenarioPresets.IsValidIndex(CurrentScenarioIndex))
	{
		CurrentScenarioIndex = 0;
	}

	Controls = FParaglideControlState();
	AssistProfile = FParaglideFlightAssistProfile();
	Atmosphere = GetCurrentScenario().Atmosphere;
	FlightState = GetCurrentScenario().InitialFlightState;
}

void UParaglideFlightComponent::ApplyOwnerTransform() const
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	const FVector WorldLocation(
		FlightState.PositionMeters.Y * 100.0f,
		FlightState.PositionMeters.X * 100.0f,
		FMath::Max(FlightState.GroundClearanceMeters, 0.0f) * 100.0f);
	const FRotator WorldRotation(FlightState.PitchDeg, FlightState.HeadingDeg, FlightState.BankDeg);
	Owner->SetActorLocationAndRotation(WorldLocation, WorldRotation);
}

void UParaglideFlightComponent::DrawScenarioDebug() const
{
	UWorld* World = GetWorld();
	if (World == nullptr || ScenarioPresets.IsEmpty())
	{
		return;
	}

	const FParaglideFlightSite& Site = GetCurrentScenario().Site;
	const float DebugLifeTime = 0.0f;
	const float GroundHeightMeters = 0.5f;

	DrawDebugSphere(
		World,
		ToWorldLocation(Site.LaunchPositionMeters, GroundHeightMeters),
		180.0f,
		12,
		FColor(70, 220, 140),
		false,
		DebugLifeTime,
		0,
		4.0f);
	DrawDebugDirectionalArrow(
		World,
		ToWorldLocation(Site.LaunchPositionMeters, GroundHeightMeters + 1.5f),
		ToWorldLocation(OffsetByHeading(Site.LaunchPositionMeters, Site.PrevailingWindHeadingDeg, 30.0f), GroundHeightMeters + 1.5f),
		260.0f,
		FColor(80, 190, 255),
		false,
		DebugLifeTime,
		0,
		5.0f);

	if (Site.Ridge.bEnabled)
	{
		const FVector2D AxisVector = HeadingVectorMeters(Site.Ridge.AxisHeadingDeg);
		const FVector2D StartMeters = Site.Ridge.PositionMeters - AxisVector * (Site.Ridge.LengthMeters * 0.5f);
		const FVector2D EndMeters = Site.Ridge.PositionMeters + AxisVector * (Site.Ridge.LengthMeters * 0.5f);
		const float WindwardHeadingDeg = GetWindwardNormalHeadingDeg(Site.Ridge, Atmosphere.WindHeadingDeg);
		const FVector2D WindwardOffset = HeadingVectorMeters(WindwardHeadingDeg) * Site.Ridge.WindwardDepthMeters;
		const FVector2D LeeOffset = HeadingVectorMeters(WindwardHeadingDeg) * -Site.Ridge.LeeDepthMeters;

		DrawDebugLine(
			World,
			ToWorldLocation(StartMeters, GroundHeightMeters),
			ToWorldLocation(EndMeters, GroundHeightMeters),
			FColor(255, 180, 40),
			false,
			DebugLifeTime,
			0,
			6.0f);
		DrawDebugLine(
			World,
			ToWorldLocation(StartMeters + WindwardOffset, GroundHeightMeters),
			ToWorldLocation(EndMeters + WindwardOffset, GroundHeightMeters),
			FColor(80, 220, 120),
			false,
			DebugLifeTime,
			0,
			3.0f);
		DrawDebugLine(
			World,
			ToWorldLocation(StartMeters + LeeOffset, GroundHeightMeters),
			ToWorldLocation(EndMeters + LeeOffset, GroundHeightMeters),
			FColor(230, 90, 90),
			false,
			DebugLifeTime,
			0,
			3.0f);
	}

	if (Site.LandingZone.bEnabled)
	{
		const FVector2D ForwardVector = HeadingVectorMeters(Site.LandingZone.HeadingDeg);
		const FVector2D RightVector = HeadingVectorMeters(Site.LandingZone.HeadingDeg + 90.0f);
		const FVector2D HalfForward = ForwardVector * (Site.LandingZone.LengthMeters * 0.5f);
		const FVector2D HalfRight = RightVector * (Site.LandingZone.WidthMeters * 0.5f);
		const FVector2D CornerA = Site.LandingZone.PositionMeters + HalfForward + HalfRight;
		const FVector2D CornerB = Site.LandingZone.PositionMeters + HalfForward - HalfRight;
		const FVector2D CornerC = Site.LandingZone.PositionMeters - HalfForward - HalfRight;
		const FVector2D CornerD = Site.LandingZone.PositionMeters - HalfForward + HalfRight;

		DrawDebugLine(World, ToWorldLocation(CornerA, GroundHeightMeters), ToWorldLocation(CornerB, GroundHeightMeters), FColor(240, 240, 90), false, DebugLifeTime, 0, 4.0f);
		DrawDebugLine(World, ToWorldLocation(CornerB, GroundHeightMeters), ToWorldLocation(CornerC, GroundHeightMeters), FColor(240, 240, 90), false, DebugLifeTime, 0, 4.0f);
		DrawDebugLine(World, ToWorldLocation(CornerC, GroundHeightMeters), ToWorldLocation(CornerD, GroundHeightMeters), FColor(240, 240, 90), false, DebugLifeTime, 0, 4.0f);
		DrawDebugLine(World, ToWorldLocation(CornerD, GroundHeightMeters), ToWorldLocation(CornerA, GroundHeightMeters), FColor(240, 240, 90), false, DebugLifeTime, 0, 4.0f);
	}

	for (const FParaglideThermalSource& Thermal : Site.Thermals)
	{
		const FVector2D ThermalCenterMeters = GetDriftedThermalCenter(Thermal, Atmosphere, FlightState.ElapsedSeconds);
		const FVector ThermalBase = ToWorldLocation(ThermalCenterMeters, 0.0f);
		const FVector ThermalTop = ThermalBase + FVector(0.0f, 0.0f, 32000.0f);

		DrawDebugCylinder(
			World,
			ThermalBase,
			ThermalTop,
			Thermal.CoreRadiusMeters * 100.0f,
			20,
			FColor(255, 180, 60),
			false,
			DebugLifeTime,
			0,
			2.5f);
		DrawDebugCylinder(
			World,
			ThermalBase,
			ThermalTop,
			Thermal.RadiusMeters * 100.0f,
			28,
			FColor(255, 110, 40),
			false,
			DebugLifeTime,
			0,
			1.5f);
	}
}

FParaglideFlightState UParaglideFlightComponent::StepFlightState(const FParaglideFlightState& CurrentState, const float DeltaSeconds) const
{
	const FParaglideFlightTuning& Tuning = FParaglideFlightTuning::Get();
	const FParaglideFlightSite& Site = GetCurrentScenario().Site;
	const FDerivedControls DerivedControls = DeriveControls(Controls);

	if (CurrentState.FlightPhase == EParaglideFlightPhase::Landed || CurrentState.FlightPhase == EParaglideFlightPhase::Crashed)
	{
		FParaglideFlightState NextState = CurrentState;
		NextState.AltitudeMeters = CurrentState.TerrainHeightMeters;
		NextState.AirspeedKmh = 0.0f;
		NextState.GroundSpeedKmh = 0.0f;
		NextState.VerticalSpeedMetersPerSecond = 0.0f;
		NextState.GroundClearanceMeters = 0.0f;
		NextState.BankDeg = ApproachValue(CurrentState.BankDeg, 0.0f, DeltaSeconds * 18.0f);
		NextState.PitchDeg = ApproachValue(CurrentState.PitchDeg, 0.0f, DeltaSeconds * 12.0f);
		NextState.TurnRateDegPerSecond = 0.0f;
		NextState.ElapsedSeconds = CurrentState.ElapsedSeconds + DeltaSeconds;
		NextState.Debug.TurbulenceLiftMetersPerSecond = 0.0f;
		NextState.Debug.FlareLiftMetersPerSecond = 0.0f;
		return NextState;
	}

	const float EffectiveTurbulence = FMath::Clamp(Atmosphere.Turbulence * (1.0f - AssistProfile.TurbulenceDamping), 0.0f, 1.0f);
	float AirspeedKmh = ApproachValue(
		CurrentState.AirspeedKmh,
		GetTargetAirspeedKmh(DerivedControls, AssistProfile, Tuning),
		DeltaSeconds * Tuning.Controls.AirspeedResponseRate * AssistProfile.InputResponsiveness);
	const float StallWarning = FMath::Clamp(
		(DerivedControls.SymmetricBrake - Tuning.Stability.StallBrakeStart) / Tuning.Stability.StallBrakeRange +
		(Tuning.Stability.StallAirspeedReferenceKmh - AirspeedKmh) / Tuning.Stability.StallAirspeedRangeKmh -
		DerivedControls.SpeedBarTravel * Tuning.Stability.SpeedBarStallRelief -
		AssistProfile.RecoveryAssist * Tuning.Stability.RecoveryAssistStallRelief,
		0.0f,
		1.0f);
	const float ControlAuthority = FMath::Clamp(
		AssistProfile.InputResponsiveness +
		AssistProfile.CoordinationAssist * Tuning.Stability.CoordinationAuthorityGain -
		StallWarning * Tuning.Stability.StallAuthorityPenalty,
		Tuning.Stability.ControlAuthorityFloor,
		Tuning.Stability.ControlAuthorityCeiling);
	const float GustBankDeg =
		FMath::Sin(CurrentState.ElapsedSeconds * 1.9f + CurrentState.HeadingDeg / 23.0f) *
		EffectiveTurbulence * Tuning.Stability.GustRollMagnitudeDeg;
	const float TargetBankDeg = FMath::Clamp(
		(DerivedControls.BrakeDifferential * Tuning.Controls.BrakeToRollDeg +
			DerivedControls.WeightShift * Tuning.Controls.WeightShiftAuthorityDeg) * ControlAuthority +
		GustBankDeg,
		-Tuning.Stability.BankLimitDeg,
		Tuning.Stability.BankLimitDeg);
	const float BankDeg = ApproachValue(
		CurrentState.BankDeg,
		TargetBankDeg,
		DeltaSeconds * (Tuning.Controls.BankResponseRate + AssistProfile.CoordinationAssist * Tuning.Controls.CoordinationBankResponseGain));
	const float TurnRateDegPerSecond = GetTurnRateDegPerSecond(BankDeg, AirspeedKmh, Tuning) * (1.0f - StallWarning * Tuning.Stability.StallTurnRatePenalty);
	const float HeadingDeg = WrapDegrees(CurrentState.HeadingDeg + TurnRateDegPerSecond * DeltaSeconds);

	float RidgeLiftMetersPerSecond = 0.0f;
	float LeeSinkMetersPerSecond = 0.0f;
	GetRidgeAirMass(CurrentState.PositionMeters, Atmosphere.WindHeadingDeg, Atmosphere.WindSpeedKmh, Site, Tuning, RidgeLiftMetersPerSecond, LeeSinkMetersPerSecond);

	float ThermalLiftMetersPerSecond = 0.0f;
	float ThermalSinkMetersPerSecond = 0.0f;
	GetThermalAirMass(CurrentState.PositionMeters, CurrentState.ElapsedSeconds, Atmosphere, Site, Tuning, ThermalLiftMetersPerSecond, ThermalSinkMetersPerSecond);

	const float TurbulenceLiftMetersPerSecond = GetTurbulenceVerticalGust(CurrentState.PositionMeters, CurrentState.ElapsedSeconds, EffectiveTurbulence, Tuning);
	const float SpeedDeviation = AirspeedKmh - Tuning.BaselineWing.TrimAirspeedKmh;
	const float BaseSinkMetersPerSecond = Tuning.BaselineWing.BaselineSinkMetersPerSecond + FMath::Square(SpeedDeviation) / Tuning.BaselineWing.SpeedToSinkCurveDivisor;
	const float BankRadians = FMath::DegreesToRadians(BankDeg);
	const float LoadFactor = 1.0f / FMath::Max(FMath::Cos(BankRadians), Tuning.Stability.LoadFactorCosFloor);
	const float InducedTurnSinkMetersPerSecond = FMath::Max(0.0f, LoadFactor - 1.0f) * Tuning.Stability.InducedTurnSinkMultiplier;
	const float BrakeSinkMetersPerSecond =
		DerivedControls.SymmetricBrake * Tuning.Controls.BrakeSinkLinearMetersPerSecond +
		FMath::Square(DerivedControls.SymmetricBrake) * Tuning.Controls.BrakeSinkQuadraticMetersPerSecond;
	const float StallSinkMetersPerSecond = FMath::Square(StallWarning) * Tuning.Stability.StallSinkMultiplier;
	const float LowAltitudeFactor = FMath::Clamp(
		(Tuning.PhaseModel.FlareArmingHeightMeters - CurrentState.GroundClearanceMeters) / Tuning.PhaseModel.FlareArmingHeightMeters,
		0.0f,
		1.0f);
	const float FlareCommand = FMath::Clamp(
		(DerivedControls.SymmetricBrake - Tuning.Landing.FlareBrakeStart) / Tuning.Landing.FlareBrakeRange,
		0.0f,
		1.0f);
	const float FlareEffectiveness = FlareCommand * LowAltitudeFactor * (1.0f - StallWarning * Tuning.Stability.FlareStallPenalty);
	const float FlareLiftMetersPerSecond =
		FlareEffectiveness *
		FMath::Clamp(
			(Tuning.Landing.FlareLiftHeightMeters - CurrentState.GroundClearanceMeters) / Tuning.Landing.FlareLiftHeightMeters,
			0.0f,
			1.0f) *
		Tuning.Landing.FlareLiftMultiplier;
	const float FlareDragKmh =
		FlareEffectiveness *
		FMath::Clamp(
			(Tuning.Landing.FlareDragHeightMeters - CurrentState.GroundClearanceMeters) / Tuning.Landing.FlareDragHeightMeters,
			0.0f,
			1.0f) *
		Tuning.Landing.FlareDragMultiplierKmh;

	AirspeedKmh = FMath::Clamp(AirspeedKmh - FlareDragKmh, Tuning.BaselineWing.FlareMinAirspeedKmh, Tuning.BaselineWing.MaxAirspeedKmh);

	const float AirMassSinkMetersPerSecond = LeeSinkMetersPerSecond + ThermalSinkMetersPerSecond;
	const float TotalSinkMetersPerSecond =
		BaseSinkMetersPerSecond +
		InducedTurnSinkMetersPerSecond +
		BrakeSinkMetersPerSecond +
		StallSinkMetersPerSecond +
		AirMassSinkMetersPerSecond;
	const float VerticalSpeedMetersPerSecond =
		ThermalLiftMetersPerSecond +
		RidgeLiftMetersPerSecond +
		TurbulenceLiftMetersPerSecond +
		FlareLiftMetersPerSecond -
		TotalSinkMetersPerSecond;
	const float TargetPitchDeg = FMath::Clamp(
		Tuning.Attitude.TrimPitchDeg +
		DerivedControls.SpeedBarTravel * Tuning.Attitude.SpeedBarPitchGainDeg -
		DerivedControls.SymmetricBrake * Tuning.Attitude.SymmetricBrakePitchLossDeg +
		StallWarning * Tuning.Attitude.StallPitchGainDeg +
		FlareEffectiveness * Tuning.Attitude.FlarePitchGainDeg -
		EffectiveTurbulence * Tuning.Attitude.TurbulencePitchGainDeg,
		Tuning.Attitude.MinPitchDeg,
		Tuning.Attitude.MaxPitchDeg);
	const float PitchDeg = ApproachValue(CurrentState.PitchDeg, TargetPitchDeg, DeltaSeconds * Tuning.Attitude.PitchResponseRate);

	const float HeadingRadians = FMath::DegreesToRadians(HeadingDeg);
	const float WindHeadingRadians = FMath::DegreesToRadians(Atmosphere.WindHeadingDeg);
	const float AirspeedMetersPerSecond = AirspeedKmh / 3.6f;
	const float WindMetersPerSecond = Atmosphere.WindSpeedKmh / 3.6f;
	const float EastMetersPerSecond = FMath::Sin(HeadingRadians) * AirspeedMetersPerSecond + FMath::Sin(WindHeadingRadians) * WindMetersPerSecond;
	const float NorthMetersPerSecond = FMath::Cos(HeadingRadians) * AirspeedMetersPerSecond + FMath::Cos(WindHeadingRadians) * WindMetersPerSecond;
	const float GroundSpeedKmh = FVector2D::Distance(FVector2D::ZeroVector, FVector2D(EastMetersPerSecond, NorthMetersPerSecond)) * 3.6f;
	const FVector2D NextPositionMeters = CurrentState.PositionMeters + FVector2D(EastMetersPerSecond, NorthMetersPerSecond) * DeltaSeconds;
	const float UnclampedAltitudeMeters = CurrentState.AltitudeMeters + VerticalSpeedMetersPerSecond * DeltaSeconds;
	const FLandingZoneMetrics LandingMetrics = GetLandingZoneMetrics(NextPositionMeters, HeadingDeg, Site.LandingZone);

	if (UnclampedAltitudeMeters <= CurrentState.TerrainHeightMeters + Tuning.Landing.TouchdownHeightMarginMeters)
	{
		const float TouchdownVerticalSpeedMetersPerSecond =
			VerticalSpeedMetersPerSecond *
			(1.0f - FMath::Clamp(FlareEffectiveness * Tuning.Landing.TouchdownVerticalRelief, 0.0f, Tuning.Landing.TouchdownVerticalRelief));
		const float TouchdownGroundSpeedKmh =
			GroundSpeedKmh *
			(1.0f - FMath::Clamp(FlareEffectiveness * Tuning.Landing.TouchdownGroundRelief, 0.0f, Tuning.Landing.TouchdownGroundRelief));
		const EParaglideLandingRating LandingRating = GetLandingRating(TouchdownVerticalSpeedMetersPerSecond, TouchdownGroundSpeedKmh, BankDeg, LandingMetrics, Tuning);

		FParaglideFlightState NextState;
		NextState.PositionMeters = NextPositionMeters;
		NextState.AltitudeMeters = CurrentState.TerrainHeightMeters;
		NextState.TerrainHeightMeters = CurrentState.TerrainHeightMeters;
		NextState.HeadingDeg = HeadingDeg;
		NextState.BankDeg = (LandingRating == EParaglideLandingRating::Smooth || LandingRating == EParaglideLandingRating::Firm) ? 0.0f : BankDeg;
		NextState.PitchDeg = LandingRating == EParaglideLandingRating::Smooth ? 0.0f : PitchDeg;
		NextState.AirspeedKmh = 0.0f;
		NextState.GroundSpeedKmh = 0.0f;
		NextState.VerticalSpeedMetersPerSecond = 0.0f;
		NextState.RidgeLiftMetersPerSecond = RidgeLiftMetersPerSecond;
		NextState.ThermalLiftMetersPerSecond = ThermalLiftMetersPerSecond;
		NextState.AirMassSinkMetersPerSecond = AirMassSinkMetersPerSecond;
		NextState.GroundClearanceMeters = 0.0f;
		NextState.DistanceKm = CurrentState.DistanceKm + (GroundSpeedKmh * DeltaSeconds) / 3600.0f;
		NextState.ElapsedSeconds = CurrentState.ElapsedSeconds + DeltaSeconds;
		NextState.TurnRateDegPerSecond = 0.0f;
		NextState.StallWarning = StallWarning;
		NextState.FlareEffectiveness = FlareEffectiveness;
		NextState.LandingZoneDistanceMeters = LandingMetrics.bValid ? LandingMetrics.DistanceMeters : -1.0f;
		NextState.LandingApproachErrorDeg = LandingMetrics.bValid ? LandingMetrics.ApproachErrorDeg : -1.0f;
		NextState.LandingRating = LandingRating;
		NextState.FlightPhase = LandingRating == EParaglideLandingRating::Crash ? EParaglideFlightPhase::Crashed : EParaglideFlightPhase::Landed;
		NextState.Debug.BaseSinkMetersPerSecond = BaseSinkMetersPerSecond;
		NextState.Debug.InducedTurnSinkMetersPerSecond = InducedTurnSinkMetersPerSecond;
		NextState.Debug.BrakeSinkMetersPerSecond = BrakeSinkMetersPerSecond;
		NextState.Debug.StallSinkMetersPerSecond = StallSinkMetersPerSecond;
		NextState.Debug.TurbulenceLiftMetersPerSecond = TurbulenceLiftMetersPerSecond;
		NextState.Debug.FlareLiftMetersPerSecond = FlareLiftMetersPerSecond;
		NextState.Debug.TotalSinkMetersPerSecond = TotalSinkMetersPerSecond;
		return NextState;
	}

	FParaglideFlightState NextState;
	NextState.PositionMeters = NextPositionMeters;
	NextState.AltitudeMeters = UnclampedAltitudeMeters;
	NextState.TerrainHeightMeters = CurrentState.TerrainHeightMeters;
	NextState.HeadingDeg = HeadingDeg;
	NextState.BankDeg = BankDeg;
	NextState.PitchDeg = PitchDeg;
	NextState.AirspeedKmh = AirspeedKmh;
	NextState.GroundSpeedKmh = GroundSpeedKmh;
	NextState.VerticalSpeedMetersPerSecond = VerticalSpeedMetersPerSecond;
	NextState.RidgeLiftMetersPerSecond = RidgeLiftMetersPerSecond;
	NextState.ThermalLiftMetersPerSecond = ThermalLiftMetersPerSecond;
	NextState.AirMassSinkMetersPerSecond = AirMassSinkMetersPerSecond;
	NextState.GroundClearanceMeters = UnclampedAltitudeMeters - CurrentState.TerrainHeightMeters;
	NextState.DistanceKm = CurrentState.DistanceKm + (GroundSpeedKmh * DeltaSeconds) / 3600.0f;
	NextState.ElapsedSeconds = CurrentState.ElapsedSeconds + DeltaSeconds;
	NextState.TurnRateDegPerSecond = TurnRateDegPerSecond;
	NextState.StallWarning = StallWarning;
	NextState.FlareEffectiveness = FlareEffectiveness;
	NextState.LandingZoneDistanceMeters = LandingMetrics.bValid ? LandingMetrics.DistanceMeters : -1.0f;
	NextState.LandingApproachErrorDeg = LandingMetrics.bValid ? LandingMetrics.ApproachErrorDeg : -1.0f;
	NextState.LandingRating = EParaglideLandingRating::None;
	NextState.FlightPhase = GetFlightPhase(NextState.ElapsedSeconds, NextState.GroundClearanceMeters, FlareEffectiveness, Tuning);
	NextState.Debug.BaseSinkMetersPerSecond = BaseSinkMetersPerSecond;
	NextState.Debug.InducedTurnSinkMetersPerSecond = InducedTurnSinkMetersPerSecond;
	NextState.Debug.BrakeSinkMetersPerSecond = BrakeSinkMetersPerSecond;
	NextState.Debug.StallSinkMetersPerSecond = StallSinkMetersPerSecond;
	NextState.Debug.TurbulenceLiftMetersPerSecond = TurbulenceLiftMetersPerSecond;
	NextState.Debug.FlareLiftMetersPerSecond = FlareLiftMetersPerSecond;
	NextState.Debug.TotalSinkMetersPerSecond = TotalSinkMetersPerSecond;
	return NextState;
}
