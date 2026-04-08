#include "ParaglideHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "CanvasItem.h"
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
	const FString ScenarioName = FlightPawn->GetCurrentScenarioName();
	const FString ScenarioSummary = FlightPawn->GetCurrentScenarioSummary();
	const FString ScenarioLine = FormatScenarioHint(ScenarioName, ScenarioSummary);
	const FString ControlsLine = TEXT("A/F/J/;/Space fly | 1-5 scenarios | R reset | Tab HUD");
	const FString SinkLine = FString::Printf(
		TEXT("Sink base %.2f | turn %.2f | dive %.2f | spiral %.2f | tumble %.2f"),
		FlightState.Debug.BaseSinkMetersPerSecond,
		FlightState.Debug.InducedTurnSinkMetersPerSecond,
		FlightState.Debug.DiveSinkMetersPerSecond,
		FlightState.Debug.SpiralSinkMetersPerSecond,
		FlightState.Debug.TumbleSinkMetersPerSecond);
	const FString StateLine = FString::Printf(
		TEXT("Phase %s | Vario %.1f m/s | Air %.0f km/h | Ground %.0f km/h"),
		*FlightPhaseToString(),
		FlightState.VerticalSpeedMetersPerSecond,
		FlightState.AirspeedKmh,
		FlightState.GroundSpeedKmh);
	const FString EnergyLine = FString::Printf(
		TEXT("AGL %.0f m | Stall %.0f%% | Dive %.0f%% | Tumble %.0f%% | G %.2f"),
		FlightState.GroundClearanceMeters,
		FlightState.StallWarning * 100.0f,
		FlightState.DiveEnergy * 100.0f,
		FlightState.TumbleAmount * 100.0f,
		FlightState.LoadFactor);
	const FString LandingLine = FString::Printf(
		TEXT("Landing %s | Flare %.0f%% | Distance %s | Error %.0f deg"),
		*LandingRatingToString(),
		FlightState.FlareEffectiveness * 100.0f,
		*FormatDistanceMeters(FlightState.LandingZoneDistanceMeters),
		FMath::Max(FlightState.LandingApproachErrorDeg, 0.0f));

	DrawPanel(36.0f, 36.0f, 760.0f, 262.0f, FLinearColor(0.02f, 0.04f, 0.07f, 0.76f), FLinearColor(0.42f, 0.73f, 1.0f, 0.62f));
	DrawTextLine(58.0f, 56.0f, TEXT("Paraglide the World"), FLinearColor(0.95f, 0.98f, 1.0f, 1.0f), 1.35f);
	DrawTextLine(58.0f, 92.0f, ScenarioLine, FLinearColor(0.82f, 0.92f, 1.0f, 1.0f), 0.95f);
	DrawTextLine(58.0f, 128.0f, StateLine, FLinearColor(0.97f, 0.98f, 1.0f, 1.0f), 0.95f);
	DrawTextLine(58.0f, 160.0f, SinkLine, FLinearColor(0.87f, 0.91f, 0.98f, 1.0f), 0.88f);
	DrawTextLine(58.0f, 192.0f, EnergyLine, FLinearColor(0.87f, 0.93f, 1.0f, 1.0f), 0.88f);
	DrawTextLine(58.0f, 224.0f, LandingLine, FLinearColor(0.82f, 0.88f, 1.0f, 1.0f), 0.9f);

	DrawPanel(36.0f, Canvas->ClipY - 148.0f, 760.0f, 96.0f, FLinearColor(0.03f, 0.05f, 0.08f, 0.72f), FLinearColor(0.55f, 0.8f, 1.0f, 0.5f));
	DrawTextLine(58.0f, Canvas->ClipY - 120.0f, ControlsLine, FLinearColor(0.92f, 0.96f, 1.0f, 1.0f), 0.9f);
	DrawTextLine(58.0f, Canvas->ClipY - 88.0f, TEXT("Press Tab to hide this overlay once you are in the sim."), FLinearColor(0.7f, 0.83f, 1.0f, 1.0f), 0.85f);
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
