// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

struct FParaglideFlightTuning
{
	struct FBaselineWing
	{
		float TrimAirspeedKmh = 39.0f;
		float MinControllableAirspeedKmh = 18.0f;
		float MaxAirspeedKmh = 62.0f;
		float BaselineSinkMetersPerSecond = 0.72f;
		float SpeedToSinkCurveDivisor = 290.0f;
		float TurnRateMinAirspeedMetersPerSecond = 6.0f;
		float FlareMinAirspeedKmh = 9.0f;
	} BaselineWing;

	struct FControls
	{
		float SpeedBarAirspeedGainKmh = 30.0f;
		float FrontRiserAirspeedGainKmh = 11.0f;
		float SymmetricBrakeAirspeedLossKmh = 11.0f;
		float DeepBrakeAirspeedLossKmh = 5.5f;
		float RearRiserAirspeedLossKmh = 3.2f;
		float ResponsivenessAirspeedGainKmh = 4.0f;
		float FrontRiserToRollDeg = 10.0f;
		float BrakeToRollDeg = 28.0f;
		float RearRiserToRollDeg = 14.0f;
		float WeightShiftAuthorityDeg = 12.0f;
		float BrakeSinkLinearMetersPerSecond = 0.38f;
		float BrakeSinkQuadraticMetersPerSecond = 0.56f;
		float RearRiserSinkLinearMetersPerSecond = 0.10f;
		float RearRiserSinkQuadraticMetersPerSecond = 0.15f;
		float RearRiserStallContribution = 0.16f;
		float AirspeedResponseRate = 24.0f;
		float BankResponseRate = 30.0f;
		float CoordinationBankResponseGain = 34.0f;
	} Controls;

	struct FStability
	{
		float StallBrakeStart = 0.72f;
		float StallBrakeRange = 0.24f;
		float StallAirspeedReferenceKmh = 23.0f;
		float StallAirspeedRangeKmh = 8.0f;
		float SpeedBarStallRelief = 0.26f;
		float RecoveryAssistStallRelief = 0.65f;
		float CoordinationAuthorityGain = 0.42f;
		float StallAuthorityPenalty = 0.38f;
		float ControlAuthorityFloor = 0.72f;
		float ControlAuthorityCeiling = 1.24f;
		float GustRollMagnitudeDeg = 4.4f;
		float BankLimitDeg = 56.0f;
		float LoadFactorCosFloor = 0.74f;
		float InducedTurnSinkMultiplier = 0.58f;
		float StallTurnRatePenalty = 0.24f;
		float StallSinkMultiplier = 3.4f;
		float FlareStallPenalty = 0.3f;
	} Stability;

	struct FLiftSources
	{
		float FallbackRidgeTerrainWindowMeters = 4500.0f;
		float FallbackRidgeWindStartKmh = 8.0f;
		float FallbackRidgeWindRangeKmh = 16.0f;
		float FallbackRidgeWindCap = 1.5f;
		float RidgeWindStartKmh = 8.0f;
		float RidgeWindRangeKmh = 14.0f;
		float RidgeWindCap = 1.85f;
		float DefaultThermalDriftFactor = 0.28f;
		float ThermalCoreRadiusMultiplier = 0.35f;
		float ThermalCoreBaseFraction = 0.94f;
		float ThermalCoreCenteringBonus = 0.54f;
		float ThermalEdgeLiftFraction = 1.10f;
		float ThermalSinkRingRadiusMultiplier = 1.32f;
		float TurbulencePrimaryMix = 0.62f;
		float TurbulenceSecondaryMix = 0.38f;
		float TurbulenceVerticalGustScale = 0.8f;
	} LiftSources;

	struct FPhaseModel
	{
		float LaunchDurationSeconds = 8.0f;
		float ApproachHeightMeters = 24.0f;
		float FlareEffectivenessThreshold = 0.1f;
		float FlarePhaseHeightMeters = 10.0f;
		float FlareArmingHeightMeters = 8.0f;
	} PhaseModel;

