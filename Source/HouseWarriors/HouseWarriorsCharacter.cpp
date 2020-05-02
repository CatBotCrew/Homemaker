// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HouseWarriorsCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "Kismet/GameplayStatics.h"
#include "Interactable.h"

//////////////////////////////////////////////////////////////////////////
// AHouseWarriorsCharacter

AHouseWarriorsCharacter::AHouseWarriorsCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// set our run and walk speeds
	RunSpeed = 200.0f;
	WalkSpeed = 100.0f;

	//Other settings
	bIsRegeneratingHealth = false;
	MaxHealth = 100;
	MaxRegenTimer = 10.0f; //in seconds
	MaxCountDownUntilRegen = 20.0f; //in seconds
	Health = MaxHealth;
	RegenTimer = 0.0f; //in seconds
	CountDownUntilRegen = MaxCountDownUntilRegen; //in seconds
	HealthPlusPerSeconds = 10;
	currentUIValue = 0.0f;

	//Interactble variables
	MinAngleToInteractable = 30.0f;
	MaxDistanceToInteract = 300.0f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = true;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 300.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 350.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bEnableCameraLag = true; //Enable or Disable camera lag behind the character

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AHouseWarriorsCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	
	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &AHouseWarriorsCharacter::Run);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &AHouseWarriorsCharacter::RunStop);

	PlayerInputComponent->BindAxis("MoveForward", this, &AHouseWarriorsCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHouseWarriorsCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	/*PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AHouseWarriorsCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AHouseWarriorsCharacter::LookUpAtRate);*/

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AHouseWarriorsCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AHouseWarriorsCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AHouseWarriorsCharacter::OnResetVR);
}


void AHouseWarriorsCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AHouseWarriorsCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AHouseWarriorsCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AHouseWarriorsCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AHouseWarriorsCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AHouseWarriorsCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AHouseWarriorsCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AHouseWarriorsCharacter::Run()
{
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void AHouseWarriorsCharacter::RunStop()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AHouseWarriorsCharacter::RegenerateHealth(float deltaTime)
{
	RegenTimer -= deltaTime;
	if (RegenTimer <= 0.0f) {
		Health += HealthPlusPerSeconds;
		RegenTimer = MaxRegenTimer;
		if (Health >= MaxHealth) {
			bIsRegeneratingHealth = false;
			Health = MaxHealth;
			CountDownUntilRegen = MaxCountDownUntilRegen;
			RegenTimer = 0.0f;
		}
	}
}

void AHouseWarriorsCharacter::CanRegenerateHealth(float deltaTime)
{
	if (bIsRegeneratingHealth == true) {
		CountDownUntilRegen -= deltaTime;
		if (CountDownUntilRegen <= 0.0f) {
			RegenerateHealth(deltaTime);
		}
	}
}

void AHouseWarriorsCharacter::DealDamage(int damageAmount)
{
	Health -= damageAmount;
	if (Health <= 0) {
		OnDeath();
	}
	else {
		bIsRegeneratingHealth = true;
		CountDownUntilRegen = MaxCountDownUntilRegen; //making sure it gets reset if the player gets damaged again.
		RegenTimer = 0.0f;
	}
}

void AHouseWarriorsCharacter::UIHealth(float& uiValue, float& isActivated)
{	
	if (bIsRegeneratingHealth) {
		isActivated = 1.0f;
		uiValue = (((Health - 0) * (5 - 0)) / (100 - 0)) + 0;
	}
	else {
		uiValue = 5.0f;
		isActivated = 0.0f;
	}
}

void AHouseWarriorsCharacter::OnDeath_Implementation()
{
	Health = MaxHealth;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Fuckka You, You Dead!"));
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false); //reloads current level
}

void AHouseWarriorsCharacter::OnHover_Implementation()
{
	/*TODO: implement shaking or something if needed here in code then call parent else instead implement in bp*/
}
