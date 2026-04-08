// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ParaglideFlightTypes.h"

class FParaglideScenarioLibrary
{
public:
	static const TArray<FParaglideScenarioPreset>& GetScenarioPresets();
	static int32 FindScenarioIndexById(FName ScenarioId);
};
