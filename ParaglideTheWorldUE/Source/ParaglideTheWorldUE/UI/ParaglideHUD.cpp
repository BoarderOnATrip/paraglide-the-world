#include "ParaglideHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "CanvasItem.h"
#include "ParaglideTheWorldUE/Flight/ParaglideFlightComponent.h"
#include "ParaglideTheWorldUE/Player/ParaglideFlightPawn.h"
#include "ParaglideTheWorldUE/Player/ParaglidePlayerController.h"

namespace
{
	FString FormatScenarioHint(const FString& ScenarioName, const FString& ScenarioSummary)
	{
		if (ScenarioSummary.IsEmpty())
		{
			return ScenarioName;
		}

		return FString::Printf(TEXT("%s - %s"), *ScenarioName, *ScenarioSummary);
	}

	FString FormatDistanceMeters(float DistanceMeters)
	{
		return DistanceMeters >= 0.0f ? FString::Printf(TEXT("%.0f m"), DistanceMeters) : FString(TEXT("--"));
	}
}

void AParaglideHUD::DrawHUD()
{
	Super::DrawHUD();

	AParaglidePlayerController* ParaglideController = Cast<AParaglidePlayerController>(PlayerOwner);
	if (!Canvas || !ParaglideController || !ParaglideController->IsParaglideHUDVisible())
	{
		return;
	}

	AParaglideFlightPawn* FlightPawn = Cast<AParaglideFlightPawn>(ParaglideController->GetPawn());
	if (!FlightPawn)
	{
		DrawPanel(36.0f, 36.0f, 520.0f, 170.0f, FLinearColor(0.03f, 0.05f, 0.08f, 0.72f), FLinearColor(0.45f, 0.75f, 1.0f, 0.55f));
		DrawTextLine(56.0f, 58.0f, TEXT("Paraglide prototype shell"), FLinearColor::White, 1.15f);
		DrawTextLine(56.0f, 94.0f, TEXT("Waiting for pawn possession."), FLinearColor(0.85f, 0.9f, 1.0f, 1.0f), 1.0f);
		return;
	}

	const FParaglideFlightState FlightState = FlightPawn->GetFlightState();
	const UParaglideFlightComponent* FlightComponent = FlightPawn->GetFlightComponent();
	const FParaglideControlState ControlState = FlightComponent ? FlightComponent->GetControlState() : FParaglideControlState();
	const FString ScenarioName = FlightPawn->GetCurrentScenarioName();
	const FString ScenarioSummary = FlightPawn->GetCurrentScenarioSummary();
	const FString ScenarioLine = FormatScenarioHint(ScenarioName, ScenarioSummary);
	const bool bLaunchControls = FlightState.FlightPhase == EParaglideFlightPhase::Launch;
	const FString ControlsLine = bLaunchControls
		? TEXT("LShift/RShift A-risers | S/L rear risers | D+F / J+K brakes | G/H or Space speed bar")
		: TEXT("LShift/RShift weight shift | S/L C-steer | D+F / J+K brakes | G/H or Space speed bar");
	const FString InputLine = FString::Printf(
		TEXT("Front %.0f / %.0f | Rear %.0f / %.0f | Brake %.0f / %.0f | Shift %.0f%% | Bar %.0f%%"),
		ControlState.LeftFrontRiserTravel * 100.0f,
		ControlState.RightFrontRiserTravel * 100.0f,
		ControlState.LeftRearRiserTravel * 100.0f,
		ControlState.RightRearRiserTravel * 100.0f,
		ControlState.LeftBrakeTravel * 100.0f,
		ControlState.RightBrakeTravel * 100.0f,
		ControlState.WeightShiftPosition * 100.0f,
		ControlState.SpeedBarTravel * 100.0f);
	const FString HintLine = bLaunchControls
		? TEXT("Launch phase: Left Shift and Right Shift are A-risers. Once the wing is flying, they become harness shift.")
		: TEXT("G/H are staged bar, Space is full bar, 6 is the low-pass line, and 7 is the acro setup.");
	const FString LiftLine = FString::Printf(
		TEXT("Lift ridge %.2f | thermal %.2f | gust %.2f | ground %.2f | sink %.2f"),
		FlightState.RidgeLiftMetersPerSecond,
		FlightState.ThermalLiftMetersPerSecond,
		FlightState.Debug.TurbulenceLiftMetersPerSecond,
		FlightState.Debug.GroundEffectLiftMetersPerSecond,
		FlightState.Debug.TotalSinkMetersPerSecond);
	const FString StateLine = FString::Printf(
		TEXT("Phase %s | Vario %.1f m/s | Air %.0f km/h | Ground %.0f km/h"),
		*FlightPhaseToString(),
		FlightState.VerticalSpeedMetersPerSecond,
		FlightState.AirspeedKmh,
		FlightState.GroundSpeedKmh);
	const FString EnergyLine = FString::Printf(
		TEXT("AGL %.0f m | AoA %.1f deg | Pressure %.0f%% | Inflation %.0f%% | G %.2f | GndFx %.0f%%"),
		FlightState.GroundClearanceMeters,
		FlightState.AngleOfAttackDeg,
		FlightState.CanopyPressure * 100.0f,
		FlightState.WingInflation * 100.0f,
		FlightState.LoadFactor,
		FlightState.GroundEffectStrength * 100.0f);
	const FString RiskLine = FString::Printf(
		TEXT("Stall %.0f%% | Dive %.0f%% | Acro %.0f%% | Tumble %.0f%% | Collapse L/R %.0f / %.0f"),
		FlightState.StallWarning * 100.0f,
		FlightState.DiveEnergy * 100.0f,
		FlightState.ManeuverEnergy * 100.0f,
		FlightState.TumbleAmount * 100.0f,
		FlightState.LeftCollapseAmount * 100.0f,
		FlightState.RightCollapseAmount * 100.0f);
	const FString SurgeLine = FString::Printf(
		TEXT("Surge %.0f deg | Load %.2f | AGL %.0f m | AoA %.1f deg | Pressure %.0f%% | Inflation %.0f%%"),
		FlightState.WingSurgeDeg,
		FlightState.LoadFactor,
		FlightState.GroundClearanceMeters,
		FlightState.AngleOfAttackDeg,
		FlightState.CanopyPressure * 100.0f,
		FlightState.WingInflation * 100.0f);
	const FString LandingLine = FString::Printf(
		TEXT("Landing %s | Flare %.0f%% | Distance %s | Error %.0f deg"),
		*LandingRatingToString(),
		FlightState.FlareEffectiveness * 100.0f,
		*FormatDistanceMeters(FlightState.LandingZoneDistanceMeters),
		FMath::Max(FlightState.LandingApproachErrorDeg, 0.0f));

	DrawPanel(36.0f, 36.0f, 780.0f, 326.0f, FLinearColor(0.02f, 0.04f, 0.07f, 0.76f), FLinearColor(0.42f, 0.73f, 1.0f, 0.62f));
	DrawTextLine(58.0f, 56.0f, TEXT("Paraglide the World"), FLinearColor(0.95f, 0.98f, 1.0f, 1.0f), 1.35f);
	DrawTextLine(58.0f, 92.0f, ScenarioLine, FLinearColor(0.82f, 0.92f, 1.0f, 1.0f), 0.95f);
	DrawTextLine(58.0f, 128.0f, StateLine, FLinearColor(0.97f, 0.98f, 1.0f, 1.0f), 0.95f);
	DrawTextLine(58.0f, 160.0f, LiftLine, FLinearColor(0.87f, 0.91f, 0.98f, 1.0f), 0.88f);
	DrawTextLine(58.0f, 192.0f, EnergyLine, FLinearColor(0.87f, 0.93f, 1.0f, 1.0f), 0.88f);
	DrawTextLine(58.0f, 224.0f, RiskLine, FLinearColor(0.86f, 0.91f, 1.0f, 1.0f), 0.86f);
	DrawTextLine(58.0f, 256.0f, SurgeLine, FLinearColor(0.86f, 0.91f, 1.0f, 1.0f), 0.84f);
	DrawTextLine(58.0f, 288.0f, LandingLine, FLinearColor(0.82f, 0.88f, 1.0f, 1.0f), 0.9f);

	DrawPanel(36.0f, Canvas->ClipY - 202.0f, 780.0f, 150.0f, FLinearColor(0.03f, 0.05f, 0.08f, 0.72f), FLinearColor(0.55f, 0.8f, 1.0f, 0.5f));
	DrawTextLine(58.0f, Canvas->ClipY - 174.0f, ControlsLine, FLinearColor(0.92f, 0.96f, 1.0f, 1.0f), 0.9f);
	DrawTextLine(58.0f, Canvas->ClipY - 142.0f, InputLine, FLinearColor(0.84f, 0.93f, 1.0f, 1.0f), 0.84f);
	DrawTextLine(58.0f, Canvas->ClipY - 110.0f, HintLine, FLinearColor(0.82f, 0.9f, 1.0f, 1.0f), 0.81f);
	DrawTextLine(58.0f, Canvas->ClipY - 78.0f, TEXT("1-7 scenarios | R reset | Tab HUD"), FLinearColor(0.7f, 0.83f, 1.0f, 1.0f), 0.85f);
}

