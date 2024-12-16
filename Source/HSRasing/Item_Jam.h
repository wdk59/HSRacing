// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item_Base.h"
#include "Item_Jam.generated.h"

/**
 * 
 */
UCLASS()
class HSRASING_API AItem_Jam : public AItem_Base
{
	GENERATED_BODY()
protected:

	virtual void PlayerPickedUp(AHSRasingPawn* Player) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
