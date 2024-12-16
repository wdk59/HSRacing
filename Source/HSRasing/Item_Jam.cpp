// Fill out your copyright notice in the Description page of Project Settings.


#include "Item_Jam.h"
#include "HSRasingPawn.h"

void AItem_Jam::PlayerPickedUp(AHSRasingPawn* Player)
{
	Super::PlayerPickedUp(Player);
	Player->AddBoosterGauge();
}

void AItem_Jam::BeginPlay()
{
	Super::BeginPlay();
}