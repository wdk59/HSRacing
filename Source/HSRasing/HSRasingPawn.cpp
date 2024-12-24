// Copyright Epic Games, Inc. All Rights Reserved.

#include "HSRasingPawn.h"
#include "HSRasingWheelFront.h"
#include "HSRasingWheelRear.h"
#include "HSRasingHud.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "WheeledVehicleMovementComponent4W.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/TextRenderComponent.h"
#include "Materials/Material.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h" 
#include "NiagaraComponent.h"
#include "GameFramework/Controller.h"

#ifndef HMD_MODULE_INCLUDED
#define HMD_MODULE_INCLUDED 0
#endif

// Needed for VR Headset
#if HMD_MODULE_INCLUDED
#include "IXRTrackingSystem.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#endif // HMD_MODULE_INCLUDED

const FName AHSRasingPawn::LookUpBinding("LookUp");
const FName AHSRasingPawn::LookRightBinding("LookRight");

#define LOCTEXT_NAMESPACE "VehiclePawn"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

AHSRasingPawn::AHSRasingPawn()
{
	// Car mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CarMesh(TEXT("/Game/Vehicle/Sedan/Sedan_SkelMesh.Sedan_SkelMesh"));
	GetMesh()->SetSkeletalMesh(CarMesh.Object);

	static ConstructorHelpers::FClassFinder<UObject> AnimBPClass(TEXT("/Game/Vehicle/Sedan/Sedan_AnimBP"));
	GetMesh()->SetAnimInstanceClass(AnimBPClass.Class);
	
	// Simulation
	UWheeledVehicleMovementComponent4W* Vehicle4W = CastChecked<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());

	check(Vehicle4W->WheelSetups.Num() == 4);

	Vehicle4W->WheelSetups[0].WheelClass = UHSRasingWheelFront::StaticClass();
	Vehicle4W->WheelSetups[0].BoneName = FName("Wheel_Front_Left");
	Vehicle4W->WheelSetups[0].AdditionalOffset = FVector(0.f, -12.f, 0.f);

	Vehicle4W->WheelSetups[1].WheelClass = UHSRasingWheelFront::StaticClass();
	Vehicle4W->WheelSetups[1].BoneName = FName("Wheel_Front_Right");
	Vehicle4W->WheelSetups[1].AdditionalOffset = FVector(0.f, 12.f, 0.f);

	Vehicle4W->WheelSetups[2].WheelClass = UHSRasingWheelRear::StaticClass();
	Vehicle4W->WheelSetups[2].BoneName = FName("Wheel_Rear_Left");
	Vehicle4W->WheelSetups[2].AdditionalOffset = FVector(0.f, -12.f, 0.f);

	Vehicle4W->WheelSetups[3].WheelClass = UHSRasingWheelRear::StaticClass();
	Vehicle4W->WheelSetups[3].BoneName = FName("Wheel_Rear_Right");
	Vehicle4W->WheelSetups[3].AdditionalOffset = FVector(0.f, 12.f, 0.f);

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->TargetOffset = FVector(0.f, 0.f, 200.f);
	SpringArm->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 600.0f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 7.f;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 90.f;

	// Create In-Car camera component 
	InternalCameraOrigin = FVector(0.0f, -40.0f, 120.0f);

	InternalCameraBase = CreateDefaultSubobject<USceneComponent>(TEXT("InternalCameraBase"));
	InternalCameraBase->SetRelativeLocation(InternalCameraOrigin);
	InternalCameraBase->SetupAttachment(GetMesh());

	InternalCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("InternalCamera"));
	InternalCamera->bUsePawnControlRotation = false;
	InternalCamera->FieldOfView = 90.f;
	InternalCamera->SetupAttachment(InternalCameraBase);

	//Setup TextRenderMaterial
	static ConstructorHelpers::FObjectFinder<UMaterial> TextMaterial(TEXT("Material'/Engine/EngineMaterials/AntiAliasedTextMaterialTranslucent.AntiAliasedTextMaterialTranslucent'"));
	
	UMaterialInterface* Material = TextMaterial.Object;

	// Create text render component for in car speed display
	InCarSpeed = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarSpeed"));
	InCarSpeed->SetTextMaterial(Material);
	InCarSpeed->SetRelativeLocation(FVector(70.0f, -75.0f, 99.0f));
	InCarSpeed->SetRelativeRotation(FRotator(18.0f, 180.0f, 0.0f));
	InCarSpeed->SetupAttachment(GetMesh());
	InCarSpeed->SetRelativeScale3D(FVector(1.0f, 0.4f, 0.4f));

	// Create text render component for in car gear display
	InCarGear = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarGear"));
	InCarGear->SetTextMaterial(Material);
	InCarGear->SetRelativeLocation(FVector(66.0f, -9.0f, 95.0f));	
	InCarGear->SetRelativeRotation(FRotator(25.0f, 180.0f,0.0f));
	InCarGear->SetRelativeScale3D(FVector(1.0f, 0.4f, 0.4f));
	InCarGear->SetupAttachment(GetMesh());
	
	// Colors for the incar gear display. One for normal one for reverse
	GearDisplayReverseColor = FColor(255, 0, 0, 255);
	GearDisplayColor = FColor(255, 255, 255, 255);

	// Colors for the in-car gear display. One for normal one for reverse
	GearDisplayReverseColor = FColor(255, 0, 0, 255);
	GearDisplayColor = FColor(255, 255, 255, 255);

	bInReverseGear = false;

	SpeedLineBody = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Speed Line Body"));
	ConstructorHelpers::FObjectFinder<UNiagaraSystem> SpeedLineBodyAsset(TEXT("NiagaraSystem'/Game/Blueprints/Effect/NS_SpeedLines.NS_SpeedLines'"));
	if (SpeedLineBodyAsset.Succeeded()) {
		SpeedLineBody->SetAsset(SpeedLineBodyAsset.Object);
		SpeedLineBody->SetupAttachment(GetMesh());
	}
	
	SpeedLineBody->SetRelativeLocation(FVector(300.0f, 0.0f, 100.0f));
	SpeedLineBody->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
	SpeedLineBody->SetRelativeScale3D(FVector(5.0f, 5.0f, 5.0f));

	BoostLineBody = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Boost Line Body"));
	ConstructorHelpers::FObjectFinder<UNiagaraSystem> BoostLineBodyAsset(TEXT("NiagaraSystem'/Game/Blueprints/Effect/NS_BoostLines.NS_BoostLines'"));
	if (BoostLineBodyAsset.Succeeded()) {
		BoostLineBody->SetAsset(BoostLineBodyAsset.Object);
		BoostLineBody->SetupAttachment(GetMesh());
	}

	BoostLineBody->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	BoostLineBody->SetRelativeRotation(FRotator(0.0f,0.0f, 0.0f));
	BoostLineBody->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));


	EnergyLineBody = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Energy Line Body"));
	ConstructorHelpers::FObjectFinder<UNiagaraSystem> EnergyLineBodyAsset(TEXT("NiagaraSystem'/Game/Assets/RocketThrusterExhaustFX/FX/NS_RocketExhaust_Energy.NS_RocketExhaust_Energy'"));
	if (EnergyLineBodyAsset.Succeeded()) {
		EnergyLineBody->SetAsset(EnergyLineBodyAsset.Object);
		EnergyLineBody->SetupAttachment(GetMesh());
	}

	EnergyLineBody->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	EnergyLineBody->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	EnergyLineBody->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
}

void AHSRasingPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AHSRasingPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHSRasingPawn::MoveRight);
	PlayerInputComponent->BindAxis("LookUp");
	PlayerInputComponent->BindAxis("LookRight");

	PlayerInputComponent->BindAction("Handbrake", IE_Pressed, this, &AHSRasingPawn::OnHandbrakePressed);
	PlayerInputComponent->BindAction("Handbrake", IE_Released, this, &AHSRasingPawn::OnHandbrakeReleased);
	PlayerInputComponent->BindAction("Boost", IE_Pressed, this, &AHSRasingPawn::OnBoostSwitch);
	PlayerInputComponent->BindAction("Boost", IE_Released, this, &AHSRasingPawn::OnBoostSwitch);

	PlayerInputComponent->BindAction("SwitchCamera", IE_Pressed, this, &AHSRasingPawn::OnToggleCamera);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AHSRasingPawn::OnResetVR); 
}

void AHSRasingPawn::MoveForward(float Val)
{
	GetVehicleMovementComponent()->SetThrottleInput(Val);
}

void AHSRasingPawn::MoveRight(float Val)
{
	GetVehicleMovementComponent()->SetSteeringInput(Val);
}

void AHSRasingPawn::OnHandbrakePressed()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(true);
}

void AHSRasingPawn::OnHandbrakeReleased()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(false);
}

void AHSRasingPawn::OnBoostSwitch()
{
	isBoost = !isBoost;
	if (isBoost) {
		SpeedLineBody->Deactivate();
		BoostLineBody->Activate();
		EnergyLineBody->Activate();

		UWorld* world = GetWorld();
		if (world) {
			if (BoostSound) {
				UGameplayStatics::SpawnSoundAtLocation(world, BoostSound, GetActorLocation());
			}
		}
	}
	else {
		BoostLineBody->Deactivate();
		EnergyLineBody->Deactivate();
	}
}

void AHSRasingPawn::AddBoosterGauge() {
	boostValue += boostGaningValue;
}


void AHSRasingPawn::OnToggleCamera()
{
	EnableIncarView(!bInCarCameraActive);
}

