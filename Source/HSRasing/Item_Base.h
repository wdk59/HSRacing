// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HSRasingPawn.h"
#include "Item_Base.generated.h"

UCLASS()
class HSRASING_API AItem_Base : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem_Base();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		class USoundBase* PickupSound;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleDefaultsOnly, Category = Item)
		class USphereComponent* CollisionComp;
	
	UPROPERTY(VisibleDefaultsOnly, Category = Item)
		class UStaticMeshComponent* MeshComp;
	
	UPROPERTY(VisibleDefaultsOnly, Category = Item)
		class URotatingMovementComponent* RotationComp;

	UFUNCTION()
		void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	virtual void PlayerPickedUp(AHSRasingPawn* Player);


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
