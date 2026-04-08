// Copyright Epic Games, Inc. All Rights Reserved.

#include "Flight/ParaglideScenarioLibrary.h"

#include "Flight/ParaglideFlightTuning.h"

namespace
{
	FVector2D OffsetByHeading(const FVector2D& OriginMeters, const float HeadingDeg, const float DistanceMeters)
	{
		const float HeadingRadians = FMath::DegreesToRadians(HeadingDeg);
		return FVector2D(
			OriginMeters.X + FMath::Sin(HeadingRadians) * DistanceMeters,
			OriginMeters.Y + FMath::Cos(HeadingRadians) * DistanceMeters);
	}

	FParaglideFlightState MakeBaseState(const FParaglideFlightSite& Site)
	{
		const FParaglideFlightTuning& Tuning = FParaglideFlightTuning::Get();

		FParaglideFlightState State;
		State.PositionMeters = Site.LaunchPositionMeters;
		State.TerrainHeightMeters = Site.TerrainHeightMeters;
		State.AltitudeMeters = Site.TerrainHeightMeters + Site.SpawnAglMeters;
		State.HeadingDeg = Site.PrevailingWindHeadingDeg;
		State.PitchDeg = Tuning.Attitude.TrimPitchDeg;
		State.AirspeedKmh = Tuning.BaselineWing.TrimAirspeedKmh;
		State.GroundSpeedKmh = Tuning.BaselineWing.TrimAirspeedKmh;
		State.VerticalSpeedMetersPerSecond = 0.1f;
		State.RidgeLiftMetersPerSecond = Site.BaseRidgeLiftMetersPerSecond;
		State.GroundClearanceMeters = Site.SpawnAglMeters;
		State.FlightPhase = EParaglideFlightPhase::Launch;
		return State;
	}

	FParaglideFlightState WithAgl(FParaglideFlightState State, const float TerrainHeightMeters, const float AglMeters)
	{
		State.TerrainHeightMeters = TerrainHeightMeters;
		State.AltitudeMeters = TerrainHeightMeters + AglMeters;
		State.GroundClearanceMeters = AglMeters;
		return State;
	}

	FParaglideFlightSite MakeLauterbrunnenSite()
	{
		FParaglideFlightSite Site;
		Site.Id = TEXT("lauterbrunnen");
		Site.Name = FText::FromString(TEXT("Lauterbrunnen"));
		Site.Region = FText::FromString(TEXT("Bernese Alps"));
		Site.Country = FText::FromString(TEXT("Switzerland"));
		Site.TerrainHeightMeters = 1485.0f;
		Site.SpawnAglMeters = 140.0f;
		Site.PrevailingWindHeadingDeg = 58.0f;
		Site.WindSpeedKmh = 19.0f;
		Site.BaseRidgeLiftMetersPerSecond = 1.4f;
		Site.RouteLengthKm = 16.0f;
		Site.LaunchPositionMeters = FVector2D::ZeroVector;
		Site.Ridge.bEnabled = true;
		Site.Ridge.AxisHeadingDeg = 148.0f;
		Site.Ridge.LengthMeters = 3300.0f;
		Site.Ridge.WindwardDepthMeters = 650.0f;
		Site.Ridge.LeeDepthMeters = 700.0f;
		Site.Ridge.PeakLiftMetersPerSecond = 2.4f;
		Site.Ridge.LeeSinkMetersPerSecond = 1.3f;
		Site.LandingZone.bEnabled = true;
		Site.LandingZone.PositionMeters = FVector2D(520.0f, 1650.0f);
		Site.LandingZone.HeadingDeg = 58.0f;
		Site.LandingZone.LengthMeters = 420.0f;
		Site.LandingZone.WidthMeters = 120.0f;

		FParaglideThermalSource ValleyThermal;
		ValleyThermal.Id = TEXT("valley-mouth");
		ValleyThermal.PositionMeters = FVector2D(900.0f, 1150.0f);
		ValleyThermal.RadiusMeters = 430.0f;
		ValleyThermal.CoreRadiusMeters = 150.0f;
		ValleyThermal.LiftMetersPerSecond = 2.2f;
		ValleyThermal.SinkRingMetersPerSecond = 0.5f;
		ValleyThermal.DriftFactor = 0.45f;
		Site.Thermals.Add(ValleyThermal);

		FParaglideThermalSource WaterfallThermal;
		WaterfallThermal.Id = TEXT("waterfall-ribbon");
		WaterfallThermal.PositionMeters = FVector2D(-450.0f, -420.0f);
		WaterfallThermal.RadiusMeters = 360.0f;
		WaterfallThermal.CoreRadiusMeters = 130.0f;
		WaterfallThermal.LiftMetersPerSecond = 1.8f;
		WaterfallThermal.SinkRingMetersPerSecond = 0.35f;
		WaterfallThermal.DriftFactor = 0.38f;
		Site.Thermals.Add(WaterfallThermal);

		return Site;
	}

