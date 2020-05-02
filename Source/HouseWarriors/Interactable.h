// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.generated.h"

UCLASS()
class HOUSEWARRIORS_API AInteractable : public AActor
{
	GENERATED_BODY()
	
		/** Box Collision */
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision, meta = (AllowPrivateAccess = "true"))
		class UBoxComponent* CollisionBox;
public:	
	// Sets default values for this actor's properties
	AInteractable();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
		void InteractActivated();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
