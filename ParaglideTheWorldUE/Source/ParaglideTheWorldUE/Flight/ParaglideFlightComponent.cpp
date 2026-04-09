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
		float LeftFrontRiserTravel = 0.0f;
		float LeftRearRiserTravel = 0.0f;
		float LeftBrakeTravel = 0.0f;
		float RightFrontRiserTravel = 0.0f;
		float RightRearRiserTravel = 0.0f;
		float RightBrakeTravel = 0.0f;
		float SymmetricFrontRiser = 0.0f;
		float SymmetricRearRiser = 0.0f;
		float SymmetricBrake = 0.0f;
		float FrontRiserDifferential = 0.0f;
		float RearRiserDifferential = 0.0f;
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
			Controls.SymmetricRearRiser * Tuning.Controls.RearRiserAirspeedLossKmh -
			Controls.SymmetricBrake * Tuning.Controls.SymmetricBrakeAirspeedLossKmh -
			FMath::Square(Controls.SymmetricBrake) * Tuning.Controls.DeepBrakeAirspeedLossKmh +
			Controls.SymmetricFrontRiser * Tuning.Controls.FrontRiserAirspeedGainKmh +
			(Assist.InputResponsiveness - 0.9f) * Tuning.Controls.ResponsivenessAirspeedGainKmh;

		return FMath::Clamp(
			TargetAirspeed,
			Tuning.BaselineWing.MinControllableAirspeedKmh,
			Tuning.BaselineWing.MaxAirspeedKmh);
	}

	float ApproachAirspeedKmh(const float Current, const float Target, const float RiseStep, const float FallStep)
	{
		if (Current < Target)
		{
			return FMath::Min(Current + RiseStep, Target);
		}

		return FMath::Max(Current - FallStep, Target);
	}

	float GetTurnRateDegPerSecond(const float BankDeg, const float AirspeedKmh, const FParaglideFlightTuning& Tuning)
	{
		const float AirspeedMetersPerSecond = FMath::Max(
			AirspeedKmh / 3.6f,
			Tuning.BaselineWing.TurnRateMinAirspeedMetersPerSecond);
		const float BankRadians = FMath::DegreesToRadians(BankDeg);

		return FMath::RadiansToDegrees((9.81f * FMath::Tan(BankRadians)) / AirspeedMetersPerSecond);
	}

	float GetSignedTurnInput(const FDerivedControls& Controls, const float FallbackValue)
	{
		if (!FMath::IsNearlyZero(Controls.BrakeDifferential, KINDA_SMALL_NUMBER))
		{
			return FMath::Sign(Controls.BrakeDifferential);
		}

		if (!FMath::IsNearlyZero(Controls.RearRiserDifferential, KINDA_SMALL_NUMBER))
		{
			return FMath::Sign(Controls.RearRiserDifferential);
		}

		if (!FMath::IsNearlyZero(Controls.FrontRiserDifferential, KINDA_SMALL_NUMBER))
		{
			return FMath::Sign(Controls.FrontRiserDifferential);
		}

		if (!FMath::IsNearlyZero(Controls.WeightShift, KINDA_SMALL_NUMBER))
		{
			return FMath::Sign(Controls.WeightShift);
		}

		return FMath::IsNearlyZero(FallbackValue, KINDA_SMALL_NUMBER) ? 1.0f : FMath::Sign(FallbackValue);
	}

	EParaglideFlightPhase GetFlightPhase(
		const EParaglideFlightPhase CurrentPhase,
		const float ElapsedSeconds,
		const float GroundClearanceMeters,
		const float WingInflation,
		const float FlareEffectiveness,
		const FParaglideFlightTuning& Tuning)
	{
		EParaglideFlightPhase FlightPhase = EParaglideFlightPhase::Soaring;

		if (CurrentPhase == EParaglideFlightPhase::Launch &&
			(GroundClearanceMeters < Tuning.Launch.FreeFlightSwitchHeightMeters || WingInflation < 0.94f || ElapsedSeconds < Tuning.PhaseModel.LaunchDurationSeconds))
		{
			FlightPhase = EParaglideFlightPhase::Launch;
		}

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
		Derived.LeftFrontRiserTravel = Controls.LeftFrontRiserTravel;
		Derived.LeftRearRiserTravel = Controls.LeftRearRiserTravel;
		Derived.LeftBrakeTravel = Controls.LeftBrakeTravel;
		Derived.RightFrontRiserTravel = Controls.RightFrontRiserTravel;
		Derived.RightRearRiserTravel = Controls.RightRearRiserTravel;
		Derived.RightBrakeTravel = Controls.RightBrakeTravel;
		Derived.SymmetricFrontRiser = (Controls.LeftFrontRiserTravel + Controls.RightFrontRiserTravel) * 0.5f;
		Derived.SymmetricRearRiser = (Controls.LeftRearRiserTravel + Controls.RightRearRiserTravel) * 0.5f;
		Derived.SymmetricBrake = (Controls.LeftBrakeTravel + Controls.RightBrakeTravel) * 0.5f;
		Derived.FrontRiserDifferential = Controls.RightFrontRiserTravel - Controls.LeftFrontRiserTravel;
		Derived.RearRiserDifferential = Controls.RightRearRiserTravel - Controls.LeftRearRiserTravel;
		Derived.BrakeDifferential = Controls.RightBrakeTravel - Controls.LeftBrakeTravel;
		Derived.WeightShift = Controls.WeightShiftPosition;
		Derived.SpeedBarTravel = Controls.SpeedBarTravel;
		return Derived;
	}

	void StepControls(FParaglideControlState& Controls, const float DeltaSeconds)
	{
		const float LeftFrontRiserTarget = FMath::Clamp(Controls.LeftFrontRiserTarget, 0.0f, 1.0f);
		const float LeftRearRiserTarget = FMath::Clamp(Controls.LeftRearRiserTarget, 0.0f, 1.0f);
		const float LeftBrakeTarget = FMath::Clamp(Controls.LeftBrakeTarget, 0.0f, 1.0f);
		const float RightFrontRiserTarget = FMath::Clamp(Controls.RightFrontRiserTarget, 0.0f, 1.0f);
		const float RightRearRiserTarget = FMath::Clamp(Controls.RightRearRiserTarget, 0.0f, 1.0f);
		const float RightBrakeTarget = FMath::Clamp(Controls.RightBrakeTarget, 0.0f, 1.0f);
		const float WeightTarget = FMath::Clamp(Controls.WeightShiftTarget, -1.0f, 1.0f);
		const float SpeedBarTarget = FMath::Clamp(Controls.SpeedBarTarget, 0.0f, 1.0f);
		const float FrontRiserStep = DeltaSeconds * 5.8f;
		const float FrontRiserReleaseStep = DeltaSeconds * 6.8f;
		const float RearRiserStep = DeltaSeconds * 5.0f;
		const float RearRiserReleaseStep = DeltaSeconds * 6.6f;
		const float BrakeStep = DeltaSeconds * 4.4f;
		const float BrakeReleaseStep = DeltaSeconds * 6.2f;
		const float WeightStep = DeltaSeconds * 6.4f;
		const float WeightReleaseStep = DeltaSeconds * 7.2f;
		const float SpeedBarStep = DeltaSeconds * 3.0f;
		const float SpeedBarReleaseStep = DeltaSeconds * 3.8f;

		Controls.LeftFrontRiserTravel = FMath::Clamp(
			ApproachValue(Controls.LeftFrontRiserTravel, LeftFrontRiserTarget, Controls.LeftFrontRiserTravel < LeftFrontRiserTarget ? FrontRiserStep : FrontRiserReleaseStep),
			0.0f,
			1.0f);
		Controls.LeftRearRiserTravel = FMath::Clamp(
			ApproachValue(Controls.LeftRearRiserTravel, LeftRearRiserTarget, Controls.LeftRearRiserTravel < LeftRearRiserTarget ? RearRiserStep : RearRiserReleaseStep),
			0.0f,
			1.0f);
		Controls.LeftBrakeTravel = FMath::Clamp(
			ApproachValue(Controls.LeftBrakeTravel, LeftBrakeTarget, Controls.LeftBrakeTravel < LeftBrakeTarget ? BrakeStep : BrakeReleaseStep),
			0.0f,
			1.0f);
		Controls.RightFrontRiserTravel = FMath::Clamp(
			ApproachValue(Controls.RightFrontRiserTravel, RightFrontRiserTarget, Controls.RightFrontRiserTravel < RightFrontRiserTarget ? FrontRiserStep : FrontRiserReleaseStep),
			0.0f,
			1.0f);
		Controls.RightRearRiserTravel = FMath::Clamp(
			ApproachValue(Controls.RightRearRiserTravel, RightRearRiserTarget, Controls.RightRearRiserTravel < RightRearRiserTarget ? RearRiserStep : RearRiserReleaseStep),
			0.0f,
			1.0f);
		Controls.RightBrakeTravel = FMath::Clamp(
			ApproachValue(Controls.RightBrakeTravel, RightBrakeTarget, Controls.RightBrakeTravel < RightBrakeTarget ? BrakeStep : BrakeReleaseStep),
			0.0f,
			1.0f);
		Controls.WeightShiftPosition = FMath::Clamp(
			ApproachValue(Controls.WeightShiftPosition, WeightTarget, FMath::IsNearlyZero(WeightTarget, KINDA_SMALL_NUMBER) ? WeightReleaseStep : WeightStep),
			-1.0f,
			1.0f);
		Controls.SpeedBarTravel = FMath::Clamp(
			ApproachValue(Controls.SpeedBarTravel, SpeedBarTarget, Controls.SpeedBarTravel < SpeedBarTarget ? SpeedBarStep : SpeedBarReleaseStep),
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

void UParaglideFlightComponent::ApplyControlInputs(const FParaglideControlState& ControlState)
{
	Controls.bWeightLeftPressed = ControlState.bWeightLeftPressed;
	Controls.bWeightRightPressed = ControlState.bWeightRightPressed;
	Controls.bLeftBrakePressed = ControlState.bLeftBrakePressed;
	Controls.bRightBrakePressed = ControlState.bRightBrakePressed;
	Controls.bSpeedBarPressed = ControlState.bSpeedBarPressed;
	Controls.bLeftFrontRiserPressed = ControlState.bLeftFrontRiserPressed;
	Controls.bLeftRearRiserPressed = ControlState.bLeftRearRiserPressed;
	Controls.bLeftBrakePrimaryPressed = ControlState.bLeftBrakePrimaryPressed;
	Controls.bLeftBrakeDeepPressed = ControlState.bLeftBrakeDeepPressed;
	Controls.bRightBrakeDeepPressed = ControlState.bRightBrakeDeepPressed;
	Controls.bRightBrakePrimaryPressed = ControlState.bRightBrakePrimaryPressed;
	Controls.bRightRearRiserPressed = ControlState.bRightRearRiserPressed;
	Controls.bRightFrontRiserPressed = ControlState.bRightFrontRiserPressed;
	Controls.bSpeedBarStageOnePressed = ControlState.bSpeedBarStageOnePressed;
	Controls.bSpeedBarStageTwoPressed = ControlState.bSpeedBarStageTwoPressed;
	Controls.LeftFrontRiserTarget = FMath::Clamp(ControlState.LeftFrontRiserTarget, 0.0f, 1.0f);
	Controls.LeftRearRiserTarget = FMath::Clamp(ControlState.LeftRearRiserTarget, 0.0f, 1.0f);
	Controls.LeftBrakeTarget = FMath::Clamp(ControlState.LeftBrakeTarget, 0.0f, 1.0f);
	Controls.RightFrontRiserTarget = FMath::Clamp(ControlState.RightFrontRiserTarget, 0.0f, 1.0f);
	Controls.RightRearRiserTarget = FMath::Clamp(ControlState.RightRearRiserTarget, 0.0f, 1.0f);
	Controls.RightBrakeTarget = FMath::Clamp(ControlState.RightBrakeTarget, 0.0f, 1.0f);
	Controls.WeightShiftTarget = FMath::Clamp(ControlState.WeightShiftTarget, -1.0f, 1.0f);
	Controls.SpeedBarTarget = FMath::Clamp(ControlState.SpeedBarTarget, 0.0f, 1.0f);
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
	FlightState.WingInflation = FMath::Clamp(FlightState.WingInflation, 0.0f, 1.0f);
	FlightState.CanopyPressure = FMath::Clamp(FlightState.CanopyPressure, 0.1f, 1.2f);
	FlightState.AngleOfAttackDeg = FMath::Clamp(FlightState.AngleOfAttackDeg, -6.0f, 18.0f);
	FlightState.LeftCollapseAmount = FMath::Clamp(FlightState.LeftCollapseAmount, 0.0f, 1.0f);
	FlightState.RightCollapseAmount = FMath::Clamp(FlightState.RightCollapseAmount, 0.0f, 1.0f);
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
		NextState.DiveEnergy = 0.0f;
		NextState.TumbleAmount = 0.0f;
		NextState.SpinRateDegPerSecond = 0.0f;
		NextState.LoadFactor = 1.0f;
		NextState.WingInflation = ApproachValue(CurrentState.WingInflation, 0.72f, DeltaSeconds * 0.9f);
		NextState.CanopyPressure = ApproachValue(CurrentState.CanopyPressure, 0.58f, DeltaSeconds * 1.2f);
		NextState.AngleOfAttackDeg = ApproachValue(CurrentState.AngleOfAttackDeg, Tuning.Airfoil.TrimAngleOfAttackDeg, DeltaSeconds * 4.0f);
		NextState.LeftCollapseAmount = ApproachValue(CurrentState.LeftCollapseAmount, 0.0f, DeltaSeconds * Tuning.Airfoil.CollapseRecoveryRate);
		NextState.RightCollapseAmount = ApproachValue(CurrentState.RightCollapseAmount, 0.0f, DeltaSeconds * Tuning.Airfoil.CollapseRecoveryRate);
		NextState.WingSurgeDeg = ApproachValue(CurrentState.WingSurgeDeg, 0.0f, DeltaSeconds * Tuning.Airfoil.SurgeResponseRate);
		NextState.Debug.DiveSinkMetersPerSecond = 0.0f;
		NextState.Debug.SpiralSinkMetersPerSecond = 0.0f;
		NextState.Debug.TumbleSinkMetersPerSecond = 0.0f;
		NextState.Debug.TurbulenceLiftMetersPerSecond = 0.0f;
		NextState.Debug.FlareLiftMetersPerSecond = 0.0f;
		return NextState;
	}

	const float EffectiveTurbulence = FMath::Clamp(Atmosphere.Turbulence * (1.0f - AssistProfile.TurbulenceDamping), 0.0f, 1.0f);
	const float FrontRiserGroundRunGainKmh =
		CurrentState.FlightPhase == EParaglideFlightPhase::Launch
			? DerivedControls.SymmetricFrontRiser * Tuning.Launch.FrontRiserGroundRunSpeedGainKmh
			: 0.0f;
	const float AirspeedTargetKmh = GetTargetAirspeedKmh(DerivedControls, AssistProfile, Tuning) + FrontRiserGroundRunGainKmh;
	const float AirspeedRiseStep = DeltaSeconds * Tuning.Controls.AirspeedResponseRate * AssistProfile.InputResponsiveness * FMath::Lerp(1.0f, 1.45f, DerivedControls.SpeedBarTravel);
	const float AirspeedFallStep = DeltaSeconds * Tuning.Controls.AirspeedResponseRate * 0.42f;
	float AirspeedKmh = ApproachAirspeedKmh(CurrentState.AirspeedKmh, AirspeedTargetKmh, AirspeedRiseStep, AirspeedFallStep);
	const float AngleOfAttackDeg = FMath::Clamp(
		Tuning.Airfoil.TrimAngleOfAttackDeg +
		DerivedControls.SymmetricBrake * Tuning.Airfoil.BrakeAngleOfAttackGainDeg +
		DerivedControls.SymmetricRearRiser * Tuning.Airfoil.RearRiserAngleOfAttackGainDeg -
		DerivedControls.SymmetricFrontRiser * Tuning.Airfoil.FrontRiserAngleOfAttackLossDeg -
		DerivedControls.SpeedBarTravel * Tuning.Airfoil.SpeedBarAngleOfAttackLossDeg +
		CurrentState.WingSurgeDeg * 0.08f,
		-8.0f,
		20.0f);
	const float InflationTarget = FMath::Clamp(
		CurrentState.FlightPhase == EParaglideFlightPhase::Launch
			? 0.26f +
				DerivedControls.SymmetricFrontRiser * Tuning.Launch.FrontRiserInflationGain +
				FMath::Clamp((AirspeedKmh - 12.0f) / Tuning.BaselineWing.TrimAirspeedKmh, 0.0f, 1.0f) * Tuning.Launch.AirspeedInflationGain -
				DerivedControls.SymmetricBrake * Tuning.Launch.BrakeInflationPenalty
			: 0.9f +
				FMath::Clamp((AirspeedKmh - Tuning.BaselineWing.MinControllableAirspeedKmh) /
					FMath::Max(Tuning.BaselineWing.TrimAirspeedKmh - Tuning.BaselineWing.MinControllableAirspeedKmh, 1.0f), 0.0f, 1.0f) * 0.16f -
				DerivedControls.SymmetricFrontRiser * 0.12f -
				DerivedControls.SymmetricBrake * 0.05f,
		0.0f,
		1.0f);
	const float WingInflation = FMath::Clamp(
		ApproachValue(
			CurrentState.WingInflation,
			InflationTarget,
			DeltaSeconds * (CurrentState.WingInflation < InflationTarget ? Tuning.Launch.InflationBuildRate : Tuning.Launch.InflationDecayRate)),
		0.0f,
		1.0f);
	const float AirspeedPressureFactor = FMath::Clamp(
		(AirspeedKmh - Tuning.BaselineWing.MinControllableAirspeedKmh) /
			FMath::Max(Tuning.BaselineWing.MaxAirspeedKmh - Tuning.BaselineWing.MinControllableAirspeedKmh, 1.0f),
		0.0f,
		1.0f);
	float PressureTarget =
		Tuning.Airfoil.BasePressure +
		AirspeedPressureFactor * Tuning.Airfoil.AirspeedPressureGain +
		WingInflation * Tuning.Airfoil.InflationPressureGain -
		DerivedControls.SymmetricFrontRiser * Tuning.Airfoil.FrontRiserPressurePenalty -
		DerivedControls.SpeedBarTravel * Tuning.Airfoil.SpeedBarPressurePenalty;
	PressureTarget -= FMath::Clamp((Tuning.Airfoil.TrimAngleOfAttackDeg - AngleOfAttackDeg) / 8.0f, 0.0f, 1.0f) * Tuning.Airfoil.LowAoAPressurePenalty;
	PressureTarget -= FMath::Clamp((AngleOfAttackDeg - (Tuning.Airfoil.TrimAngleOfAttackDeg + 6.0f)) / 8.0f, 0.0f, 1.0f) * Tuning.Airfoil.HighAoAPressurePenalty;
	PressureTarget -= EffectiveTurbulence * 0.08f;
	const float CanopyPressure = FMath::Clamp(
		ApproachValue(CurrentState.CanopyPressure, PressureTarget, DeltaSeconds * 4.8f),
		0.05f,
		1.2f);
	const float SurgeTargetDeg =
		DerivedControls.SymmetricFrontRiser * Tuning.Airfoil.FrontRiserSurgeGainDeg +
		DerivedControls.SpeedBarTravel * Tuning.Airfoil.SpeedBarSurgeGainDeg -
		DerivedControls.SymmetricBrake * Tuning.Airfoil.BrakeSurgeDampingDeg -
		DerivedControls.SymmetricRearRiser * 6.0f;
	const float WingSurgeDeg = ApproachValue(CurrentState.WingSurgeDeg, SurgeTargetDeg, DeltaSeconds * Tuning.Airfoil.SurgeResponseRate);
	const float GustShock = FMath::Max(
		0.0f,
		FMath::Abs(FMath::Sin(CurrentState.ElapsedSeconds * 3.6f + CurrentState.PositionMeters.X * 0.015f - CurrentState.PositionMeters.Y * 0.011f)) *
			EffectiveTurbulence -
			0.18f);
	const float CollapseRisk = FMath::Clamp(
		(Tuning.Airfoil.CollapsePressureThreshold - CanopyPressure) / Tuning.Airfoil.CollapsePressureRange +
		GustShock * 0.7f +
		FMath::Clamp((FMath::Abs(WingSurgeDeg) - 8.0f) / 18.0f, 0.0f, 1.0f) * 0.45f +
		DerivedControls.SymmetricFrontRiser * 0.24f,
		0.0f,
		1.0f);
	const float AsymmetryInput = FMath::Clamp(
		FMath::Max(
			FMath::Max(
				FMath::Abs(DerivedControls.BrakeDifferential),
				FMath::Abs(DerivedControls.RearRiserDifferential) * 0.7f),
			FMath::Max(
				FMath::Abs(DerivedControls.FrontRiserDifferential) * 0.6f,
				FMath::Abs(DerivedControls.WeightShift) * 0.85f)),
		0.0f,
		1.0f);
	const float CollapseBias = FMath::Clamp(
		DerivedControls.BrakeDifferential * 0.42f +
		DerivedControls.RearRiserDifferential * 0.22f +
		DerivedControls.FrontRiserDifferential * 0.24f +
		DerivedControls.WeightShift * 0.18f,
		-1.0f,
		1.0f);
	const float LeftCollapseTarget = CollapseRisk * FMath::Clamp(0.5f - CollapseBias * 0.5f, 0.05f, 1.0f);
	const float RightCollapseTarget = CollapseRisk * FMath::Clamp(0.5f + CollapseBias * 0.5f, 0.05f, 1.0f);
	const float LeftCollapseAmount = FMath::Clamp(
		ApproachValue(
			CurrentState.LeftCollapseAmount,
			LeftCollapseTarget,
			DeltaSeconds * (CurrentState.LeftCollapseAmount < LeftCollapseTarget ? Tuning.Airfoil.CollapseBuildRate : Tuning.Airfoil.CollapseRecoveryRate)),
		0.0f,
		1.0f);
	const float RightCollapseAmount = FMath::Clamp(
		ApproachValue(
			CurrentState.RightCollapseAmount,
			RightCollapseTarget,
			DeltaSeconds * (CurrentState.RightCollapseAmount < RightCollapseTarget ? Tuning.Airfoil.CollapseBuildRate : Tuning.Airfoil.CollapseRecoveryRate)),
		0.0f,
		1.0f);
	const float CollapseAverage = (LeftCollapseAmount + RightCollapseAmount) * 0.5f;
	const float CollapseDifferential = RightCollapseAmount - LeftCollapseAmount;
	const float StallWarning = FMath::Clamp(
		(AngleOfAttackDeg - (Tuning.Airfoil.TrimAngleOfAttackDeg + 4.6f)) / 7.2f +
		DerivedControls.SymmetricRearRiser * Tuning.Controls.RearRiserStallContribution +
		(Tuning.Stability.StallAirspeedReferenceKmh - AirspeedKmh) / Tuning.Stability.StallAirspeedRangeKmh -
		DerivedControls.SpeedBarTravel * Tuning.Stability.SpeedBarStallRelief -
		AssistProfile.RecoveryAssist * Tuning.Stability.RecoveryAssistStallRelief +
		FMath::Clamp((0.58f - CanopyPressure) / 0.22f, 0.0f, 1.0f) * 0.5f +
		CollapseAverage * 0.22f,
		0.0f,
		1.0f);
	const float SignedTurnInput = GetSignedTurnInput(DerivedControls, CurrentState.SpinRateDegPerSecond);
	const float DiveDrive = FMath::Clamp(
		DerivedControls.SpeedBarTravel * 1.15f +
		DerivedControls.SymmetricFrontRiser * 0.58f +
		FMath::Clamp((FMath::Abs(CurrentState.BankDeg) - 35.0f) / 55.0f, 0.0f, 1.0f) * 0.45f +
		FMath::Clamp((CurrentState.AirspeedKmh - Tuning.BaselineWing.TrimAirspeedKmh) / 18.0f, 0.0f, 1.0f) * 0.25f -
		DerivedControls.SymmetricRearRiser * 0.18f -
		DerivedControls.SymmetricBrake * 0.48f,
		0.0f,
		1.0f);
	const float DiveEnergy = FMath::Clamp(
		CurrentState.DiveEnergy +
		(DiveDrive * Tuning.Maneuvers.DiveBuildRate - (1.0f - DiveDrive) * Tuning.Maneuvers.DiveRecoveryRate) * DeltaSeconds,
		0.0f,
		1.0f);
	const float TumbleTrigger = FMath::Clamp(
		(StallWarning - Tuning.Maneuvers.TumbleTriggerStall) / (1.0f - Tuning.Maneuvers.TumbleTriggerStall),
		0.0f,
		1.0f) * FMath::Clamp(
			(AsymmetryInput - Tuning.Maneuvers.TumbleTriggerAsymmetry) / (1.0f - Tuning.Maneuvers.TumbleTriggerAsymmetry),
			0.0f,
			1.0f) *
		FMath::Lerp(0.75f, 1.25f, CollapseAverage);
	const float TumbleAmount = FMath::Clamp(
		CurrentState.TumbleAmount +
		(TumbleTrigger * Tuning.Maneuvers.TumbleBuildRate - (1.0f - TumbleTrigger) * Tuning.Maneuvers.TumbleRecoveryRate) * DeltaSeconds,
		0.0f,
		1.0f);
	const float TargetSpinRateDegPerSecond = TumbleAmount > KINDA_SMALL_NUMBER
		? SignedTurnInput * (Tuning.Maneuvers.TumbleRollRateDegPerSecond * (0.35f + TumbleAmount * 0.65f))
		: 0.0f;
	const float SpinRateDegPerSecond = ApproachValue(CurrentState.SpinRateDegPerSecond, TargetSpinRateDegPerSecond, DeltaSeconds * 680.0f);
	const float ControlAuthority = FMath::Clamp(
		AssistProfile.InputResponsiveness +
		AssistProfile.CoordinationAssist * Tuning.Stability.CoordinationAuthorityGain -
		StallWarning * Tuning.Stability.StallAuthorityPenalty -
		CollapseAverage * Tuning.Airfoil.CollapseAuthorityPenalty,
		Tuning.Stability.ControlAuthorityFloor,
		Tuning.Stability.ControlAuthorityCeiling) *
		FMath::Lerp(0.38f, 1.0f, WingInflation) *
		FMath::Lerp(0.45f, 1.0f, FMath::Clamp(CanopyPressure, 0.0f, 1.0f));
	const float GustBankDeg =
		FMath::Sin(CurrentState.ElapsedSeconds * 1.9f + CurrentState.HeadingDeg / 23.0f) *
		EffectiveTurbulence * Tuning.Stability.GustRollMagnitudeDeg;
	const float ManeuverBankLimitDeg = FMath::Lerp(
		Tuning.Stability.BankLimitDeg,
		Tuning.Maneuvers.AcroBankLimitDeg,
		FMath::Max(DiveEnergy * 0.9f, TumbleAmount));
	const float CommandedBankDeg =
		(DerivedControls.BrakeDifferential * (Tuning.Controls.BrakeToRollDeg + DiveEnergy * Tuning.Maneuvers.AcroBankBonusDeg) +
			DerivedControls.RearRiserDifferential * (Tuning.Controls.RearRiserToRollDeg + DiveEnergy * (Tuning.Maneuvers.AcroBankBonusDeg * 0.24f)) +
			DerivedControls.FrontRiserDifferential * Tuning.Controls.FrontRiserToRollDeg +
			DerivedControls.WeightShift * (Tuning.Controls.WeightShiftAuthorityDeg + DiveEnergy * (Tuning.Maneuvers.AcroBankBonusDeg * 0.45f))) *
		ControlAuthority;
	const float CollapseBankDeg = CollapseDifferential * Tuning.Airfoil.CollapseBankGainDeg;
	const float TumbleBankDeg = TumbleAmount * SignedTurnInput * (64.0f + TumbleAmount * 48.0f);
	const float TargetBankDeg = FMath::Clamp(
		CommandedBankDeg + GustBankDeg + CollapseBankDeg + TumbleBankDeg,
		-ManeuverBankLimitDeg,
		ManeuverBankLimitDeg);
	const float BankDeg = ApproachValue(
		CurrentState.BankDeg,
		TargetBankDeg,
		DeltaSeconds * (Tuning.Controls.BankResponseRate + AssistProfile.CoordinationAssist * Tuning.Controls.CoordinationBankResponseGain + DiveEnergy * 28.0f + TumbleAmount * 18.0f));
	const float BaseTurnRateDegPerSecond =
		GetTurnRateDegPerSecond(BankDeg, AirspeedKmh, Tuning) *
		(1.0f + DiveEnergy * Tuning.Maneuvers.DiveTurnRateBonus) *
		(1.0f - StallWarning * Tuning.Stability.StallTurnRatePenalty);
	const float TurnRateDegPerSecond = BaseTurnRateDegPerSecond + SpinRateDegPerSecond * TumbleAmount * 0.22f;
	const float HeadingDeg = WrapDegrees(
		CurrentState.HeadingDeg +
		(TurnRateDegPerSecond + TumbleAmount * SignedTurnInput * Tuning.Maneuvers.TumbleHeadingRateDegPerSecond * 0.5f) * DeltaSeconds);

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
	const float DiveSinkMetersPerSecond = DiveEnergy * Tuning.Maneuvers.DiveSinkMultiplier;
	const float SpiralSinkMetersPerSecond =
		FMath::Clamp((FMath::Abs(BankDeg) - 58.0f) / 42.0f, 0.0f, 1.0f) *
		(0.8f + DiveEnergy) *
		Tuning.Maneuvers.SpiralSinkMultiplier;
	const float BrakeSinkMetersPerSecond =
		DerivedControls.SymmetricBrake * Tuning.Controls.BrakeSinkLinearMetersPerSecond +
		FMath::Square(DerivedControls.SymmetricBrake) * Tuning.Controls.BrakeSinkQuadraticMetersPerSecond;
	const float FrontRiserSinkMetersPerSecond =
		DerivedControls.SymmetricFrontRiser * 0.18f +
		FMath::Square(DerivedControls.SymmetricFrontRiser) * 0.34f;
	const float RearRiserSinkMetersPerSecond =
		DerivedControls.SymmetricRearRiser * Tuning.Controls.RearRiserSinkLinearMetersPerSecond +
		FMath::Square(DerivedControls.SymmetricRearRiser) * Tuning.Controls.RearRiserSinkQuadraticMetersPerSecond;
	const float StallSinkMetersPerSecond = FMath::Square(StallWarning) * Tuning.Stability.StallSinkMultiplier;
	const float TumbleSinkMetersPerSecond = TumbleAmount * Tuning.Maneuvers.TumbleSinkMultiplier;
	const float CollapseSinkMetersPerSecond = CollapseAverage * Tuning.Airfoil.CollapseSinkMetersPerSecond;
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

	AirspeedKmh = FMath::Clamp(
		AirspeedKmh +
		DerivedControls.SpeedBarTravel * 4.0f +
		DiveEnergy * Tuning.Maneuvers.DiveAirspeedGainKmh -
		TumbleAmount * (4.0f + StallWarning * 6.0f) -
		CollapseAverage * 5.4f -
		FlareDragKmh,
		Tuning.BaselineWing.FlareMinAirspeedKmh,
		Tuning.BaselineWing.MaxAirspeedKmh);

	const float AirMassSinkMetersPerSecond = LeeSinkMetersPerSecond + ThermalSinkMetersPerSecond;
	const float LiftEfficiency = FMath::Clamp(
		0.44f +
		WingInflation * 0.50f +
		CanopyPressure * 0.38f -
		CollapseAverage * 0.22f,
		0.40f,
		1.35f);
	const float LaunchLiftMetersPerSecond =
		CurrentState.FlightPhase == EParaglideFlightPhase::Launch
			? FMath::Clamp(
				(WingInflation * CanopyPressure - Tuning.Launch.TakeoffPressureThreshold) / 0.24f,
				0.0f,
				1.0f) *
				Tuning.Launch.LaunchLiftMultiplier *
				FMath::Clamp((AirspeedKmh - Tuning.BaselineWing.MinControllableAirspeedKmh * 0.7f) / 10.0f, 0.0f, 1.0f)
			: 0.0f;
	const float EffectiveThermalLiftMetersPerSecond =
		ThermalLiftMetersPerSecond * (1.12f + (1.0f - DerivedControls.SymmetricBrake) * 0.08f);
	const float EffectiveRidgeLiftMetersPerSecond =
		RidgeLiftMetersPerSecond * (1.16f - DerivedControls.SymmetricBrake * 0.05f);
	const float TotalSinkMetersPerSecond =
		BaseSinkMetersPerSecond +
		InducedTurnSinkMetersPerSecond +
		DiveSinkMetersPerSecond +
		SpiralSinkMetersPerSecond +
		FrontRiserSinkMetersPerSecond +
		RearRiserSinkMetersPerSecond +
		BrakeSinkMetersPerSecond +
		StallSinkMetersPerSecond +
		CollapseSinkMetersPerSecond +
		TumbleSinkMetersPerSecond +
		AirMassSinkMetersPerSecond;
	const float VerticalSpeedMetersPerSecond =
		(EffectiveThermalLiftMetersPerSecond +
		 EffectiveRidgeLiftMetersPerSecond +
		 TurbulenceLiftMetersPerSecond +
		 FlareLiftMetersPerSecond +
		 LaunchLiftMetersPerSecond) * LiftEfficiency -
		TotalSinkMetersPerSecond;
	const float DynamicMinPitchDeg = FMath::Lerp(Tuning.Attitude.MinPitchDeg, -Tuning.Maneuvers.TumblePitchLimitDeg, FMath::Max(DiveEnergy, TumbleAmount));
	const float DynamicMaxPitchDeg = FMath::Lerp(Tuning.Attitude.MaxPitchDeg, Tuning.Maneuvers.TumblePitchLimitDeg * 0.8f, TumbleAmount);
	const float TumblePitchOffsetDeg =
		TumbleAmount *
		FMath::Sin(CurrentState.ElapsedSeconds * 8.5f + (SpinRateDegPerSecond < 0.0f ? PI * 0.5f : -PI * 0.5f)) *
		Tuning.Maneuvers.TumblePitchLimitDeg;
	const float TargetPitchDeg = FMath::Clamp(
		Tuning.Attitude.TrimPitchDeg +
		AngleOfAttackDeg * 0.45f -
		DerivedControls.SpeedBarTravel * Tuning.Attitude.SpeedBarPitchGainDeg -
		DerivedControls.SymmetricFrontRiser * Tuning.Attitude.FrontRiserPitchDiveDeg +
		DerivedControls.SymmetricRearRiser * Tuning.Attitude.RearRiserPitchGainDeg +
		DerivedControls.SymmetricBrake * Tuning.Attitude.SymmetricBrakePitchLossDeg +
		StallWarning * Tuning.Attitude.StallPitchGainDeg +
		FlareEffectiveness * Tuning.Attitude.FlarePitchGainDeg -
		EffectiveTurbulence * Tuning.Attitude.TurbulencePitchGainDeg -
		WingSurgeDeg * 0.55f -
		CollapseAverage * Tuning.Airfoil.CollapsePitchLossDeg -
		DiveEnergy * Tuning.Maneuvers.DivePitchGainDeg +
		TumblePitchOffsetDeg,
		DynamicMinPitchDeg,
		DynamicMaxPitchDeg);
	const float PitchDeg = ApproachValue(CurrentState.PitchDeg, TargetPitchDeg, DeltaSeconds * (Tuning.Attitude.PitchResponseRate + DiveEnergy * 18.0f + TumbleAmount * 12.0f));

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
		NextState.WingInflation = WingInflation;
		NextState.CanopyPressure = CanopyPressure;
		NextState.AngleOfAttackDeg = AngleOfAttackDeg;
		NextState.LeftCollapseAmount = LeftCollapseAmount;
		NextState.RightCollapseAmount = RightCollapseAmount;
		NextState.WingSurgeDeg = WingSurgeDeg;
		NextState.DiveEnergy = DiveEnergy;
		NextState.TumbleAmount = TumbleAmount;
		NextState.SpinRateDegPerSecond = SpinRateDegPerSecond;
		NextState.LoadFactor = LoadFactor;
		NextState.LandingZoneDistanceMeters = LandingMetrics.bValid ? LandingMetrics.DistanceMeters : -1.0f;
		NextState.LandingApproachErrorDeg = LandingMetrics.bValid ? LandingMetrics.ApproachErrorDeg : -1.0f;
		NextState.LandingRating = LandingRating;
		NextState.FlightPhase = LandingRating == EParaglideLandingRating::Crash ? EParaglideFlightPhase::Crashed : EParaglideFlightPhase::Landed;
		NextState.Debug.BaseSinkMetersPerSecond = BaseSinkMetersPerSecond;
		NextState.Debug.InducedTurnSinkMetersPerSecond = InducedTurnSinkMetersPerSecond;
		NextState.Debug.BrakeSinkMetersPerSecond = BrakeSinkMetersPerSecond;
		NextState.Debug.StallSinkMetersPerSecond = StallSinkMetersPerSecond;
		NextState.Debug.DiveSinkMetersPerSecond = DiveSinkMetersPerSecond;
		NextState.Debug.SpiralSinkMetersPerSecond = SpiralSinkMetersPerSecond;
		NextState.Debug.TumbleSinkMetersPerSecond = TumbleSinkMetersPerSecond + CollapseSinkMetersPerSecond + FrontRiserSinkMetersPerSecond;
		NextState.Debug.TurbulenceLiftMetersPerSecond = TurbulenceLiftMetersPerSecond;
		NextState.Debug.FlareLiftMetersPerSecond = FlareLiftMetersPerSecond + LaunchLiftMetersPerSecond;
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
	NextState.WingInflation = WingInflation;
	NextState.CanopyPressure = CanopyPressure;
	NextState.AngleOfAttackDeg = AngleOfAttackDeg;
	NextState.LeftCollapseAmount = LeftCollapseAmount;
	NextState.RightCollapseAmount = RightCollapseAmount;
	NextState.WingSurgeDeg = WingSurgeDeg;
	NextState.DiveEnergy = DiveEnergy;
	NextState.TumbleAmount = TumbleAmount;
	NextState.SpinRateDegPerSecond = SpinRateDegPerSecond;
	NextState.LoadFactor = LoadFactor;
	NextState.LandingZoneDistanceMeters = LandingMetrics.bValid ? LandingMetrics.DistanceMeters : -1.0f;
	NextState.LandingApproachErrorDeg = LandingMetrics.bValid ? LandingMetrics.ApproachErrorDeg : -1.0f;
	NextState.LandingRating = EParaglideLandingRating::None;
	NextState.FlightPhase = GetFlightPhase(
		CurrentState.FlightPhase,
		NextState.ElapsedSeconds,
		NextState.GroundClearanceMeters,
		NextState.WingInflation,
		FlareEffectiveness,
		Tuning);
	NextState.Debug.BaseSinkMetersPerSecond = BaseSinkMetersPerSecond;
	NextState.Debug.InducedTurnSinkMetersPerSecond = InducedTurnSinkMetersPerSecond;
	NextState.Debug.BrakeSinkMetersPerSecond = BrakeSinkMetersPerSecond;
	NextState.Debug.StallSinkMetersPerSecond = StallSinkMetersPerSecond;
	NextState.Debug.DiveSinkMetersPerSecond = DiveSinkMetersPerSecond;
	NextState.Debug.SpiralSinkMetersPerSecond = SpiralSinkMetersPerSecond;
	NextState.Debug.TumbleSinkMetersPerSecond = TumbleSinkMetersPerSecond + CollapseSinkMetersPerSecond + FrontRiserSinkMetersPerSecond;
	NextState.Debug.TurbulenceLiftMetersPerSecond = TurbulenceLiftMetersPerSecond;
	NextState.Debug.FlareLiftMetersPerSecond = FlareLiftMetersPerSecond + LaunchLiftMetersPerSecond;
	NextState.Debug.TotalSinkMetersPerSecond = TotalSinkMetersPerSecond;
	return NextState;
}
