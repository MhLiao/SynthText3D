// Fill out your copyright notice in the Description page of Project Settings.


#include "myCameraRecordPawn.h"
#include <stdio.h>
#include <stdlib.h>


// Sets default values
AmyCameraRecordPawn::AmyCameraRecordPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//AutoPossessPlayer = EAutoReceiveInput::Player0;

    // Create a dummy root component we can attach things to.
    //RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    // Create a camera and a visible object
    //UCameraComponent* OurCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("OurCamera"));
    //OurVisibleComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OurVisibleComponent"));
    // Attach our camera and visible object to our root component. Offset and rotate the camera.
    //OurCamera->SetupAttachment(RootComponent);
    //OurCamera->SetRelativeLocation(FVector(-250.0f, 0.0f, 250.0f));
    //OurCamera->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));
    //OurVisibleComponent->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AmyCameraRecordPawn::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine)
    {
        // Put up a debug message for five seconds. The -1 "Key" value (first argument) indicates that we will never need to update or refresh this message.
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("We are using AmyCameraRecordPawn."));
    }
	
	
}

// Called every frame
void AmyCameraRecordPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
/*void AmyCameraRecordPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}*/

void AmyCameraRecordPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//UE_LOG(LogTemp, Warning, TEXT("???*************************"));
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//check(InputComponent);

	//PlayerInputComponent->BindAxis("CatchNow", this, &AmyCameraRecordPawn::MouseLBegin);
	//PlayerInputComponent->BindAxis("CatchNow", this, &AmyCameraRecordPawn::MouseLStop);
	PlayerInputComponent->BindAction("CatchNow", IE_Pressed, this, &AmyCameraRecordPawn::MouseLBegin);
	PlayerInputComponent->BindAction("CatchNow", IE_Released, this, &AmyCameraRecordPawn::MouseLStop);

	// Set up "movement" bindings.
    PlayerInputComponent->BindAxis("MoveForward", this, &AmyCameraRecordPawn::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AmyCameraRecordPawn::MoveRight);
    //
    PlayerInputComponent->BindAxis("Turn", this, &AmyCameraRecordPawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &AmyCameraRecordPawn::AddControllerPitchInput);
}

void AmyCameraRecordPawn::MouseLBegin()
{
	// TODO
	//GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Blue, TEXT("MouseLStop_OK"));
}

// Record camera informations
void AmyCameraRecordPawn::MouseLStop()
{
	//GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Blue, TEXT("MouseLStop_OK"));
	// GetActorLocation and GetActorRotation are methods defined under AActor
	// GetControlRotation: Pawn.cpp
	FVector currentLocation = GetActorLocation();
	FRotator currentRotation = GetControlRotation();//GetActorRotation();
	//
	FILE* info_file = fopen("camera_trajectory1.txt","a");
	fprintf(info_file, "%.3f\n", currentLocation.X);
	fprintf(info_file, "%.3f\n", currentLocation.Y);
	fprintf(info_file, "%.3f\n", currentLocation.Z);
	fprintf(info_file, "%.3f\n", currentRotation.Pitch);
	fprintf(info_file, "%.3f\n", currentRotation.Yaw);
	fprintf(info_file, "%.3f\n", currentRotation.Roll);
	fclose(info_file);
	UE_LOG(LogTemp, Warning, TEXT("Saved a position."));
	// use trajectory to get images(lit, depth, mask)

}

float myScale=10.0;

void AmyCameraRecordPawn::MoveForward(float Value)
{
    // Find out which way is "forward" and record that the player wants to move that way.
    //GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Blue, TEXT("move forward"));
    FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
    FVector currentLocation = GetActorLocation();
    if(Value!=0.0){
    	currentLocation+=Direction*Value*myScale;
    	SetActorLocation(currentLocation);
    	//SetActorLocation(FVector(0.0,0.0,32.0));
    }
    //AddMovementInput(Direction, Value);
}

void AmyCameraRecordPawn::MoveRight(float Value)
{
    // Find out which way is "right" and record that the player wants to move that way.
    //GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Blue, TEXT("move right"));
    //UE_LOG(LogTemp, Warning, TEXT("Thred : %f"), Value);
    FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
    FVector currentLocation = GetActorLocation();
    if(Value!=0.0){
    	currentLocation+=Direction*Value*myScale;
    	SetActorLocation(currentLocation);
    	//SetActorLocation(FVector(0.0,0.0,32.0));
    }
    //AddMovementInput(Direction, Value);
}

void AmyCameraRecordPawn::Turn(float Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Turn : %f"), Value);
	FRotator currentRotation = GetActorRotation();
	if(Value!=0.0){
		currentRotation.Yaw += Value;
		//SetControlRotation(currentRotation);
	}
}

void AmyCameraRecordPawn::LookUp(float Value)
{
	UE_LOG(LogTemp, Warning, TEXT("lookup : %f"), Value);
	FRotator currentRotation = GetActorRotation();
	UE_LOG(LogTemp, Warning, TEXT("currentRotation : %.3f  %.3f"), currentRotation.Pitch, currentRotation.Yaw);
	if(Value!=0.0){
		currentRotation.Pitch += Value;
		//SetControlRotation(currentRotation);
	}
}
