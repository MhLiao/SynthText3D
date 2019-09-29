// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "myCameraRecordPawn.generated.h"

UCLASS()
class UNREALTEXT_API AmyCameraRecordPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AmyCameraRecordPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MouseLBegin();
	void MouseLStop();

	// Handles input for moving forward and backward.
    //UFUNCTION()
    void MoveForward(float Value);

    // Handles input for moving right and left.
    //UFUNCTION()
    void MoveRight(float Value);

    void Turn(float Value);
    void LookUp(float Value);
	
};