	FParaglideFlightSite MakeRomeSite()
	{
		FParaglideFlightSite Site;
		Site.Id = TEXT("rome");
		Site.Name = FText::FromString(TEXT("Rome"));
		Site.Region = FText::FromString(TEXT("Monte Mario"));
		Site.Country = FText::FromString(TEXT("Italy"));
		Site.TerrainHeightMeters = 138.0f;
		Site.SpawnAglMeters = 150.0f;
		Site.PrevailingWindHeadingDeg = 248.0f;
		Site.WindSpeedKmh = 15.0f;
		Site.BaseRidgeLiftMetersPerSecond = 0.6f;
		Site.RouteLengthKm = 12.0f;
		Site.LaunchPositionMeters = FVector2D::ZeroVector;
		Site.Ridge.bEnabled = true;
		Site.Ridge.AxisHeadingDeg = 338.0f;
		Site.Ridge.LengthMeters = 1900.0f;
		Site.Ridge.WindwardDepthMeters = 360.0f;
		Site.Ridge.LeeDepthMeters = 450.0f;
		Site.Ridge.PeakLiftMetersPerSecond = 1.1f;
		Site.Ridge.LeeSinkMetersPerSecond = 0.8f;
		Site.LandingZone.bEnabled = true;
		Site.LandingZone.PositionMeters = FVector2D(320.0f, -1950.0f);
		Site.LandingZone.HeadingDeg = 238.0f;
		Site.LandingZone.LengthMeters = 360.0f;
		Site.LandingZone.WidthMeters = 100.0f;
		return Site;
	}

	FParaglideFlightSite MakeIstanbulSite()
	{
		FParaglideFlightSite Site;
		Site.Id = TEXT("istanbul");
		Site.Name = FText::FromString(TEXT("Istanbul"));
		Site.Region = FText::FromString(TEXT("Camlica Hill"));
		Site.Country = FText::FromString(TEXT("Turkey"));
		Site.TerrainHeightMeters = 262.0f;
		Site.SpawnAglMeters = 170.0f;
		Site.PrevailingWindHeadingDeg = 32.0f;
		Site.WindSpeedKmh = 18.0f;
		Site.BaseRidgeLiftMetersPerSecond = 0.8f;
		Site.RouteLengthKm = 14.0f;
		Site.LaunchPositionMeters = FVector2D::ZeroVector;
		Site.Ridge.bEnabled = true;
		Site.Ridge.AxisHeadingDeg = 318.0f;
		Site.Ridge.LengthMeters = 2200.0f;
		Site.Ridge.WindwardDepthMeters = 420.0f;
		Site.Ridge.LeeDepthMeters = 520.0f;
		Site.Ridge.PeakLiftMetersPerSecond = 1.5f;
		Site.Ridge.LeeSinkMetersPerSecond = 0.95f;
		Site.LandingZone.bEnabled = true;
		Site.LandingZone.PositionMeters = FVector2D(-2100.0f, -1100.0f);
		Site.LandingZone.HeadingDeg = 22.0f;
		Site.LandingZone.LengthMeters = 380.0f;
		Site.LandingZone.WidthMeters = 120.0f;
		return Site;
	}

	FParaglideScenarioPreset MakeRidgePassScenario()
	{
		FParaglideScenarioPreset Scenario;
		Scenario.Id = TEXT("ridge-pass");
		Scenario.Name = FText::FromString(TEXT("Ridge Pass"));
		Scenario.Summary = FText::FromString(TEXT("Start in the windward band and read progressive ridge lift against lee sink."));
		Scenario.Setup = FText::FromString(TEXT("Medium wind, stable entry speed, and enough clearance to work the face."));
		Scenario.RecommendedInputs = FText::FromString(TEXT("Fly a clean line first, then add light brake and weight shift near the ridge."));
		Scenario.KeyOutputs = {TEXT("ridge lift"), TEXT("net vario"), TEXT("turn authority")};
		Scenario.Site = MakeLauterbrunnenSite();
		Scenario.Atmosphere.WindHeadingDeg = Scenario.Site.PrevailingWindHeadingDeg;
		Scenario.Atmosphere.WindSpeedKmh = FMath::Max(Scenario.Site.WindSpeedKmh, 24.0f);
		Scenario.Atmosphere.Turbulence = 0.1f;

		const float WindwardNormalHeadingDeg = Scenario.Site.Ridge.AxisHeadingDeg - 90.0f;
		FParaglideFlightState State = MakeBaseState(Scenario.Site);
		State = WithAgl(State, Scenario.Site.TerrainHeightMeters, 135.0f);
		const FVector2D AlongPosition = OffsetByHeading(
			Scenario.Site.Ridge.PositionMeters,
			Scenario.Site.Ridge.AxisHeadingDeg,
			-Scenario.Site.Ridge.LengthMeters * 0.14f);
		State.PositionMeters = OffsetByHeading(
			AlongPosition,
			WindwardNormalHeadingDeg,
			Scenario.Site.Ridge.WindwardDepthMeters * 0.42f);
		State.HeadingDeg = Scenario.Site.Ridge.AxisHeadingDeg;
		State.AirspeedKmh = 39.0f;
		State.GroundSpeedKmh = 39.0f;
		State.VerticalSpeedMetersPerSecond = 0.4f;
		State.FlightPhase = EParaglideFlightPhase::Soaring;
		Scenario.InitialFlightState = State;
		return Scenario;
	}

