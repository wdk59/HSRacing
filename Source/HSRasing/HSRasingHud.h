// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/HUD.h"
#include "HSRasingHud.generated.h"


UCLASS(config = Game)
class AHSRasingHud : public AHUD
{
	GENERATED_BODY()

public:
	AHSRasingHud();

	/** Font used to render the vehicle info */
	UPROPERTY()
	UFont* HUDFont;

	// Begin AHUD interface
	virtual void DrawHUD() override;
	// End AHUD interface
};