void AHSRasingPawn::EnableIncarView(const bool bState, const bool bForce)
{
	if ((bState != bInCarCameraActive) || ( bForce == true ))
	{
		bInCarCameraActive = bState;
		
		if (bState == true)
		{
			OnResetVR();
			Camera->Deactivate();
			InternalCamera->Activate();
		}
		else
		{
			InternalCamera->Deactivate();
			Camera->Activate();
		}
		
		InCarSpeed->SetVisibility(bInCarCameraActive);
		InCarGear->SetVisibility(bInCarCameraActive);
	}
}


void AHSRasingPawn::Tick(float Delta)
{
	Super::Tick(Delta);

	// Setup the flag to say we are in reverse gear
	bInReverseGear = GetVehicleMovement()->GetCurrentGear() < 0;
	
	// Update the strings used in the hud (incar and onscreen)
	UpdateHUDStrings();

	// Set the string in the incar hud
	SetupInCarHUD();

	bool bHMDActive = false;
#if HMD_MODULE_INCLUDED
	if ((GEngine->XRSystem.IsValid() == true) && ((GEngine->XRSystem->IsHeadTrackingAllowed() == true) || (GEngine->IsStereoscopic3D() == true)))
	{
		bHMDActive = true;
	}
#endif // HMD_MODULE_INCLUDED
	if (bHMDActive == false)
	{
		if ( (InputComponent) && (bInCarCameraActive == true ))
		{
			FRotator HeadRotation = InternalCamera->GetRelativeRotation();
			HeadRotation.Pitch += InputComponent->GetAxisValue(LookUpBinding);
			HeadRotation.Yaw += InputComponent->GetAxisValue(LookRightBinding);
			InternalCamera->SetRelativeRotation(HeadRotation);
		}
	}

	/* Boost */
	// Boost Term
	boostTerm += Delta;
	if (boostTerm > 0.5f) {

		// Boost Speed
		if (isBoost)
			GetMesh()->AddForce(FVector(GetMesh()->GetComponentVelocity().X * boostLevel, GetMesh()->GetComponentVelocity().Y * boostLevel, GetMesh()->GetComponentVelocity().Z), NAME_None, true);
		// Initialize Boost Term
		boostTerm = 0.0f;
	}

	if (isBoost) {
		


		// Reduce Guage
		boostValue -= boostReducingValue;
		if (boostValue <= 0.0f) {
			OnBoostSwitch();
			boostValue = 0.0f;
		}
	}
}

void AHSRasingPawn::BeginPlay()
{
	Super::BeginPlay();

	bool bEnableInCar = false;
#if HMD_MODULE_INCLUDED
	bEnableInCar = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
#endif // HMD_MODULE_INCLUDED
	EnableIncarView(bEnableInCar,true);

	BoostLineBody->Deactivate();
	EnergyLineBody->Deactivate();
}

void AHSRasingPawn::OnResetVR()
{
#if HMD_MODULE_INCLUDED
	if (GEngine->XRSystem.IsValid())
	{
		GEngine->XRSystem->ResetOrientationAndPosition();
		InternalCamera->SetRelativeLocation(InternalCameraOrigin);
		GetController()->SetControlRotation(FRotator());
	}
#endif // HMD_MODULE_INCLUDED
}

void AHSRasingPawn::UpdateHUDStrings()
{

	//UpdateKPH
	KPH = FMath::Abs(GetVehicleMovement()->GetForwardSpeed()) * 0.036f;
	KPH_int = FMath::FloorToInt(KPH);

	//KPH Niagara Effect
	if (!isBoost) {
		if (KPH_int >= 70) {
			SpeedLineBody->Activate();
		}
		else {
			SpeedLineBody->Deactivate();
		}
	}

	// Using FText because this is display text that should be localizable
	SpeedDisplayString = FText::Format(LOCTEXT("SpeedFormat", "{0} km/h"), FText::AsNumber(KPH_int));
	
	if (bInReverseGear == true)
	{
		GearDisplayString = FText(LOCTEXT("ReverseGear", "R"));
	}
	else
	{
		int32 Gear = GetVehicleMovement()->GetCurrentGear();
		GearDisplayString = (Gear == 0) ? LOCTEXT("N", "N") : FText::AsNumber(Gear);
	}	
}

void AHSRasingPawn::SetupInCarHUD()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if ((PlayerController != nullptr) && (InCarSpeed != nullptr) && (InCarGear != nullptr) )
	{
		// Setup the text render component strings
		InCarSpeed->SetText(SpeedDisplayString);
		InCarGear->SetText(GearDisplayString);
		
		if (bInReverseGear == false)
		{
			InCarGear->SetTextRenderColor(GearDisplayColor);
		}
		else
		{
			InCarGear->SetTextRenderColor(GearDisplayReverseColor);
		}
	}
}

#undef LOCTEXT_NAMESPACE

PRAGMA_ENABLE_DEPRECATION_WARNINGS
