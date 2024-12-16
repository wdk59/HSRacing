// Copyright Epic Games, Inc. All Rights Reserved.

#include "HSRasingGameMode.h"
#include "HSRasingPawn.h"
#include "HSRasingHud.h"

AHSRasingGameMode::AHSRasingGameMode()
{
	DefaultPawnClass = AHSRasingPawn::StaticClass();
	//HUDClass = AHSRasingHud::StaticClass();
}
