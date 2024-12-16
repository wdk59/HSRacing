// Fill out your copyright notice in the Description page of Project Settings.


#include "Item_Oil.h"

void AItem_Oil::PlayerPickedUp(AHSRasingPawn* Player)
{
	Super::PlayerPickedUp(Player);
	//Player -> SlideCar();
}

void AItem_Oil::BeginPlay()
{
	Super::BeginPlay();
}