void AParaglideHUD::DrawPanel(float X, float Y, float Width, float Height, const FLinearColor& FillColor, const FLinearColor& BorderColor)
{
	if (!Canvas)
	{
		return;
	}

	FCanvasTileItem Tile(FVector2D(X, Y), FVector2D(Width, Height), FillColor);
	Tile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Tile);

	const float BorderThickness = 2.0f;
	FCanvasTileItem TopBorder(FVector2D(X, Y), FVector2D(Width, BorderThickness), BorderColor);
	TopBorder.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TopBorder);

	FCanvasTileItem BottomBorder(FVector2D(X, Y + Height - BorderThickness), FVector2D(Width, BorderThickness), BorderColor);
	BottomBorder.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(BottomBorder);

	FCanvasTileItem LeftBorder(FVector2D(X, Y), FVector2D(BorderThickness, Height), BorderColor);
	LeftBorder.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(LeftBorder);

	FCanvasTileItem RightBorder(FVector2D(X + Width - BorderThickness, Y), FVector2D(BorderThickness, Height), BorderColor);
	RightBorder.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(RightBorder);
}

void AParaglideHUD::DrawTextLine(float X, float Y, const FString& Text, const FLinearColor& Color, float Scale)
{
	if (!Canvas || !GEngine)
	{
		return;
	}

	UFont* Font = GEngine->GetSmallFont();
	if (!Font)
	{
		return;
	}

	FCanvasTextItem TextItem(FVector2D(X, Y), FText::FromString(Text), Font, Color);
	TextItem.Scale = FVector2D(Scale, Scale);
	TextItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(TextItem);
}