	FParaglideScenarioPreset MakeThermalClimbScenario()
	{
		FParaglideScenarioPreset Scenario;
		Scenario.Id = TEXT("thermal-climb");
		Scenario.Name = FText::FromString(TEXT("Thermal Climb"));
		Scenario.Summary = FText::FromString(TEXT("Enter a seeded thermal edge and feel how centering changes climb quality."));
		Scenario.Setup = FText::FromString(TEXT("Light-to-medium wind, drifting core, and enough height to circle without rushing."));
		Scenario.RecommendedInputs = FText::FromString(TEXT("Use gentle coordinated turns and watch the vario improve as you center."));
		Scenario.KeyOutputs = {TEXT("thermal lift"), TEXT("sink ring"), TEXT("climb stability")};
		Scenario.Site = MakeLauterbrunnenSite();
		Scenario.Atmosphere.WindHeadingDeg = Scenario.Site.PrevailingWindHeadingDeg;
		Scenario.Atmosphere.WindSpeedKmh = FMath::Max(Scenario.Site.WindSpeedKmh - 3.0f, 12.0f);
		Scenario.Atmosphere.Turbulence = 0.08f;

		FParaglideFlightState State = MakeBaseState(Scenario.Site);
		State = WithAgl(State, Scenario.Site.TerrainHeightMeters, 180.0f);
		const FParaglideThermalSource& Thermal = Scenario.Site.Thermals[0];
		State.PositionMeters = OffsetByHeading(Thermal.PositionMeters, Scenario.Site.PrevailingWindHeadingDeg + 90.0f, Thermal.RadiusMeters * 0.58f);
		State.HeadingDeg = Scenario.Site.PrevailingWindHeadingDeg + 28.0f;
		State.AirspeedKmh = 37.0f;
		State.GroundSpeedKmh = 34.0f;
		State.VerticalSpeedMetersPerSecond = 0.2f;
		State.FlightPhase = EParaglideFlightPhase::Soaring;
		Scenario.InitialFlightState = State;
		return Scenario;
	}

	FParaglideScenarioPreset MakeGlideTransitionScenario()
	{
		FParaglideScenarioPreset Scenario;
		Scenario.Id = TEXT("glide-transition");
		Scenario.Name = FText::FromString(TEXT("Glide Transition"));
		Scenario.Summary = FText::FromString(TEXT("Compare trim, brake, and speed-bar glide tradeoffs in neutral air."));
		Scenario.Setup = FText::FromString(TEXT("High enough to experiment, low turbulence, and just enough wind to feel penetration."));
		Scenario.RecommendedInputs = FText::FromString(TEXT("Compare trim, deep brake, and speed bar without changing the line too much."));
		Scenario.KeyOutputs = {TEXT("airspeed range"), TEXT("glide trend"), TEXT("stall warning")};
		Scenario.Site = MakeRomeSite();
		Scenario.Atmosphere.WindHeadingDeg = Scenario.Site.PrevailingWindHeadingDeg;
		Scenario.Atmosphere.WindSpeedKmh = 6.0f;
		Scenario.Atmosphere.Turbulence = 0.04f;

		FParaglideFlightState State = MakeBaseState(Scenario.Site);
		State = WithAgl(State, Scenario.Site.TerrainHeightMeters, 180.0f);
		State.HeadingDeg = Scenario.Site.PrevailingWindHeadingDeg;
		State.AirspeedKmh = 38.0f;
		State.GroundSpeedKmh = 36.0f;
		State.VerticalSpeedMetersPerSecond = -0.8f;
		State.FlightPhase = EParaglideFlightPhase::Soaring;
		Scenario.InitialFlightState = State;
		return Scenario;
	}