	struct FLanding
	{
		float FlareBrakeStart = 0.58f;
		float FlareBrakeRange = 0.32f;
		float FlareLiftHeightMeters = 5.0f;
		float FlareLiftMultiplier = 3.4f;
		float FlareDragHeightMeters = 7.0f;
		float FlareDragMultiplierKmh = 10.0f;
		float TouchdownHeightMarginMeters = 0.4f;
		float TouchdownVerticalRelief = 0.72f;
		float TouchdownGroundRelief = 0.42f;
		float VerticalSpeedSeverityMultiplier = 2.35f;
		float GroundSpeedSeverityOffsetKmh = 12.0f;
		float GroundSpeedSeverityDivisor = 5.0f;
		float BankSeverityDivisorDeg = 10.0f;
		float ApproachErrorSeverityDivisorDeg = 32.0f;
		float LandingDistanceSeverityDivisorMeters = 38.0f;
		float SmoothSeverityThreshold = 2.4f;
		float FirmSeverityThreshold = 3.8f;
		float HardSeverityThreshold = 5.4f;
	} Landing;

	struct FAttitude
	{
		float TrimPitchDeg = -6.0f;
		float SpeedBarPitchGainDeg = 12.5f;
		float FrontRiserPitchDiveDeg = 10.2f;
		float SymmetricBrakePitchLossDeg = 8.0f;
		float RearRiserPitchGainDeg = 1.6f;
		float StallPitchGainDeg = 8.8f;
		float FlarePitchGainDeg = 6.2f;
		float TurbulencePitchGainDeg = 1.4f;
		float MinPitchDeg = -34.0f;
		float MaxPitchDeg = 12.0f;
		float PitchResponseRate = 40.0f;
	} Attitude;

	struct FAirfoil
	{
		float TrimAngleOfAttackDeg = 6.4f;
		float BrakeAngleOfAttackGainDeg = 8.0f;
		float RearRiserAngleOfAttackGainDeg = 2.5f;
		float FrontRiserAngleOfAttackLossDeg = 5.6f;
		float SpeedBarAngleOfAttackLossDeg = 6.2f;
		float BasePressure = 0.78f;
		float AirspeedPressureGain = 0.45f;
		float InflationPressureGain = 0.26f;
		float FrontRiserPressurePenalty = 0.20f;
		float SpeedBarPressurePenalty = 0.06f;
		float LowAoAPressurePenalty = 0.24f;
		float HighAoAPressurePenalty = 0.10f;
		float CollapsePressureThreshold = 0.32f;
		float CollapsePressureRange = 0.20f;
		float CollapseBuildRate = 1.0f;
		float CollapseRecoveryRate = 0.78f;
		float CollapseAuthorityPenalty = 0.50f;
		float CollapseSinkMetersPerSecond = 1.5f;
		float CollapseBankGainDeg = 28.0f;
		float CollapsePitchLossDeg = 7.0f;
		float SurgeResponseRate = 18.0f;
		float FrontRiserSurgeGainDeg = 18.0f;
		float SpeedBarSurgeGainDeg = 18.0f;
		float BrakeSurgeDampingDeg = 16.0f;
	} Airfoil;

	struct FLaunch
	{
		float FrontRiserInflationGain = 0.72f;
		float AirspeedInflationGain = 0.34f;
		float BrakeInflationPenalty = 0.20f;
		float InflationBuildRate = 1.8f;
		float InflationDecayRate = 1.2f;
		float LaunchLiftMultiplier = 3.4f;
		float TakeoffPressureThreshold = 0.58f;
		float FreeFlightSwitchHeightMeters = 8.0f;
		float FrontRiserGroundRunSpeedGainKmh = 12.0f;
	} Launch;

	struct FManeuvers
	{
		float DiveBuildRate = 2.1f;
		float DiveRecoveryRate = 1.12f;
		float DiveAirspeedGainKmh = 28.0f;
		float DivePitchGainDeg = 16.0f;
		float DiveSinkMultiplier = 1.28f;
		float DiveTurnRateBonus = 0.32f;
		float AcroBankLimitDeg = 112.0f;
		float AcroBankBonusDeg = 32.0f;
		float SpiralSinkMultiplier = 2.15f;
		float TumbleTriggerStall = 0.82f;
		float TumbleTriggerAsymmetry = 0.58f;
		float TumbleBuildRate = 2.4f;
		float TumbleRecoveryRate = 1.6f;
		float TumbleRollRateDegPerSecond = 520.0f;
		float TumblePitchLimitDeg = 74.0f;
		float TumbleHeadingRateDegPerSecond = 155.0f;
		float TumbleSinkMultiplier = 5.8f;
	} Maneuvers;

	static const FParaglideFlightTuning& Get()
	{
		static const FParaglideFlightTuning Tuning;
		return Tuning;
	}
};
