#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ParaglideHUD.generated.h"

UCLASS()
class PARAGLIDETHEWORLDUE_API AParaglideHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawPanel(float X, float Y, float Width, float Height, const FLinearColor& FillColor, const FLinearColor& BorderColor);
	void DrawTextLine(float X, float Y, const FString& Text, const FLinearColor& Color, float Scale = 1.0f);
	FString FlightPhaseToString() const;
	FString LandingRatingToString() const;
};