	FParaglideScenarioPreset MakeApproachScenario()
	{
		FParaglideScenarioPreset Scenario;
		Scenario.Id = TEXT("approach");
		Scenario.Name = FText::FromString(TEXT("Approach"));
		Scenario.Summary = FText::FromString(TEXT("Short final setup to read alignment, energy, and landing-zone guidance."));
		Scenario.Setup = FText::FromString(TEXT("Low altitude, mild headwind, and a landing line that rewards stable alignment."));
		Scenario.RecommendedInputs = FText::FromString(TEXT("Stay coordinated, read the landing error, and avoid big late inputs."));
		Scenario.KeyOutputs = {TEXT("groundspeed"), TEXT("vertical speed"), TEXT("approach error")};
		Scenario.Site = MakeIstanbulSite();
		Scenario.Atmosphere.WindHeadingDeg = Scenario.Site.PrevailingWindHeadingDeg;
		Scenario.Atmosphere.WindSpeedKmh = 14.0f;
		Scenario.Atmosphere.Turbulence = 0.06f;

		FParaglideFlightState State = MakeBaseState(Scenario.Site);
		State = WithAgl(State, Scenario.Site.TerrainHeightMeters, 18.0f);
		const FVector2D BehindApproach = OffsetByHeading(Scenario.Site.LandingZone.PositionMeters, Scenario.Site.LandingZone.HeadingDeg + 180.0f, 240.0f);
		State.PositionMeters = OffsetByHeading(BehindApproach, Scenario.Site.LandingZone.HeadingDeg + 90.0f, 18.0f);
		State.HeadingDeg = Scenario.Site.LandingZone.HeadingDeg - 8.0f;
		State.AirspeedKmh = 31.0f;
		State.GroundSpeedKmh = 25.0f;
		State.VerticalSpeedMetersPerSecond = -1.2f;
		State.FlightPhase = EParaglideFlightPhase::Approach;
		Scenario.InitialFlightState = State;
		return Scenario;
	}

	FParaglideScenarioPreset MakeFlareScenario()
	{
		FParaglideScenarioPreset Scenario;
		Scenario.Id = TEXT("flare");
		Scenario.Name = FText::FromString(TEXT("Flare"));
		Scenario.Summary = FText::FromString(TEXT("Final meters prototype for comparing flare timing."));
		Scenario.Setup = FText::FromString(TEXT("Very low final, committed landing line, and enough speed for timing to matter."));
		Scenario.RecommendedInputs = FText::FromString(TEXT("Use symmetric brake timing near the ground and watch flare effectiveness."));
		Scenario.KeyOutputs = {TEXT("flare effectiveness"), TEXT("touchdown speed"), TEXT("landing rating")};
		Scenario.Site = MakeIstanbulSite();
		Scenario.Atmosphere.WindHeadingDeg = Scenario.Site.PrevailingWindHeadingDeg;
		Scenario.Atmosphere.WindSpeedKmh = 10.0f;
		Scenario.Atmosphere.Turbulence = 0.04f;

		FParaglideFlightState State = MakeBaseState(Scenario.Site);
		State = WithAgl(State, Scenario.Site.TerrainHeightMeters, 2.8f);
		State.PositionMeters = OffsetByHeading(Scenario.Site.LandingZone.PositionMeters, Scenario.Site.LandingZone.HeadingDeg + 180.0f, 34.0f);
		State.HeadingDeg = Scenario.Site.LandingZone.HeadingDeg;
		State.AirspeedKmh = 27.0f;
		State.GroundSpeedKmh = 24.0f;
		State.VerticalSpeedMetersPerSecond = -1.3f;
		State.FlightPhase = EParaglideFlightPhase::Approach;
		Scenario.InitialFlightState = State;
		return Scenario;
	}
}

const TArray<FParaglideScenarioPreset>& FParaglideScenarioLibrary::GetScenarioPresets()
{
	static const TArray<FParaglideScenarioPreset> Presets = {
		MakeRidgePassScenario(),
		MakeThermalClimbScenario(),
		MakeGlideTransitionScenario(),
		MakeApproachScenario(),
		MakeFlareScenario(),
	};

	return Presets;
}

int32 FParaglideScenarioLibrary::FindScenarioIndexById(const FName ScenarioId)
{
	const TArray<FParaglideScenarioPreset>& Presets = GetScenarioPresets();

	for (int32 Index = 0; Index < Presets.Num(); ++Index)
	{
		if (Presets[Index].Id == ScenarioId)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}