FString AParaglideHUD::FlightPhaseToString() const
{
	if (const AParaglidePlayerController* ParaglideController = Cast<AParaglidePlayerController>(PlayerOwner))
	{
		if (const AParaglideFlightPawn* FlightPawn = Cast<AParaglideFlightPawn>(ParaglideController->GetPawn()))
		{
			switch (FlightPawn->GetFlightState().FlightPhase)
			{
			case EParaglideFlightPhase::Launch:
				return TEXT("Launch");
			case EParaglideFlightPhase::Soaring:
				return TEXT("Soaring");
			case EParaglideFlightPhase::Approach:
				return TEXT("Approach");
			case EParaglideFlightPhase::Flare:
				return TEXT("Flare");
			case EParaglideFlightPhase::Landed:
				return TEXT("Landed");
			case EParaglideFlightPhase::Crashed:
				return TEXT("Crashed");
			}
		}
	}

	return TEXT("Unknown");
}

FString AParaglideHUD::LandingRatingToString() const
{
	if (const AParaglidePlayerController* ParaglideController = Cast<AParaglidePlayerController>(PlayerOwner))
	{
		if (const AParaglideFlightPawn* FlightPawn = Cast<AParaglideFlightPawn>(ParaglideController->GetPawn()))
		{
			switch (FlightPawn->GetFlightState().LandingRating)
			{
			case EParaglideLandingRating::None:
				return TEXT("None");
			case EParaglideLandingRating::Smooth:
				return TEXT("Smooth");
			case EParaglideLandingRating::Firm:
				return TEXT("Firm");
			case EParaglideLandingRating::Hard:
				return TEXT("Hard");
			case EParaglideLandingRating::Crash:
				return TEXT("Crash");
			}
		}
	}

	return TEXT("None");
}
