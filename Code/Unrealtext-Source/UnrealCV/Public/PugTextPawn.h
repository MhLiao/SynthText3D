// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ProceduralMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Engine/Texture2D.h"
//#include "
#include "PugTextPawn.generated.h"

UCLASS()
class UNREALCV_API APugTextPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APugTextPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void PostActorCreated();

	void PostLoad();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void CreateTriangle();
	void CreateTriangle2();

	void CreateTriangleComplex();
	bool CreateTriangleWithCube();
	bool CreateTriangleSameInstance();
	bool CreateTriangleCameraLight();
	bool CreateTriangleCameraLight2();

	bool CreateTriangleCameraLight_for_preStereo();
	bool CreateStereoText();
	bool isOnTheLeft(FVector2D p1, FVector2D p2, FVector2D p);

	void myProjection();

	FVector CameraLocation;
	FRotator CameraRotation;
	FVector CameraRotationV;

	FVector textRotationV;
	FRotator textRotation;
	FVector textLocation;

	float inp[4];
	float angle[4][2];

	int bboxNum=0;
	float bbox2D[100][4];
	float bboxMax1,bboxMax2;
	float bboxAngle[100][4][3];

	int charNum=0;
	float char2D[500][4];
	float charAngle[500][4][3];

	bool needMove=true;
	FString texture1_name="hahaha";
	FString texture2_name="hahaha2";

	float Emission=1.0;
	//FVector lightColor=FVector(1.0,0.0,0.0,1.0);
	FLinearColor lightColor;
	float gapLength=0.0;
	float textDepth=0.0;

	bool geomFlag=false;
	bool geomMaterialNum=0;
	bool needCurved=true;
	float curveGapPercent=0.1;

	int resizeFlag=3;
	bool needCalculateBBOX=false;
	bool needCalculateCharBBOX=false;

	//UTexture2D* LoadTexture2D(const FString path, bool& IsValid, int32& OutWidth, int32& OutHeight);

private:
	//UDirectionalLightComponent* directionalLight;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent * mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UDecalComponent* CursorToWorld;

	UTexture2D* LoadTexture2D(const FString path, bool& IsValid, int32& OutWidth, int32& OutHeight);
	//bool getHitSame

	UTexture2D *textImg;
	UTexture2D *textImg2;
	TArray<FVector> vertices;
	TArray<FVector> realvertices;
	TArray<int32> Triangles;
	TArray<FVector> normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> tangents;
	TArray<FLinearColor> vertexColors;

	static const int Y_COUNT=100;
	static const int Z_COUNT=100;
	float vertexLens[Z_COUNT][Y_COUNT+1];
	TArray<FVector> imgVertices;
	TArray<FVector> imgNormals;
	FVector2D** relaVertices;
	FVector** pixels;
	FVector** pixel_normals;
	int textureH,textureW;
	bool needRecordAlpha=false;
	TArray<uint8> mRGBA;
	//const TArray<uint8>* UncompressedRGBA=NULL;

	//UTexture *Texture;
	//UMaterial* Material;
	//UMaterialInstanceDynamic *DynamicMaterial;
	
};
