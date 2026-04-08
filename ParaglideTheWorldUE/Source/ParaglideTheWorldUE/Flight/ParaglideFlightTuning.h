// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

struct FParaglideFlightTuning
{
	struct FBaselineWing
	{
		float TrimAirspeedKmh = 38.0f;
		float MinControllableAirspeedKmh = 18.0f;
		float MaxAirspeedKmh = 55.0f;
		float BaselineSinkMetersPerSecond = 1.02f;
		float SpeedToSinkCurveDivisor = 150.0f;
		float TurnRateMinAirspeedMetersPerSecond = 6.0f;
		float FlareMinAirspeedKmh = 9.0f;
	} BaselineWing;

	struct FControls
	{
		float SpeedBarAirspeedGainKmh = 10.5f;
		float SymmetricBrakeAirspeedLossKmh = 12.5f;
		float DeepBrakeAirspeedLossKmh = 5.5f;
		float ResponsivenessAirspeedGainKmh = 4.0f;
		float BrakeToRollDeg = 28.0f;
		float WeightShiftAuthorityDeg = 12.0f;
		float BrakeSinkLinearMetersPerSecond = 0.42f;
		float BrakeSinkQuadraticMetersPerSecond = 0.65f;
		float AirspeedResponseRate = 16.0f;
		float BankResponseRate = 30.0f;
		float CoordinationBankResponseGain = 34.0f;
	} Controls;

	struct FStability
	{
		float StallBrakeStart = 0.72f;
		float StallBrakeRange = 0.24f;
		float StallAirspeedReferenceKmh = 23.0f;
		float StallAirspeedRangeKmh = 8.0f;
		float SpeedBarStallRelief = 0.18f;
		float RecoveryAssistStallRelief = 0.65f;
		float CoordinationAuthorityGain = 0.42f;
		float StallAuthorityPenalty = 0.38f;
		float ControlAuthorityFloor = 0.62f;
		float ControlAuthorityCeiling = 1.16f;
		float GustRollMagnitudeDeg = 4.4f;
		float BankLimitDeg = 46.0f;
		float LoadFactorCosFloor = 0.74f;
		float InducedTurnSinkMultiplier = 0.95f;
		float StallTurnRatePenalty = 0.24f;
		float StallSinkMultiplier = 3.3f;
		float FlareStallPenalty = 0.3f;
	} Stability;

	struct FLiftSources
	{
		float FallbackRidgeTerrainWindowMeters = 4500.0f;
		float FallbackRidgeWindStartKmh = 8.0f;
		float FallbackRidgeWindRangeKmh = 16.0f;
		float FallbackRidgeWindCap = 1.35f;
		float RidgeWindStartKmh = 8.0f;
		float RidgeWindRangeKmh = 14.0f;
		float RidgeWindCap = 1.45f;
		float DefaultThermalDriftFactor = 0.28f;
		float ThermalCoreRadiusMultiplier = 0.35f;
		float ThermalCoreBaseFraction = 0.7f;
		float ThermalCoreCenteringBonus = 0.3f;
		float ThermalEdgeLiftFraction = 0.72f;
		float ThermalSinkRingRadiusMultiplier = 1.45f;
		float TurbulencePrimaryMix = 0.62f;
		float TurbulenceSecondaryMix = 0.38f;
		float TurbulenceVerticalGustScale = 0.78f;
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
		float SpeedBarPitchGainDeg = 3.8f;
		float SymmetricBrakePitchLossDeg = 8.4f;
		float StallPitchGainDeg = 8.8f;
		float FlarePitchGainDeg = 6.2f;
		float TurbulencePitchGainDeg = 1.4f;
		float MinPitchDeg = -22.0f;
		float MaxPitchDeg = 12.0f;
		float PitchResponseRate = 40.0f;
	} Attitude;

	static const FParaglideFlightTuning& Get()
	{
		static const FParaglideFlightTuning Tuning;
		return Tuning;
	}
};
