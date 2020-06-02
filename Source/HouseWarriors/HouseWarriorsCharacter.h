// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HouseWarriorsCharacter.generated.h"

UCLASS(config=Game)
class AHouseWarriorsCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AHouseWarriorsCharacter();

protected:

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float BaseLookUpRate;

	/*Health implementation settings*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = HealthDebug)
		float RegenTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = HealthDebug)
		float CountDownUntilRegen;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = HealthDebug)
		int Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health)
		float MaxHealth; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health)
		float MaxRegenTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health)
		float MaxCountDownUntilRegen;

	/*This variable is used for how much health you get back over time while regenerating*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health)
		int HealthPlusPerSeconds;

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/*Jump behaviour*/
	UFUNCTION(BlueprintCallable, Category = "Character Movement")
		void StartJump();
	/*stop jumping call*/
	void StopJump();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/* Called for faster movement input*/
	void Run();
	void RunStop();

	/*Health regeneration over time*/
	void RegenerateHealth(float deltaTime);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	/** The target ground speed when running. */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float RunSpeed;

	/** The target ground speed when walking slowly. */
	UPROPERTY(Category = "Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float WalkSpeed;

	/** The interact distance */
	UPROPERTY(Category = "Interaction", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float MaxDistanceToInteract;

	UPROPERTY(Category = "Interaction", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
		float MinAngleToInteractable;

	UFUNCTION(BlueprintCallable, Category = "Health")
		void CanRegenerateHealth(float deltaTime);

	UFUNCTION(BlueprintCallable, Category = "Health")
		void DealDamage(int damageAmount);

	UFUNCTION(BlueprintCallable, Category = "UI")
		void UIHealth(float& uiValue, float& isActivated);

	UFUNCTION(BlueprintNativeEvent, Category = "Health")
		void OnDeath();
		void OnDeath_Implementation();

	/*Using this to play sounds for now*/
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
		void OnHover();
		void OnHover_Implementation();


protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

private:
	/*booleans for use in this h & cpp and not anywhere else*/
	bool bIsRegeneratingHealth;

	/*private float values*/
	float currentUIValue;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

