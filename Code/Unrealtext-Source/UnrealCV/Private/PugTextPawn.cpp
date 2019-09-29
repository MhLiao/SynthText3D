// Fill out your copyright notice in the Description page of Project Settings.
#include "UnrealCVPrivate.h"
#include "PugTextPawn.h"
#include "Engine/World.h"
#include "ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EngineGlobals.h"
#include "Engine.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/PlayerController.h"

#include "UE4CVServer.h"
#include "CineCameraActor.h"

#include "FileManagerGeneric.h"
#include "Runtime/ImageWrapper/Public/Interfaces/IImageWrapper.h"
#include "Runtime/ImageWrapper/Public/Interfaces/IImageWrapperModule.h"

#include "Runtime/Engine/Classes/Engine/GameViewportClient.h"

#include <fstream>
#include <stdlib.h>
#include <math.h>
#include <time.h>

//const float PAI2 = 3.1415926;
const float elementMeshY = 2.5;//half length
const float elementMeshZ = 1.25;//half length

char cache_dir[]="~/my_bbox_cache.txt";


// Sets default values
APugTextPawn::APugTextPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	RootComponent = mesh;
	inp[0]=-26.296875;
	inp[1]=27.0;
	inp[2]=-0.0;//13.640625;
	inp[3]=0.0;//10.828125;

}

// Called when the game starts or when spawned
void APugTextPawn::BeginPlay()
{
	Super::BeginPlay();

}

static IImageWrapperPtr GetImageWrapperByExtention(const FString InImagePath)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	if (InImagePath.EndsWith(".png"))
	{
		return ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	}
	else if (InImagePath.EndsWith(".jpg") || InImagePath.EndsWith(".jpeg"))
	{
		return ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
	}
	else if (InImagePath.EndsWith(".bmp"))
	{
		return ImageWrapperModule.CreateImageWrapper(EImageFormat::BMP);
	}
	else if (InImagePath.EndsWith(".ico"))
	{
		return ImageWrapperModule.CreateImageWrapper(EImageFormat::ICO);

	}
	else if (InImagePath.EndsWith("exr"))
	{
		return ImageWrapperModule.CreateImageWrapper(EImageFormat::EXR);
	}
	else if (InImagePath.EndsWith(".icns"))
	{
		return ImageWrapperModule.CreateImageWrapper(EImageFormat::ICNS);
	}
	return nullptr;
}

UTexture2D* APugTextPawn::LoadTexture2D(const FString path, bool& IsValid, int32& OutWidth, int32& OutHeight)
{
	UTexture2D* Texture = nullptr;
	IsValid = false;
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*path))
	{
		return nullptr;
	}
	TArray<uint8> RawFileData;
	if (!FFileHelper::LoadFileToArray(RawFileData, *path))
	{
		return nullptr;
	}
	IImageWrapperPtr ImageWrapper = GetImageWrapperByExtention(path);
	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
	{
		const TArray<uint8>* UncompressedRGBA=NULL;
		//if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedRGBA))
		if (ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, UncompressedRGBA))
		{
			Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_R8G8B8A8);
			if (Texture != nullptr)
			{
				IsValid = true;
				OutWidth = ImageWrapper->GetWidth();
				OutHeight = ImageWrapper->GetHeight();
				void* TextureData = Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
				FMemory::Memcpy(TextureData, UncompressedRGBA->GetData(), UncompressedRGBA->Num());
				//UE_LOG(LogTemp, Warning, TEXT("xxxx %d"), UncompressedRGBA->Num());
				Texture->PlatformData->Mips[0].BulkData.Unlock();
				Texture->UpdateResource();
			}
			if(needRecordAlpha){
				mRGBA.Empty();
				int pIndex=0;
				//UE_LOG(LogTemp, Warning, TEXT("***%d %d"),OutHeight, OutWidth);
				for(int ih=0;ih<OutHeight;++ih){
					for(int iw=0;iw<OutWidth;++iw){
						if((*UncompressedRGBA)[ih*OutWidth*4+iw*4]>230){
							mRGBA.Add(unsigned(1));
							++pIndex;
						}else{
							mRGBA.Add(unsigned(0));
						}
						
					}
				}
				//UE_LOG(LogTemp, Warning, TEXT("***pixel yeah%d"),pIndex);
			}
			//for(int pi=0;pi<UncompressedRGBA->Num();++pi)
			//	UE_LOG(LogTemp, Warning, TEXT("***pixel yeah! %d [%d %d %d %d]"), pi, (*UncompressedRGBA)[pi*4+0], (*UncompressedRGBA)[pi*4+1], (*UncompressedRGBA)[pi*4+2], (*UncompressedRGBA)[pi*4+3]);
					
		}
	}
	return Texture;

}

// Called every frame
void APugTextPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(needMove==false){
		return;
	}

	bool bIsMatinee = false;
	//FVector CameraLocation;
	//FRotator CameraRotation;
	ACineCameraActor* CineCameraActor = nullptr;
	for (AActor* Actor : this->GetWorld()->GetCurrentLevel()->Actors)
	{
		// if (Actor && Actor->IsA(AMatineeActor::StaticClass())) // AMatineeActor is deprecated
		if (Actor && Actor->IsA(ACineCameraActor::StaticClass()))
		{
			bIsMatinee = true;
			CameraLocation = Actor->GetActorLocation();
			CameraRotation = Actor->GetActorRotation();
			break;
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("isMatinee: %d"), (int)bIsMatinee);
	if (!bIsMatinee)
	{
		APawn* Pawn = FUE4CVServer::Get().GetPawn();
		CameraLocation = Pawn->GetActorLocation();
		CameraRotation = Pawn->GetControlRotation();
	}
	//cameraRotationV=CameraRotation.Rotation();
	
	mesh->ClearAllMeshSections();
	bool createMeshFlag;
	if(geomFlag)
		createMeshFlag = CreateTriangleCameraLight_for_preStereo();//
	else
		createMeshFlag = CreateTriangleCameraLight2();
	//bool createMeshFlag =CreateTriangleCameraLight2();
	//UE_LOG(LogTemp, Warning, TEXT("asdf"));
	if(createMeshFlag){
		//UE_LOG(LogTemp, Warning, TEXT("xxxx"));
		//textRotation.Yaw += 15.0;
		mesh->SetAllPhysicsPosition(textLocation);
		UE_LOG(LogTemp, Warning, TEXT("xxxx %f %f %f"), textLocation.X, textLocation.Y, textLocation.Z);
		//textRotation.Pitch=0.0;
		//textRotation.Yaw=0.0;
		//textRotation.Roll=0.0;
		textRotation.Pitch=textRotation.Roll=textRotation.Yaw=0.0;
		mesh->SetAllPhysicsRotation(textRotation);
		//mesh->SetWorldScale3D(FVector(0.8f));
		//// UE_LOG(LogTemp, Warning, TEXT("  rotation: %.3f %.3f %.3f"), textRotation.Pitch, textRotation.Yaw, textRotation.Roll);
		//// UE_LOG(LogTemp, Warning, TEXT("Impactnormal : %.3f %.3f %.3f"), textRotationV.X, textRotationV.Y, textRotationV.Z);
		//UE_LOG(LogTemp, Warning, TEXT("normal : %.3f %.3f %.3f"), Hit.Normal.X, Hit.Normal.Y, Hit.Normal.Z);
		//UE_LOG(LogTemp, Warning, TEXT("camera Rotation : %.3f %.3f %.3f"), CameraRotation.Pitch, CameraRotation.Yaw, CameraRotation.Roll);
		 //UE_LOG(LogTemp, Warning, TEXT("text Rotation : %.3f %.3f %.3f"), textRotation.Pitch, textRotation.Yaw, textRotation.Roll);
		FString path[5] = { "/Game/UnrealText/aamat1_Red.aamat1_Red" ,
			"/Game/Downtown/Materials/MetalPainted_Blue.MetalPainted_Blue",
			"/Game/StarterContent/Materials/M_Ground_Grass.M_Ground_Grass",
			"/Game/StarterContent/Materials/M_Wood_Oak.M_Wood_Oak",
			"/Game/StarterContent/Materials/M_Rock_Basalt.M_Rock_Basalt"
		};
		static UMaterialInterface* TemplateMaterial = nullptr;
		if(geomFlag)
			TemplateMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *(path[geomMaterialNum])));
		else
			TemplateMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *(path[0])));
		//static UMaterialInterface* TemplateMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *(path[0])));
		if (TemplateMaterial == nullptr) {
			UE_LOG(LogTemp, Warning, TEXT(" temp material NULL"));
			return;
		}
		UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(TemplateMaterial, nullptr);//
		if (MaterialInstance)
		{

			UE_LOG(LogTemp, Warning, TEXT("zzzz"));
			//设置材质参数
			bool isValid;
			int outW, outH;
			FString imgName;

			imgName = "~/WordCrops/";
			imgName += texture1_name;
			imgName += ".png";
			needRecordAlpha=false;
			textImg = this->LoadTexture2D(imgName, isValid, outW, outH);
			needRecordAlpha=false;
			textureH = outH;
			textureW = outW;
			if(textImg==nullptr){
				//UE_LOG(LogTemp, Warning, TEXT(" texture1 NULL"));
				UE_LOG(LogTemp, Warning, TEXT(" texture1 NULL %s"),*imgName);
			}
			imgName = "~/WordCrops/";
			imgName += texture2_name;
			imgName += ".png";
			needRecordAlpha=true;
			textImg2 = this->LoadTexture2D(imgName, isValid, outW, outH);
			needRecordAlpha=false;
			if(textImg2==nullptr){
				//UE_LOG(LogTemp, Warning, TEXT(" texture2 NULL"));
				UE_LOG(LogTemp, Warning, TEXT(" texture2 NULL %s"),*imgName);
			}

			if(geomFlag){
				createMeshFlag = CreateStereoText();
				UE_LOG(LogTemp, Warning, TEXT("Stero Done"));
				mesh->SetMaterial(0, MaterialInstance);
			}else{
				MaterialInstance->SetTextureParameterValue(TEXT("textContent"), textImg);
				MaterialInstance->SetTextureParameterValue(TEXT("textTransp"), textImg2);
				MaterialInstance->SetScalarParameterValue(TEXT("Emission"), Emission);
				MaterialInstance->SetVectorParameterValue(TEXT("lightColor"), lightColor);
				mesh->SetMaterial(0, MaterialInstance);
				UE_LOG(LogTemp, Warning, TEXT("Done"));
			}
		}
	}
}

// This is called when actor is spawned (at runtime or when you drop it into the world in editor)
void APugTextPawn::PostActorCreated()
{
	Super::PostActorCreated();
	CreateTriangle();
}

// This is called when actor is already in level and map is opened
void APugTextPawn::PostLoad()
{
	Super::PostLoad();
	//CreateTriangle();
}

// Called to bind functionality to input
void APugTextPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	UE_LOG(LogTemp, Warning, TEXT(" *************************"));
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UE_LOG(LogTemp, Warning, TEXT(" *************************"));
}

void APugTextPawn::CreateTriangle()
{
	vertices.Empty();
	Triangles.Empty();
	normals.Empty();
	UV0.Empty();
	tangents.Empty();
	vertexColors.Empty();
	
	//TArray<FVector> vertices;
	vertices.Add(FVector(0, -25, -25));
	vertices.Add(FVector(0, 25, -25));
	vertices.Add(FVector(0, -25, 25));
	vertices.Add(FVector(0, 25,25));
	Triangles.Add(0);
	Triangles.Add(2);
	Triangles.Add(1);
	Triangles.Add(3);
	Triangles.Add(1);
	Triangles.Add(2);
	//Triangles.Add(3);
	//Triangles.Add(4);
	//Triangles.Add(2);

	//TArray<FVector> normals;
	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));

	//TArray<FVector2D> UV0;
	UV0.Add(FVector2D(1.0, 1.0));
	UV0.Add(FVector2D(0.0, 1.0));
	UV0.Add(FVector2D(1.0, 0.0));
	UV0.Add(FVector2D(0.0, 0.0));


	//TArray<FProcMeshTangent> tangents;
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));

	//TArray<FLinearColor> vertexColors;
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

	mesh->CreateMeshSection_LinearColor(0, vertices, Triangles, normals, UV0, vertexColors, tangents, true);

	// Enable collision data
	mesh->ContainsPhysicsTriMeshData(true);
}


void APugTextPawn::CreateTriangle2()
{
	vertices.Empty();
	Triangles.Empty();
	normals.Empty();
	UV0.Empty();
	tangents.Empty();
	vertexColors.Empty();

	//TArray<FVector> vertices;
	vertices.Add(FVector(0, -5, -5));
	vertices.Add(FVector(0, 5, -5));
	vertices.Add(FVector(0, -5, 5));
	vertices.Add(FVector(0, 5, 5));
	Triangles.Add(0);
	Triangles.Add(2);
	Triangles.Add(1);
	Triangles.Add(3);
	Triangles.Add(1);
	Triangles.Add(2);
	//Triangles.Add(3);
	//Triangles.Add(4);
	//Triangles.Add(2);

	//TArray<FVector> normals;
	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));
	normals.Add(FVector(1, 0, 0));

	//TArray<FVector2D> UV0;
	UV0.Add(FVector2D(80, 80));
	UV0.Add(FVector2D(100, 80));
	UV0.Add(FVector2D(80, 100));
	UV0.Add(FVector2D(100, 100));


	//TArray<FProcMeshTangent> tangents;
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));
	tangents.Add(FProcMeshTangent(0, 1, 0));

	//TArray<FLinearColor> vertexColors;
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
	vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));

	mesh->CreateMeshSection_LinearColor(0, vertices, Triangles, normals, UV0, vertexColors, tangents, true);

	// Enable collision data
	mesh->ContainsPhysicsTriMeshData(true);
}

float distanceInVector(FVector v, FVector u) {
	if (v.Size() != 0.0)
		return FVector::DotProduct(v, u) / v.Size();
	else
		return 0.0;
}

void APugTextPawn::myProjection(){
	return;
}


void APugTextPawn::CreateTriangleComplex()
{
	vertices.Empty();
	Triangles.Empty();
	normals.Empty();
	UV0.Empty();
	tangents.Empty();
	vertexColors.Empty();
	//TArray<FVector> vertices;
	bool bIsMatinee = false;

	float actorL = 1000.0;
	FVector TraceEnd;
	FVector TraceStart;
	TraceEnd.X = CameraLocation.X;// +0.1*CameraRotation.Pitch;
	TraceEnd.Y = CameraLocation.Y;// +0.1*CameraRotation.Yaw;
	TraceEnd.Z = CameraLocation.Z;// +0.1*CameraRotation.Roll;
	UE_LOG(LogTemp, Warning, TEXT("camera position: %.3f %.3f %.3f"), CameraLocation.X, CameraLocation.Y, CameraLocation.Z);

	//TraceEnd.X += actorL * (cos(PAI * CameraRotation.Pitch / 180.0) * cos(PAI * CameraRotation.Yaw / 180.0));
	//TraceEnd.Y += actorL * (cos(PAI * CameraRotation.Pitch / 180.0) * sin(PAI * CameraRotation.Yaw / 180.0));
	//TraceEnd.Z += actorL * (sin(PAI * CameraRotation.Pitch / 180.0));
	//
	//==============
	FCollisionQueryParams TraceParams(FName(TEXT("TraceUsableActor")), true, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;

	FHitResult Hit(ForceInit);
	//====================================================================================
	FVector elementLocation;

	float x_scale = 1.0, y_scale = 1.0, z_scale = 1.0;
	const int Y_NUM = 100, Z_NUM = 50;
	const float Y_LEN = 50.0;
	const float Z_LEN = 25.0;
	float half_y_len = Y_LEN / 2.0;
	float half_z_len = Z_LEN / 2.0;

	float centerHitLen;
	FVector centerHitNormal;
	FVector centerHitHoz;
	FVector centerHitUp;
	FVector centerHitLocation;
	float y_textAngle, z_textAngle;
	TraceEnd.X += actorL * (cos(PAI * CameraRotation.Pitch / 180.0) * cos(PAI * (CameraRotation.Yaw) / 180.0));
	TraceEnd.Y += actorL * (cos(PAI * CameraRotation.Pitch / 180.0) * sin(PAI * (CameraRotation.Yaw) / 180.0));
	TraceEnd.Z += actorL * (sin(PAI * CameraRotation.Pitch / 180.0));
	bool hitflag = GetWorld()->LineTraceSingleByChannel(Hit, CameraLocation, TraceEnd, ECC_Visibility, TraceParams);
	if (hitflag) {
		centerHitLen = Hit.Distance;
		centerHitNormal = Hit.ImpactNormal;
		UE_LOG(LogTemp, Warning, TEXT("centerHitNormal: %.3f %.3f %.3f"), centerHitNormal.X, centerHitNormal.Y, centerHitNormal.Z);
		if (centerHitNormal.X != 0.0 && centerHitNormal.Y != 0.0) {
			UE_LOG(LogTemp, Warning, TEXT("c1"));
			centerHitHoz.X = sqrt((centerHitNormal.X*centerHitNormal.X + centerHitNormal.Y*centerHitNormal.Y) / (centerHitNormal.X*centerHitNormal.X));
			centerHitHoz.Y = -(centerHitNormal.X / centerHitNormal.Y)*centerHitHoz.X;//sqrt((centerHitNormal.X*centerHitNormal.X + centerHitNormal.Y*centerHitNormal.Y) / (centerHitNormal.X*centerHitNormal.X));
		}
		//
		else if (centerHitNormal.X != 0.0 && centerHitNormal.Y == 0.0) {
			UE_LOG(LogTemp, Warning, TEXT("c2"));
			centerHitHoz.X = 0.0;
			centerHitHoz.Y = 1.0;
		}
		else if (centerHitNormal.X == 0.0 && centerHitNormal.Y != 0.0) {
			UE_LOG(LogTemp, Warning, TEXT("c3"));
			centerHitHoz.X = 0.0;
			centerHitHoz.Y = 1.0;
		}
		else {
			centerHitHoz.X = 0.0;
			centerHitHoz.Y = 1.0;
		}
		centerHitHoz.Z = 0.0;
		if (centerHitHoz.Y < 0.0)
			centerHitHoz = -centerHitHoz;
		centerHitUp = FVector::CrossProduct(centerHitHoz, centerHitNormal);
		centerHitLocation = Hit.Location;

	}
	UE_LOG(LogTemp, Warning, TEXT("centerHitHoz: %.3f %.3f %.3f"), centerHitHoz.X, centerHitHoz.Y, centerHitHoz.Z);
	UE_LOG(LogTemp, Warning, TEXT("centerHitUp: %.3f %.3f %.3f"), centerHitUp.X, centerHitUp.Y, centerHitUp.Z);
	if (hitflag) {

		for (int i = 0;i < Y_NUM;i++) {
			for (int j = 0;j < Z_NUM;j++) {
				//
				y_textAngle = atan((-half_y_len + ((float)i)*(Y_LEN / Y_NUM)) / centerHitLen);
				z_textAngle = atan((-half_z_len + ((float)j)*(Z_LEN / Z_NUM)) / centerHitLen);
				actorL = 1.2*centerHitLen;
				TraceEnd = CameraLocation;
				TraceEnd.X += actorL * (cos(PAI * (CameraRotation.Pitch) / 180.0 + z_textAngle) * cos(PAI * (CameraRotation.Yaw) / 180.0 + y_textAngle));
				TraceEnd.Y += actorL * (cos(PAI * (CameraRotation.Pitch) / 180.0 + z_textAngle) * sin(PAI * (CameraRotation.Yaw) / 180.0 + y_textAngle));
				TraceEnd.Z += actorL * (sin(PAI * (CameraRotation.Pitch) / 180.0 + z_textAngle));
				actorL = 0.8*centerHitLen;
				TraceStart = CameraLocation;
				TraceStart.X += actorL * (cos(PAI * (CameraRotation.Pitch) / 180.0 + z_textAngle) * cos(PAI * (CameraRotation.Yaw) / 180.0 + y_textAngle));
				TraceStart.Y += actorL * (cos(PAI * (CameraRotation.Pitch) / 180.0 + z_textAngle) * sin(PAI * (CameraRotation.Yaw) / 180.0 + y_textAngle));
				TraceStart.Z += actorL * (sin(PAI * (CameraRotation.Pitch) / 180.0 + z_textAngle));

				//FVector textRotationV;
				//FRotator textRotation;
				if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams)) {
					textRotationV = Hit.ImpactNormal;
					elementLocation = Hit.Location;
					textRotation = textRotationV.Rotation();
					
					//UE_LOG(LogTemp, Warning, TEXT("hit: %s"), *Hit.GetActor()->GetName());
				}
				else {
					elementLocation = TraceEnd;
				}

				FVector shiftV = elementLocation - centerHitLocation;
				float nL = distanceInVector(centerHitNormal, shiftV);
				float hL = distanceInVector(centerHitHoz, shiftV);
				float uL = distanceInVector(centerHitUp, shiftV);
				//SetActorLocation(textLocation);
				//newTA->SetActorRotation(CameraRotation);
				//mesh->SetAllPhysicsPosition(textLocation);
				//mesh->SetAllPhysicsRotation(textRotation);
				//if()
				
				vertices.Add(FVector(nL, hL, -uL));
				//UE_LOG(LogTemp, Warning, TEXT("shiftv: %.3f %.3f %.3f"), shiftV.X,shiftV.Y,shiftV.Z);
				//UE_LOG(LogTemp, Warning, TEXT("element location: %.3f %.3f %.3f"), nL,hL,uL);
				//vertices.Add(FVector(0.0, y_scale * (-5.0 + ((float)i)*(Y_LEN/Y_NUM) ), z_scale * (-5.0 + ((float)j)*(Z_LEN/Z_NUM)) ));
				//vertices.Add(elementLocation);
				//vertices.Add(shiftV);
				normals.Add(FVector(1, 0, 0));
				UV0.Add(FVector2D(((float)i) / Y_NUM, ((float)j) / Z_NUM));
				tangents.Add(FProcMeshTangent(0, 1, 0));
				vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
			}
		}
		//TArray<int32> Triangles;
		for (int i = 0;i < Y_NUM - 1;i++) {
			for (int j = 0;j < Z_NUM - 1;j++) {
				Triangles.Add(Z_NUM*i + j);
				Triangles.Add(Z_NUM*i + j + 1);
				Triangles.Add(Z_NUM*(i + 1) + j);
				//Triangles.Add(Z_NUM*i + j + 1);
				//
				Triangles.Add(Z_NUM*(i + 1) + j + 1);
				Triangles.Add(Z_NUM*(i + 1) + j);
				Triangles.Add(Z_NUM*i + j + 1);
				//Triangles.Add(Z_NUM*(i + 1) + j);
			}
		}

		mesh->CreateMeshSection_LinearColor(0, vertices, Triangles, normals, UV0, vertexColors, tangents, true);

		// Enable collision data
		mesh->ContainsPhysicsTriMeshData(true);
	}
}

bool isEqZero(float f){
	if(f<0.001 && f>-0.001)
		return true;
	else
		return false;
}

bool APugTextPawn::CreateTriangleWithCube()
{
	/*angle[0][0]=-19.0;
	angle[0][1]=3.5;
	angle[1][0]=-19.0;
	angle[1][1]=-3.5;
	//
	angle[2][0]=5.0;
	angle[2][1]=3.5;
	angle[3][0]=5.0;
	angle[3][1]=-3.5;*/
	//
	float inp2[4];
	//std::ifstream inf(cache_dir);
	/*std::ifstream inf;
	inf.open(cache_dir);
	if (!inf.is_open())
	{
		//std::cout << "Error opening file";
		UE_LOG(LogTemp, Warning, TEXT("Error opening file"));
		return false;
	}
	char d[256];
	//while (!inf.eof())
	for(int i=0;i<4;i++)
	{
		inf.getline(d,100);
		//std::cout << d << std::endl;
		inp2[i] = atof(d);
		UE_LOG(LogTemp, Warning, TEXT("Reading: %.3f"),inp2[i]);
	}
	inf.close();*/
	//
	float mid_0=(inp[0]+inp[2])*0.5;
	float mid_1=(inp[1]+inp[3])*0.5;
	angle[0][0]=inp[0]+(mid_0-inp[0])*0.01;
	angle[0][1]=inp[1]+(mid_1-inp[1])*0.01;
	angle[1][0]=inp[0]+(mid_0-inp[0])*0.01;
	angle[1][1]=inp[3]+(mid_1-inp[3])*0.01;
	angle[2][0]=inp[2]+(mid_0-inp[2])*0.01;
	angle[2][1]=inp[1]+(mid_1-inp[1])*0.01;
	angle[3][0]=inp[2]+(mid_0-inp[2])*0.01;
	angle[3][1]=inp[3]+(mid_1-inp[3])*0.01;
	//
	FVector corner[4];
	FVector projectedCorner[4];
	FString cornerHitInstanceName[4];
	FVector cornerNorm[4];
	float LeftThred,TopThred,BottomThred,RightThred;
	bool hitFlag[4];
	//
	FVector relativeNorm, relativeHorz, relativeUp, relativeLocation;

	vertices.Empty();
	Triangles.Empty();
	normals.Empty();
	UV0.Empty();
	tangents.Empty();
	vertexColors.Empty();
	//TArray<FVector> vertices;
	bool bIsMatinee = false;

	float actorL = 1000000000.0;
	//UE_LOG(LogTemp, Warning, TEXT("camera position: %.3f %.3f %.3f"), CameraLocation.X, CameraLocation.Y, CameraLocation.Z);
	//===================================================================================
	float minAngle0,maxAngle0,minAngle1,maxAngle1;
	minAngle0=minAngle1=1000.0;
	maxAngle0=maxAngle1=-1000.0;
	for(int i=0;i<4;i++){
		if(angle[i][0]<minAngle0)
			minAngle0=angle[i][0];
		if(angle[i][0]>maxAngle0)
			maxAngle0=angle[i][0];
		if(angle[i][1]<minAngle1)
			minAngle1=angle[i][1];
		if(angle[i][1]>maxAngle1)
			maxAngle1=angle[i][1];
	}
	//==============
	FCollisionQueryParams TraceParams(FName(TEXT("TraceUsableActor")), true, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnFaceIndex = true;

	FHitResult Hit(ForceInit);
	FVector TraceEnd;
	FVector TraceStart;
	TraceStart = CameraLocation;
	int findHitCount=0;
	for(int i=0;i<4;i++)
		hitFlag[i]=false;
	//float angleStep
	/*while(hitFlag[0]*hitFlag[1]*hitFlag[2]*hitFlag[3]==0){
		for(int i=0;i<4;i++){
			if(hitFlag[i]==true)
				continue;
			TraceEnd = CameraLocation;
			TraceEnd.X += actorL * (cos(PAI * (CameraRotation.Pitch+angle[i][1]) / 180.0) * cos(PAI * (CameraRotation.Yaw+angle[i][0]) / 180.0));
			TraceEnd.Y += actorL * (cos(PAI * (CameraRotation.Pitch+angle[i][1]) / 180.0) * sin(PAI * (CameraRotation.Yaw+angle[i][0]) / 180.0));
			TraceEnd.Z += actorL * (sin(PAI * (CameraRotation.Pitch+angle[i][1]) / 180.0));
			// ++ // UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %.3f %.3f %.3f"), i, TraceEnd.X,TraceEnd.Y,TraceEnd.Z);
			bool hitflag = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
			if(hitflag){
				corner[i]=Hit.Location;
				cornerNorm[i]=Hit.ImpactNormal;
				cornerHitInstanceName[i]=Hit.GetActor()->GetName();
				hitFlag[i]=true;
			}else{
				hitFlag[i]=false;

			}
		}
		for(int i=0;i<4;i++){
			// ++ // UE_LOG(LogTemp, Warning, TEXT("corner %d: %.3f %.3f %.3f"), i, corner[i].X,corner[i].Y,corner[i].Z);
			// ++ // UE_LOG(LogTemp, Warning, TEXT("angle %d: %.3f %.3f"), i, angle[i][0], angle[i][1]);
			
			angle[i][0]*=1.0;//0.99;
			angle[i][1]*=1.0;//0.99;
			//angle[i][0]*=
		}
		++findHitCount;
		if(findHitCount>=100){
			UE_LOG(LogTemp, Warning, TEXT("hit iteration: %d"), findHitCount);
			return false;
		}

	}*/
	for(int i=0;i<4;i++){
			TraceEnd = CameraLocation;
			TraceEnd.X += actorL * (cos(PAI * (CameraRotation.Pitch+angle[i][1]) / 180.0) * cos(PAI * (CameraRotation.Yaw+angle[i][0]) / 180.0));
			TraceEnd.Y += actorL * (cos(PAI * (CameraRotation.Pitch+angle[i][1]) / 180.0) * sin(PAI * (CameraRotation.Yaw+angle[i][0]) / 180.0));
			TraceEnd.Z += actorL * (sin(PAI * (CameraRotation.Pitch+angle[i][1]) / 180.0));
			// ++ // UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %.3f %.3f %.3f"), i, TraceEnd.X,TraceEnd.Y,TraceEnd.Z);
			bool hitflag = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
			corner[i]=Hit.Location;
			cornerNorm[i]=Hit.ImpactNormal;
			hitFlag[i]=hitflag;
	}
	if(hitFlag[0]*hitFlag[1]*hitFlag[2]*hitFlag[3]==0){
		UE_LOG(LogTemp, Warning, TEXT("NO HIT!!!"));
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("NO HIT"));
		return false;
	}




	//for(int i=0;i<4;i++){
	//	UE_LOG(LogTemp, Warning, TEXT("corner %d: %.3f %.3f %.3f"), i, corner[i].X,corner[i].Y,corner[i].Z);
	//	UE_LOG(LogTemp, Warning, TEXT("angle %d: %.3f %.3f"), i, angle[i][0], angle[i][1]);
	//}
	
	//====================================================================================
	for(int i=0;i<4;i++){
		if(hitFlag[i]==true){
			UE_LOG(LogTemp, Warning, TEXT("angle %d: %.3f %.3f"), i, angle[i][0], angle[i][1]);
			UE_LOG(LogTemp, Warning, TEXT("corner %d: %.3f %.3f %.3f"), i, corner[i].X,corner[i].Y,corner[i].Z);
			relativeLocation=corner[i];
			relativeNorm=cornerNorm[i];
			if(!isEqZero(relativeNorm.X) && !isEqZero(relativeNorm.Y)){
				relativeHorz.Y=fabs(relativeNorm.X / sqrt(relativeNorm.X*relativeNorm.X+relativeNorm.Y*relativeNorm.Y) );
				relativeHorz.X=-relativeNorm.Y*relativeHorz.Y/relativeNorm.X;
			}else if(!isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y)){
				relativeHorz.X=0.0;
				relativeHorz.Y=1.0;
			}else if(isEqZero(relativeNorm.X) && !isEqZero(relativeNorm.Y)){
				relativeHorz.X=-1.0;
				relativeHorz.Y=0.0;
			}else{
				relativeHorz.X=0.0;
				relativeHorz.Y=1.0;
			}
			relativeHorz.Z=0.0;
			relativeUp=FVector::CrossProduct(relativeHorz,relativeNorm);
			/*if(relativeUp.Z<0.0){
				UE_LOG(LogTemp, Warning, TEXT("** up vector down **"));
				relativeHorz=-relativeHorz;
				relativeUp=-relativeUp;
			}*/

			textRotationV = relativeNorm;
			textLocation = relativeLocation + 0.5;//0.002*(CameraLocation - relativeLocation);
			textRotation = textRotationV.Rotation();
			if(isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y)){
				textRotation.Yaw=0.0;
			}
			//
			break;
		}
	}
	//==================
	for(int i=0;i<4;i++){
		FVector shiftV=corner[i]-relativeLocation;
		float nl=distanceInVector(relativeNorm,shiftV);
		float hl=distanceInVector(relativeHorz,shiftV);
		float ul=distanceInVector(-relativeUp,shiftV);

		projectedCorner[i].X=nl;
		projectedCorner[i].Y=hl;
		projectedCorner[i].Z=ul;
	}
	//
	/*int mostCenterPoint=0;
	if(projectedCorner[0].X>projectedCorner[1].X && projectedCorner[0].Y<projectedCorner[2].Y){
		mostCenterPoint=0;
		LeftThred=projectedCorner[0].X;
		TopThred=projectedCorner[0].Y;
		BottomThred=projectedCorner[3].Y+(-projectedCorner[3].X)*(projectedCorner[1].Y-projectCorner[3].Y)/(projectedCorner[1].X-projectCorner[3].X);
		RightThred=projectedCorner[3].X+(BottomThred-projectCorner[3].Y)*(projectCorner[2].X-projectCorner[3].X)/(projectCorner[2].Y-projectCorner[3].Y);
	}else if(projectedCorner[1].X>projectedCorner[0].X && projectedCorner[1].Y>projectedCorner[3].Y){
		mostCenterPoint=1;

	}else if(projectedCorner[2].X<projectedCorner[3].X && projectedCorner[2].Y<projectedCorner[0].Y){
		mostCenterPoint=2;
	}else{
		mostCenterPoint=3;
	}*/
	if(projectedCorner[0].Y<projectedCorner[1].Y){
		LeftThred=projectedCorner[0].Y;
	}else{
		LeftThred=projectedCorner[1].Y;
	}
	if(projectedCorner[0].Z<projectedCorner[2].Z){
		TopThred=projectedCorner[0].Z;
	}else{
		TopThred=projectedCorner[2].Z;
	}
	if(projectedCorner[1].Z>projectedCorner[3].Z){
		BottomThred=projectedCorner[1].Z;
	}else{
		BottomThred=projectedCorner[3].Z;
	}
	if(projectedCorner[2].Y>projectedCorner[3].Y){
		RightThred=projectedCorner[2].Y;
	}else{
		RightThred=projectedCorner[3].Y;
	}
	/*
	//=========
	for(int i=0;i<4;i++){
		FVector shiftV=corner[i]-relativeLocation;
		float nl=distanceInVector(relativeNorm,shiftV);
		float hl=distanceInVector(relativeHorz,shiftV);
		float ul=distanceInVector(-relativeUp,shiftV);

		if(hl>LeftThred){
			hl=LeftThred;
		}else if(hl<RightThred){
			hl=RightThred;
		}
		if(ul>TopThred){
			ul=TopThred;
		}else if(ul<BottomThred){
			ul=BottomThred;
		}
		shiftV=nl*relativeNorm+hl*relativeHorz-ul*relativeUp;
		corner[i]=relativeLocation+shiftV;
	}

	*/



	/*if(corner[0].Y<corner[1].Y){
		LeftThred=corner[0].Y;
	}else{
		LeftThred=corner[1].Y;
	}
	if(corner[0].Z<corner[2].Z){
		TopThred=corner[0].Z;
	}else{
		TopThred=corner[2].Z;
	}
	if(corner[1].Z>corner[3].Z){
		BottomThred=corner[1].Z;
	}else{
		BottomThred=corner[3].Z;
	}
	if(corner[2].Y>corner[3].Y){
		RightThred=corner[2].Y;
	}else{
		RightThred=corner[3].Y;
	}*/

	for(int i=0;i<4;i++){
		UE_LOG(LogTemp, Warning, TEXT("relativeCorner : %.3f %.3f %.3f"), projectedCorner[i].X,projectedCorner[i].Y,projectedCorner[i].Z);
	}
	//=========
	UE_LOG(LogTemp, Warning, TEXT("Thred : %.3f %.3f %.3f %.3f"), LeftThred, RightThred, TopThred, BottomThred);
	UE_LOG(LogTemp, Warning, TEXT("relativeNorm : %.3f %.3f %.3f"), relativeNorm.X,relativeNorm.Y,relativeNorm.Z);
			
	UE_LOG(LogTemp, Warning, TEXT("relativeHorz : %.3f %.3f %.3f"), relativeHorz.X,relativeHorz.Y,relativeHorz.Z);
	
	UE_LOG(LogTemp, Warning, TEXT("relativeUp : %.3f %.3f %.3f"), relativeUp.X,relativeUp.Y,relativeUp.Z);
			
	//====================================================================================
	
	/* This is now assured in bbox_generator
	bool allHitInstanceSame=true;
	for(int i=0;i<4;i++){
		if(cornerHitInstanceName[i] != cornerHitInstanceName[(i+1)%4]){
			allHitInstanceSame=false;
			break;
			//return false;
		}
	}*/
	// ++ // UE_LOG(LogTemp, Warning, TEXT("hit: %.3f %.3f %.3f"), relativeLocation.X, relativeLocation.Y, relativeLocation.Z);
	// ++ // UE_LOG(LogTemp, Warning, TEXT("hit normal: %.3f %.3f %.3f"), relativeNorm.X, relativeNorm.Y, relativeNorm.Z);
	//if(!allHitInstanceSame){}
	FVector normOf012,normOf123, normQuad;
	FVector elementLocation;
	normOf012=FVector::CrossProduct(corner[1]-corner[0], corner[2]-corner[0]);
	if(isEqZero(normOf012.X+normOf012.Y+normOf012.Z))
		return false;
	if(FVector::DotProduct(normOf012, CameraLocation-corner[1]) )
		normOf012=-normOf012;
	normOf123=FVector::CrossProduct(corner[2]-corner[3], corner[1]-corner[3]);
	if(isEqZero(normOf123.X+normOf123.Y+normOf123.Z))
		return false;
	if(FVector::DotProduct(normOf123, CameraLocation-corner[1]) )
		normOf123=-normOf123;
	normQuad=normOf012/normOf012.Size() + normOf123/normOf123.Size();
	normQuad.Y=0.0;
	if(isEqZero(normQuad.X+normQuad.Y+normQuad.Z))
		return false;
	normQuad/=normQuad.Size();
	UE_LOG(LogTemp, Warning, TEXT("norm012: %.3f %.3f %.3f"),normOf012.X/normOf012.Size(), normOf012.Y/normOf012.Size(), normOf012.Z/normOf012.Size());
	UE_LOG(LogTemp, Warning, TEXT("norm123: %.3f %.3f %.3f"),normOf123.X/normOf123.Size(), normOf123.Y/normOf123.Size(), normOf123.Z/normOf123.Size());
	UE_LOG(LogTemp, Warning, TEXT("normQuad: %.3f %.3f %.3f"),normQuad.X, normQuad.Y, normQuad.Z);
	float QuadLen=100.0;
	/*float** vertexLens;
	vertexLens=new float*[Z_COUNT];
	for(int i=0;i<Z_COUNT;i++){
		vertexLens[i]=new float[Y_COUNT+1];
	}*/

	int OutQuad=0;
	bool currentInQuad=false;
	bool beforeQuad,afterQuad;
	for(int i=0;i<Z_COUNT;i++){
		FVector edgeV1, edgeV2, interV;
		FVector oldEdgeV1, oldEdgeV2, oldInterV;
		edgeV1=corner[0]+((float)i)*(corner[1]-corner[0])/((float)Z_COUNT);
		edgeV2=corner[2]+((float)i)*(corner[3]-corner[2])/((float)Z_COUNT);
		// ++ // UE_LOG(LogTemp, Warning, TEXT("edgev1: %.3f %.3f %.3f"),edgeV1.X, edgeV1.Y, edgeV1.Z);
		if(i>0){
			vertexLens[i][0]=(edgeV1-oldEdgeV1).Size();
			vertexLens[i][Y_COUNT]=(edgeV2-oldEdgeV2).Size();
		}
		else{
			vertexLens[i][0]=0.0;
			vertexLens[i][Y_COUNT]=0.0;
		}
		oldEdgeV1=edgeV1;
		oldEdgeV2=edgeV2;	

		OutQuad=0;
		beforeQuad=true;
		afterQuad=false;

		for(int j=0;j<Y_COUNT;j++){
			interV=edgeV1+((float)j)*(edgeV2-edgeV1)/((float)Y_COUNT);
			//
			TraceStart=interV+normQuad*QuadLen;
			TraceEnd=interV-50.0*normQuad;

			TraceStart=CameraLocation;
			TraceEnd=CameraLocation+1.1*(interV-CameraLocation);


			if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams)) {
				FVector fv=TraceStart-Hit.Location;
				elementLocation = Hit.Location + 0.3*fv/fv.Size();
				FVector elementVector = elementLocation-CameraLocation;
				//SpawnRotation.RotateVector(GunOffset);
				//float exl=fabs(distanceInVector(CameraRotation.RotateVector(FVector(1.0,0.0,0.0)), elementVector));
				//float eyl=fabs(distanceInVector(CameraRotation.RotateVector(FVector(0.0,1.0,0.0)), elementVector));
				//float ezl=fabs(distanceInVector(CameraRotation.RotateVector(FVector(0.0,0.0,1.0)), elementVector));

				float exl=distanceInVector(CameraRotation.RotateVector(FVector(1.0,0.0,0.0)), elementVector);
				float eyl=distanceInVector(CameraRotation.RotateVector(FVector(0.0,1.0,0.0)), elementVector);
				float ezl=distanceInVector(CameraRotation.RotateVector(FVector(0.0,0.0,1.0)), elementVector);
				//FVector aaa=CameraRotation.RotateVector(FVector(1.0,0.0,0.0));
				//UE_LOG(LogTemp, Warning, TEXT("norm ratated: %.3f  %.3f  %.3f"), aaa.X, aaa.Y, aaa.Z);
				float eAngle0=atan(eyl/exl)*360.0/PAI;
				float eAngle1=atan(ezl/exl)*360.0/PAI;
				if(eAngle0<1.2*minAngle0 || eAngle0>1.2*maxAngle0 || eAngle1<1.2*minAngle1 || eAngle1>1.2*maxAngle1){
					//UE_LOG(LogTemp, Warning, TEXT(" angle1 out."));
					bool sechitflag=GetWorld()->LineTraceSingleByChannel(Hit, elementLocation, TraceEnd, ECC_Visibility, TraceParams);
					if(sechitflag){
						elementLocation=Hit.Location;
					}else if(j>0){
						//elementLocation=interV;
						FVector T=TraceEnd-elementLocation;
						T/=T.Size();
						float t=((elementLocation.X-oldInterV.X)*T.X+(elementLocation.Y-oldInterV.Y)*T.Y+(elementLocation.Z-oldInterV.Z)*T.Z)/(T.X*T.X+T.Y*T.Y+T.Z*T.Z);
						elementLocation=elementLocation+t*T;
					}else{
						elementLocation=interV;
					}
				}
				//UE_LOG(LogTemp, Warning, TEXT("element angle: %.3f  %.3f"), eAngle0,eAngle1);
				//UE_LOG(LogTemp, Warning, TEXT("hit: %s"), *Hit.GetActor()->GetName());
			}else if(j>0){
				FVector T=TraceEnd-elementLocation;
				T/=T.Size();
				float t=((elementLocation.X-oldInterV.X)*T.X+(elementLocation.Y-oldInterV.Y)*T.Y+(elementLocation.Z-oldInterV.Z)*T.Z)/(T.X*T.X+T.Y*T.Y+T.Z*T.Z);
				elementLocation=elementLocation+t*T;
			}else{
				elementLocation=interV;
			}
			//
			FVector shiftV=elementLocation-relativeLocation;
			float nl=distanceInVector(relativeNorm,shiftV);
			float hl=distanceInVector(relativeHorz,shiftV);
			float ul=distanceInVector(-relativeUp,shiftV);
			//UE_LOG(LogTemp, Warning, TEXT("element: %.3f %.3f %.3f"),elementLocation.X, elementLocation.Y, elementLocation.Z);
			//UE_LOG(LogTemp, Warning, TEXT("shiftv: %.3f %.3f %.3f"),nl,hl,ul);
			//UE_LOG(LogTemp, Warning, TEXT("shiftv: %.3f %.3f %.3f"),shiftV.X, shiftV.Y, shiftV.Z);
			/*currentInQuad=true;
			if(hl>LeftThred){
				hl=LeftThred;
				currentInQuad=false;
				//elementLocation.
			}else if(hl<RightThred){
				hl=RightThred;
				currentInQuad=false;
			}
			if(ul>TopThred){
				ul=TopThred;
				currentInQuad=false;
			}else if(ul<BottomThred){
				ul=BottomThred;
				currentInQuad=false;
			}

			//if current online & beforeQuad
			//then update x vertices before. x=(i*Y_COUNT)+j - (i*_Y_COUNT)
			//else if current online
			//then add x vertices.
			//else, just add one

			if(currentInQuad==true && OutQuad>0){
				for(int on=0;on<OutQuad;on++){
					vertices.Add(FVector(nl,hl,ul));
				}
			}else if(currentInQuad==true && OutQuad==0){
				vertices.Add(FVector(nl,hl,ul));
			}else{//} if(currentInQuad==false){
				++OutQuad;
			}
			
			
			shiftV=nl*relativeNorm+hl*relativeHorz-ul*relativeUp;
			elementLocation=relativeLocation+shiftV;*/

			//===
			vertices.Add(FVector(nl,hl,ul));
			//vertices.Add(shiftV);
			normals.Add(FVector(1,0,0));
			//***********************************
			//UV0.Add( FVector2D((float)j/(float)Y_COUNT, (float)i/(float)Z_COUNT) );
			tangents.Add(FProcMeshTangent(0, 1, 0));
			vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
			//
			if(j>0){
				vertexLens[i][j]=(elementLocation-oldInterV).Size();
			}
			oldInterV=elementLocation;
		}
	}
	
	for(int i=0;i<Z_COUNT;i++){
		if(i>0){
			vertexLens[i][0]=(vertices[i*Y_COUNT+0]-vertices[(i-1)*Y_COUNT+0]).Size();
			vertexLens[i][Y_COUNT]=(vertices[i*Y_COUNT+Y_COUNT-1]-vertices[(i-1)*Y_COUNT+Y_COUNT-1]).Size();
		}else{
			vertexLens[i][0]=vertexLens[i][Y_COUNT]=0.0;
		}
		//
		for(int j=1;j<Y_COUNT;j++){
			vertexLens[i][j]=(vertices[i*Y_COUNT+j]-vertices[i*Y_COUNT+j-1]).Size();
		}
	}
	//leftLen=rightLen=0.0;
	for(int i=0;i<Z_COUNT;i++){
		if(i>0){
			vertexLens[i][0]+=vertexLens[i-1][0];
			vertexLens[i][Y_COUNT]+=vertexLens[i-1][Y_COUNT];
		}else{
			vertexLens[i][0]=vertexLens[i][Y_COUNT]=0.0;
		}
		for(int j=2;j<Y_COUNT;j++){
			vertexLens[i][j]+=vertexLens[i][j-1];
		}
	}

	//vertexLens[Z_COUNT-1][0], vertexLens[0][Y_COUNT-1] ,vertexLens[Z_COUNT-1][Y_COUNT], vertexLens[Z_COUNT-1][Y_COUNT-1]

	for(int i=0;i<Z_COUNT;i++){
		vertexLens[i][0] /= vertexLens[Z_COUNT-1][0];
		vertexLens[i][Y_COUNT] /= vertexLens[Z_COUNT-1][Y_COUNT];
		UV0.Add( FVector2D(0.0,vertexLens[i][0]) );
		for(int j=1;j<Y_COUNT;j++){
			float xRate = vertexLens[i][j]/vertexLens[i][Y_COUNT-1];
			UV0.Add( FVector2D(xRate, vertexLens[i][0] + xRate*(vertexLens[i][Y_COUNT]-vertexLens[i][0])) );
			//UE_LOG(LogTemp, Warning, TEXT("UV-%d-%d: %.3f %.3f"),i,j,xRate, vertexLens[i][0] + xRate*(vertexLens[i][Y_COUNT]-vertexLens[i][0]));
		}
	}
	for(int i=0;i<bboxNum;++i){
		float corner2D[4][2];
		corner2D[0][0]=bbox2D[i][0];
		corner2D[0][1]=bbox2D[i][1];
		corner2D[1][0]=bbox2D[i][0];
		corner2D[1][1]=bbox2D[i][3];
		corner2D[2][0]=bbox2D[i][2];
		corner2D[2][1]=bbox2D[i][1];
		corner2D[3][0]=bbox2D[i][2];
		corner2D[3][1]=bbox2D[i][3];
		float cur_value;
		int targetY,targetZ;
		for(int j=0;j<4;++j){
			//find min distance between 4 corner point of corner2D[j]
			
			//find 3D coordinate

			//store in bboxAngle[i][j]
			float cornerRate_y = corner2D[j][0]/float(textureW);
			float cornerRate_z = corner2D[j][1]/float(textureH);
			//UE_LOG(LogUnrealCV, Warning, TEXT("  BBOX   %.3f %.3f %d %d"), cornerRate_y, cornerRate_z, textureW, textureH);
			float yRate,zRate;
			bool foundT=false;
			for(int zi=1;zi<Z_COUNT;++zi){
				//if(cornerRate_y<)
				for(int yi=1;yi<Y_COUNT;++yi){
					yRate = vertexLens[zi][yi]/vertexLens[zi][Y_COUNT-1];
					zRate = vertexLens[zi][0] + yRate*(vertexLens[zi][Y_COUNT]-vertexLens[zi][0]);
					if(cornerRate_y < yRate && cornerRate_z < zRate){
						targetZ=zi;
						targetY=yi;
						foundT=true;
						break;
					}
				}
				if(foundT)
					break;
			}
			if(!foundT){
				UE_LOG(LogUnrealCV, Warning, TEXT("NOOOOOOOO"));
			}else{
				float range[4][2];
				if(targetY==1)
					yRate=0.0;
				else
					yRate=vertexLens[targetZ-1][targetY-1]/vertexLens[targetZ-1][Y_COUNT-1];
				zRate = vertexLens[targetZ-1][0] + yRate*(vertexLens[targetZ-1][Y_COUNT]-vertexLens[targetZ-1][0]);
				range[0][0]=yRate;
				range[0][1]=zRate;
				if(targetY==1)
					yRate=0.0;
				else
					yRate=vertexLens[targetZ][targetY-1]/vertexLens[targetZ][Y_COUNT-1];
				zRate = vertexLens[targetZ][0] + yRate*(vertexLens[targetZ][Y_COUNT]-vertexLens[targetZ][0]);
				range[1][0]=yRate;
				range[1][1]=zRate;
				yRate=vertexLens[targetZ-1][targetY]/vertexLens[targetZ-1][Y_COUNT-1];
				zRate = vertexLens[targetZ-1][0] + yRate*(vertexLens[targetZ-1][Y_COUNT]-vertexLens[targetZ-1][0]);
				range[2][0]=yRate;
				range[2][1]=zRate;
				yRate=vertexLens[targetZ][targetY]/vertexLens[targetZ][Y_COUNT-1];
				zRate = vertexLens[targetZ][0] + yRate*(vertexLens[targetZ][Y_COUNT]-vertexLens[targetZ][0]);
				range[3][0]=yRate;
				range[3][1]=zRate;
				//
				float curRate=(range[0][1]-cornerRate_z)/(range[0][1]-range[1][1]);
				FVector interL=vertices[Y_COUNT*(targetZ-1)+(targetY-1)]+curRate*(vertices[Y_COUNT*(targetZ)+(targetY-1)]-vertices[Y_COUNT*(targetZ-1)+(targetY-1)]);
				curRate=(range[2][1]-cornerRate_z)/(range[2][1]-range[3][1]);
				FVector interR=vertices[Y_COUNT*(targetZ-1)+(targetY)]+curRate*(vertices[Y_COUNT*(targetZ)+(targetY)]-vertices[Y_COUNT*(targetZ-1)+(targetY)]);
				curRate=(range[0][0]-cornerRate_y)/(range[0][0]-range[2][0]);
				FVector interM=interL+curRate*(interR-interL);
				interM+=relativeLocation;
				bboxAngle[i][j][0]=interM.X;
				bboxAngle[i][j][1]=interM.Y;
				bboxAngle[i][j][2]=interM.Z;
				UE_LOG(LogUnrealCV, Warning, TEXT("  BBOX   %.3f %.3f %.3f"), interM.X, interM.Y, interM.Z);
			}
			//
		}
	}

	for (int i = 0;i < Y_COUNT - 1;i++) {
		for (int j = 0;j < Z_COUNT - 1;j++) {
			Triangles.Add(Z_COUNT*i + j);
			//Triangles.Add(Z_COUNT*i + j + 1);
			Triangles.Add(Z_COUNT*(i + 1) + j);
			Triangles.Add(Z_COUNT*i + j + 1);				
			//
			Triangles.Add(Z_COUNT*(i + 1) + j + 1);
			//Triangles.Add(Z_COUNT*(i + 1) + j);
			Triangles.Add(Z_COUNT*i + j + 1);
			Triangles.Add(Z_COUNT*(i + 1) + j);
		}
	}


	mesh->CreateMeshSection_LinearColor(0, vertices, Triangles, normals, UV0, vertexColors, tangents, true);

	// Enable collision data
	mesh->ContainsPhysicsTriMeshData(true);
	return true;
}

bool APugTextPawn::CreateTriangleSameInstance()
{
	//float angle[4][2];
	angle[0][0]=-5.0;
	angle[0][1]=2.5;
	angle[1][0]=-5.0;
	angle[1][1]=-2.5;
	//
	angle[2][0]=5.0;
	angle[2][1]=2.5;
	angle[3][0]=5.0;
	angle[3][1]=-2.5;
	//
	FVector corner[4];
	FString cornerHitInstanceName[4];
	TArray<FString> elementHitHistory;
	TArray<int> elementHitCount;
	TArray<float> elementHitAngle0;
	TArray<float> elementHitAngle1;
	FVector cornerNorm[4];
	bool hitFlag[4];
	//
	FVector relativeNorm, relativeHorz, relativeUp, relativeLocation;

	vertices.Empty();
	Triangles.Empty();
	normals.Empty();
	UV0.Empty();
	tangents.Empty();
	vertexColors.Empty();
	//TArray<FVector> vertices;
	bool bIsMatinee = false;

	float actorL = 100000.0;
	//UE_LOG(LogTemp, Warning, TEXT("camera position: %.3f %.3f %.3f"), CameraLocation.X, CameraLocation.Y, CameraLocation.Z);
	//===================================================================================
	float minAngle0,maxAngle0,minAngle1,maxAngle1;
	minAngle0=minAngle1=1000.0;
	maxAngle0=maxAngle1=-1000.0;
	for(int i=0;i<4;i++){
		if(angle[i][0]<minAngle0)
			minAngle0=angle[i][0];
		if(angle[i][0]>maxAngle0)
			maxAngle0=angle[i][0];
		if(angle[i][1]<minAngle1)
			minAngle1=angle[i][1];
		if(angle[i][1]>maxAngle1)
			maxAngle1=angle[i][1];
	}
	//==============
	FCollisionQueryParams TraceParams(FName(TEXT("TraceUsableActor")), true, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnFaceIndex = true;

	FHitResult Hit(ForceInit);
	FVector TraceEnd;
	FVector TraceStart;
	TraceStart = CameraLocation;
	
	//for(int i=0;i<4;i++){
	//	UE_LOG(LogTemp, Warning, TEXT("corner %d: %.3f %.3f %.3f"), i, corner[i].X,corner[i].Y,corner[i].Z);
	//	UE_LOG(LogTemp, Warning, TEXT("angle %d: %.3f %.3f"), i, angle[i][0], angle[i][1]);
	//}
			
	//====================================================================================
	// ++ // UE_LOG(LogTemp, Warning, TEXT("hit: %.3f %.3f %.3f"), relativeLocation.X, relativeLocation.Y, relativeLocation.Z);
	// ++ // UE_LOG(LogTemp, Warning, TEXT("hit normal: %.3f %.3f %.3f"), relativeNorm.X, relativeNorm.Y, relativeNorm.Z);
	//if(!allHitInstanceSame){}

	elementHitHistory.Add("None");
	elementHitCount.Add(0);

	float z_angleStep1=angle[1][1]-angle[0][1];
	float z_angleStep2=angle[3][1]-angle[2][1];
	float y_angleStep1=angle[1][0]-angle[0][0];
	float y_angleStep2=angle[3][0]-angle[2][0];
	for(int i=0;i<Z_COUNT;i++){	
		float z_angle1=angle[0][1]+z_angleStep1*((float)i)/((float)Z_COUNT);
		float z_angle2=angle[2][1]+z_angleStep2*((float)i)/((float)Z_COUNT);
		float y_angle1=angle[0][0]+y_angleStep1*((float)i)/((float)Z_COUNT);
		float y_angle2=angle[2][0]+y_angleStep2*((float)i)/((float)Z_COUNT);
		for(int j=0;j<Y_COUNT;j++){
			float interYAngle=y_angle2*((float)j)/((float)Y_COUNT) + y_angle1*((float)(Z_COUNT-j))/((float)Z_COUNT);
			float interZAngle=z_angle2*((float)j)/((float)Y_COUNT) + z_angle1*((float)(Z_COUNT-j))/((float)Z_COUNT);
			
			TraceEnd=CameraLocation;
			TraceEnd.X += actorL * (cos(PAI * (CameraRotation.Pitch+interZAngle) / 180.0) * cos(PAI * (CameraRotation.Yaw+interYAngle) / 180.0));
			TraceEnd.Y += actorL * (cos(PAI * (CameraRotation.Pitch+interZAngle) / 180.0) * sin(PAI * (CameraRotation.Yaw+interYAngle) / 180.0));
			TraceEnd.Z += actorL * (sin(PAI * (CameraRotation.Pitch+interZAngle) / 180.0));
	

			if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams)) {
				FString hitInst=Hit.GetActor()->GetName();
				//UE_LOG(LogTemp, Warning, TEXT("hit instance %d %d:"),i,j);
				//elementHitHistory
				bool foundExist=false;
				for(int k=1;k<elementHitHistory.Num();k++){
					if(elementHitHistory[k] == hitInst){
						++elementHitCount[k];
						foundExist=true;
						break;
					}
				}
				if(!foundExist){
					elementHitHistory.Add(hitInst);
					elementHitCount.Add(1);
					elementHitAngle0.Add(interYAngle);
					elementHitAngle1.Add(interZAngle);
				}
				//
				//elementHitAngle0.Add();
			}else{
				//elementHitInstanceName[i][j]=TEXT("None");
				++elementHitCount[0];
			}
		}
	}
	//
	//UE_LOG(LogTemp, Warning, TEXT("hit instance num %d"), elementHitHistory.Num());
	//UE_LOG(LogTemp, Warning, TEXT("hit instance num %d"), elementHitAngle0.Num());
	//UE_LOG(LogTemp, Warning, TEXT("hit instance num %d"), elementHitHistory.Num());
	int maxHitInstanceCount=0;
	float maxHitInstanceAngle[2];
	for(int k=1;k<elementHitHistory.Num();k++){
		if(elementHitCount[k]>maxHitInstanceCount){
			maxHitInstanceCount=elementHitCount[k];
			maxHitInstanceAngle[0]=elementHitAngle0[k-1];
			maxHitInstanceAngle[1]=elementHitAngle1[k-1];
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("max hit %d: %.3f %.3f"), maxHitInstanceCount, maxHitInstanceAngle[0], maxHitInstanceAngle[1]);
	//===========================================================================
	/*float** vertexLens;
	vertexLens=new float*[Z_COUNT];
	for(int i=0;i<Z_COUNT;i++){
		vertexLens[i]=new float[Y_COUNT+1];
	}*/


	//leftLen=rightLen=0.0;
	for(int i=0;i<Z_COUNT;i++){
		if(i>0){
			vertexLens[i][0]+=vertexLens[i-1][0];
			vertexLens[i][Y_COUNT]+=vertexLens[i-1][Y_COUNT];
		}else{
			vertexLens[i][0]=vertexLens[i][Y_COUNT]=0.0;
		}
		for(int j=2;j<Y_COUNT;j++){
			vertexLens[i][j]+=vertexLens[i][j-1];
		}
	}

	for(int i=0;i<Z_COUNT;i++){
		vertexLens[i][0] /= vertexLens[Z_COUNT-1][0];
		vertexLens[i][Y_COUNT] /= vertexLens[Z_COUNT-1][Y_COUNT];
		UV0.Add( FVector2D(0.0,vertexLens[i][0]) );
		for(int j=1;j<Y_COUNT;j++){
			float xRate = vertexLens[i][j]/vertexLens[i][Y_COUNT-1];
			UV0.Add( FVector2D(xRate, vertexLens[i][0] + xRate*(vertexLens[i][Y_COUNT]-vertexLens[i][0])) );
			//UE_LOG(LogTemp, Warning, TEXT("UV-%d-%d: %.3f %.3f"),i,j,xRate, vertexLens[i][0] + xRate*(vertexLens[i][Y_COUNT]-vertexLens[i][0]));
		}
	}

	for (int i = 0;i < Y_COUNT - 1;i++) {
		for (int j = 0;j < Z_COUNT - 1;j++) {
			Triangles.Add(Z_COUNT*i + j);
			//Triangles.Add(Z_COUNT*i + j + 1);
			Triangles.Add(Z_COUNT*(i + 1) + j);
			Triangles.Add(Z_COUNT*i + j + 1);				
			//
			Triangles.Add(Z_COUNT*(i + 1) + j + 1);
			//Triangles.Add(Z_COUNT*(i + 1) + j);
			Triangles.Add(Z_COUNT*i + j + 1);
			Triangles.Add(Z_COUNT*(i + 1) + j);
		}
	}



	//mesh->CreateMeshSection_LinearColor(0, vertices, Triangles, normals, UV0, vertexColors, tangents, true);

	// Enable collision data
	//mesh->ContainsPhysicsTriMeshData(true);

	return true;
}


bool APugTextPawn::CreateTriangleCameraLight(){
	vertices.Empty();
	Triangles.Empty();
	normals.Empty();
	UV0.Empty();
	tangents.Empty();
	vertexColors.Empty();

	float mid_0=(inp[0]+inp[2])*0.5;
	float mid_1=(inp[1]+inp[3])*0.5;
	angle[0][0]=inp[0]+(mid_0-inp[0])*0.01;
	angle[0][1]=inp[1]+(mid_1-inp[1])*0.01;
	angle[1][0]=inp[0]+(mid_0-inp[0])*0.01;
	angle[1][1]=inp[3]+(mid_1-inp[3])*0.01;
	angle[2][0]=inp[2]+(mid_0-inp[2])*0.01;
	angle[2][1]=inp[1]+(mid_1-inp[1])*0.01;
	angle[3][0]=inp[2]+(mid_0-inp[2])*0.01;
	angle[3][1]=inp[3]+(mid_1-inp[3])*0.01;

	FVector corner[4];
	FVector projectedCorner[4];
	FString cornerHitInstanceName;
	FVector cornerNorm[4];
	FVector relativeNorm, relativeHorz, relativeUp, relativeLocation;
	bool hitFlag[4];

	bool bIsMatinee = false;

	float actorL = 1000000000.0;
	FCollisionQueryParams TraceParams(FName(TEXT("TraceUsableActor")), true, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnFaceIndex = true;

	FHitResult Hit(ForceInit);
	FVector TraceEnd;
	FVector TraceStart;
	TraceStart = CameraLocation;

	for(int i=0;i<4;i++)
		hitFlag[i]=false;
	for(int i=0;i<4;i++){
			TraceEnd = CameraLocation;
			TraceEnd.X += actorL * (cos(PAI * (CameraRotation.Pitch+angle[i][1]) / 180.0) * cos(PAI * (CameraRotation.Yaw+angle[i][0]) / 180.0));
			TraceEnd.Y += actorL * (cos(PAI * (CameraRotation.Pitch+angle[i][1]) / 180.0) * sin(PAI * (CameraRotation.Yaw+angle[i][0]) / 180.0));
			TraceEnd.Z += actorL * (sin(PAI * (CameraRotation.Pitch+angle[i][1]) / 180.0));
			// ++ // UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %.3f %.3f %.3f"), i, TraceEnd.X,TraceEnd.Y,TraceEnd.Z);
			bool hitflag = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
			corner[i]=Hit.Location;
			cornerNorm[i]=Hit.ImpactNormal;
			hitFlag[i]=hitflag;
			//cornerHitInstanceName=Hit.GetActor()->GetName();
	}
	if(hitFlag[0]*hitFlag[1]*hitFlag[2]*hitFlag[3]==0){
		UE_LOG(LogTemp, Warning, TEXT("NO HIT!!!"));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("NO HIT"));
		return false;
	}
	//
	for(int i=0;i<4;i++){
			UE_LOG(LogTemp, Warning, TEXT("angle %d: %.3f %.3f"), i, angle[i][0], angle[i][1]);
			UE_LOG(LogTemp, Warning, TEXT("corner %d: %.3f %.3f %.3f"), i, corner[i].X,corner[i].Y,corner[i].Z);
			relativeLocation=corner[i];
			relativeNorm=cornerNorm[i];
			/*if(FVector::DotProduct(relativeNorm,(CameraLocation-corner[i]))<0.0){
				UE_LOG(LogTemp, Warning, TEXT("** swap norm vector **"));
				relativeNorm=-relativeNorm;
			}*/
			bool normUp=false;
			if(!isEqZero(relativeNorm.X) && !isEqZero(relativeNorm.Y)){
				relativeHorz.Y=fabs(relativeNorm.X / sqrt(relativeNorm.X*relativeNorm.X+relativeNorm.Y*relativeNorm.Y) );
				relativeHorz.X=-relativeNorm.Y*relativeHorz.Y/relativeNorm.X;
				relativeHorz.Z=0.0;
			}else if(!isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y)){
				relativeHorz.X=0.0;
				relativeHorz.Y=1.0;
				relativeHorz.Z=0.0;
			}else if(isEqZero(relativeNorm.X) && !isEqZero(relativeNorm.Y)){
				relativeHorz.X=-1.0;
				relativeHorz.Y=0.0;
				relativeHorz.Z=0.0;
			}else{
				// Norm vector (0,0,1)
				//relativeHorz.X=0.0;
				//relativeHorz.Y=1.0;
				relativeHorz=FVector::CrossProduct(relativeNorm,(CameraLocation-corner[i]));
				normUp=true;
			}
			relativeUp=FVector::CrossProduct(relativeNorm,relativeHorz);
			if(relativeUp.Z<-0.001 && normUp==false){
				UE_LOG(LogTemp, Warning, TEXT("** swap up vector **"));
				relativeHorz=-relativeHorz;
				relativeUp=-relativeUp;
			}

			textRotationV = relativeNorm;
			textLocation = relativeLocation;//0.002*(CameraLocation - relativeLocation);
			textRotation = textRotationV.Rotation();
			if(isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y)){
				textRotation.Yaw=0.0;
			}
			//
			break;
	}
	for(int i=0;i<4;i++){
		UE_LOG(LogTemp, Warning, TEXT("Corner : %.3f %.3f %.3f"), corner[i].X,corner[i].Y,corner[i].Z);
	}
	UE_LOG(LogTemp, Warning, TEXT("relativeNorm : %.3f %.3f %.3f"), relativeNorm.X,relativeNorm.Y,relativeNorm.Z);
			
	UE_LOG(LogTemp, Warning, TEXT("relativeHorz : %.3f %.3f %.3f"), relativeHorz.X,relativeHorz.Y,relativeHorz.Z);
	
	UE_LOG(LogTemp, Warning, TEXT("relativeUp : %.3f %.3f %.3f"), relativeUp.X,relativeUp.Y,relativeUp.Z);
	//
	float up1=distanceInVector(relativeUp,corner[0]-corner[1]);
	float up2=distanceInVector(relativeUp,corner[2]-corner[1]);
	UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), up1,up2);
	FVector dirV;
	float betweenAngle;
	if(up1<0.0 || up2<0.0)
		return false;
	if(up1>up2){
		dirV=(corner[1]-corner[0]);
		if(dirV.Size()==0.0)
			return false;
		betweenAngle=FVector::DotProduct(dirV,-relativeUp)/(dirV.Size()*relativeUp.Size());
		corner[0]+=(dirV/dirV.Size())*((up1-up2)/betweenAngle);
	}else{
		dirV=(corner[3]-corner[2]);
		if(dirV.Size()==0.0)
			return false;
		betweenAngle=FVector::DotProduct(dirV,-relativeUp)/(dirV.Size()*relativeUp.Size());
		corner[2]+=(dirV/dirV.Size())*((up2-up1)/betweenAngle);
	}
	//
	float left1=distanceInVector(relativeHorz,corner[0]-corner[2]);
	float left2=distanceInVector(relativeHorz,corner[1]-corner[2]);
	UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), left1,left2);
	if(left1<0.0 || left2<0.0)
		return false;
	if(left1>left2){
		dirV=(corner[2]-corner[0]);
		if(dirV.Size()==0.0)
			return false;
		betweenAngle=FVector::DotProduct(dirV,-relativeHorz)/(dirV.Size()*relativeHorz.Size());
		corner[0]+=(dirV/dirV.Size())*((left1-left2)/betweenAngle);
	}else{
		dirV=(corner[3]-corner[1]);
		if(dirV.Size()==0.0)
			return false;
		betweenAngle=FVector::DotProduct(dirV,-relativeHorz)/(dirV.Size()*relativeHorz.Size());
		corner[1]+=(dirV/dirV.Size())*((left2-left1)/betweenAngle);
	}
	//
	float bottom1=distanceInVector(-relativeUp,corner[1]-corner[0]);
	float bottom2=distanceInVector(-relativeUp,corner[3]-corner[0]);
	UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), bottom1,bottom2);
	if(bottom1<0.0 || bottom2<0.0)
		return false;
	if(bottom1>bottom2){
		dirV=(corner[0]-corner[1]);
		if(dirV.Size()==0.0)
			return false;
		betweenAngle=FVector::DotProduct(dirV,relativeUp)/(dirV.Size()*relativeUp.Size());
		corner[1]+=(dirV/dirV.Size())*((bottom1-bottom2)/betweenAngle);
	}else{
		dirV=(corner[2]-corner[3]);
		if(dirV.Size()==0.0)
			return false;
		betweenAngle=FVector::DotProduct(dirV,relativeUp)/(dirV.Size()*relativeUp.Size());
		corner[3]+=(dirV/dirV.Size())*((bottom2-bottom1)/betweenAngle);
	}
	//
	float right1=distanceInVector(-relativeHorz,corner[2]-corner[0]);
	float right2=distanceInVector(-relativeHorz,corner[3]-corner[0]);
	UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), right1,right2);
	if(right1<0.0 || right2<0.0)
		return false;
	if(right1>right2){
		dirV=(corner[0]-corner[2]);
		if(dirV.Size()==0.0)
			return false;
		betweenAngle=FVector::DotProduct(dirV,relativeHorz)/(dirV.Size()*relativeHorz.Size());
		corner[2]+=(dirV/dirV.Size())*((right1-right2)/betweenAngle);
	}else{
		dirV=(corner[1]-corner[3]);
		if(dirV.Size()==0.0)
			return false;
		betweenAngle=FVector::DotProduct(dirV,relativeHorz)/(dirV.Size()*relativeHorz.Size());
		corner[3]+=(dirV/dirV.Size())*((right2-right1)/betweenAngle);
	}
	//
	UE_LOG(LogTemp, Warning, TEXT("trac"));
	//CameraRotationV = CameraRotation.Rotation();
	//angle[0][0]=acos(FVector::DotProduct(corner[0]-CameraLocation,CameraRotationV)/((corner[0]-CameraLocation).Size()*CameraRotationV.Size()));
	FRotator corner0Rotator=(corner[0]-CameraLocation).Rotation();
	FRotator corner1Rotator=(corner[1]-CameraLocation).Rotation();
	FRotator corner2Rotator=(corner[2]-CameraLocation).Rotation();
	FRotator corner3Rotator=(corner[3]-CameraLocation).Rotation();
	angle[0][0]=corner0Rotator.Yaw-CameraRotation.Yaw;
	angle[0][1]=corner0Rotator.Pitch-CameraRotation.Pitch;
	angle[1][0]=corner1Rotator.Yaw-CameraRotation.Yaw;
	angle[1][1]=corner1Rotator.Pitch-CameraRotation.Pitch;
	angle[2][0]=corner2Rotator.Yaw-CameraRotation.Yaw;
	angle[2][1]=corner2Rotator.Pitch-CameraRotation.Pitch;
	angle[3][0]=corner3Rotator.Yaw-CameraRotation.Yaw;
	angle[3][1]=corner3Rotator.Pitch-CameraRotation.Pitch;
	//
	for(int i=0;i<4;i++){
			TraceEnd = CameraLocation+(corner[i]-CameraLocation)*1.5;
			// ++ // 
			UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %.3f %.3f %.3f"), i, TraceEnd.X,TraceEnd.Y,TraceEnd.Z);
			bool hitflag = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
			if(false == hitflag)
				return false;
			corner[i]=Hit.Location;
			UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %.3f"), i, corner[i].X);
			cornerNorm[i]=Hit.ImpactNormal;
			UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %.3f"), i, cornerNorm[i].X);
			hitFlag[i]=hitflag;
			UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %d"), i, hitflag);
			cornerHitInstanceName=Hit.GetActor()->GetName();
			UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %s"), i, *cornerHitInstanceName);
	}
	/*for(int i=0;i<4;i++){
			UE_LOG(LogTemp, Warning, TEXT("angle %d: %.3f %.3f"), i, angle[i][0], angle[i][1]);
			UE_LOG(LogTemp, Warning, TEXT("corner %d: %.3f %.3f %.3f"), i, corner[i].X,corner[i].Y,corner[i].Z);
			relativeLocation=corner[i];
			relativeNorm=cornerNorm[i];
			if(FVector::DotProduct(relativeNorm,(CameraLocation-corner[i]))<0.0){
				UE_LOG(LogTemp, Warning, TEXT("** swap norm vector **"));
				relativeNorm=-relativeNorm;
			}
			if(!isEqZero(relativeNorm.X) && !isEqZero(relativeNorm.Y)){
				relativeHorz.Y=fabs(relativeNorm.X / sqrt(relativeNorm.X*relativeNorm.X+relativeNorm.Y*relativeNorm.Y) );
				relativeHorz.X=-relativeNorm.Y*relativeHorz.Y/relativeNorm.X;
			}else if(!isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y)){
				relativeHorz.X=0.0;
				relativeHorz.Y=1.0;
			}else if(isEqZero(relativeNorm.X) && !isEqZero(relativeNorm.Y)){
				relativeHorz.X=-1.0;
				relativeHorz.Y=0.0;
			}else{
				relativeHorz.X=0.0;
				relativeHorz.Y=1.0;
			}
			relativeHorz.Z=0.0;
			relativeUp=FVector::CrossProduct(relativeNorm,relativeHorz);
			if(relativeUp.Z<-0.001){
				UE_LOG(LogTemp, Warning, TEXT("** swap up vector **"));
				relativeHorz=-relativeHorz;
				relativeUp=-relativeUp;
			}

			textRotationV = relativeNorm;
			textLocation = relativeLocation;//0.002*(CameraLocation - relativeLocation);
			textRotation = textRotationV.Rotation();
			if(isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y)){
				textRotation.Yaw=0.0;
			}
			//
			break;
	}*/

	for(int i=0;i<4;i++){
			UE_LOG(LogTemp, Warning, TEXT(" 2 angle %d: %.3f %.3f"), i, angle[i][0], angle[i][1]);
			UE_LOG(LogTemp, Warning, TEXT(" 2 corner %d: %.3f %.3f %.3f"), i, corner[i].X,corner[i].Y,corner[i].Z);
			relativeLocation=corner[i];
			relativeNorm=cornerNorm[i];
			if(FVector::DotProduct(relativeNorm,(CameraLocation-corner[i]))<0.0){
				//UE_LOG(LogTemp, Warning, TEXT("** swap norm vector **"));
				relativeNorm=-relativeNorm;
			}
			bool normUp=false;
			if(!isEqZero(relativeNorm.X) && !isEqZero(relativeNorm.Y)){
				relativeHorz.Y=fabs(relativeNorm.X / sqrt(relativeNorm.X*relativeNorm.X+relativeNorm.Y*relativeNorm.Y) );
				relativeHorz.X=-relativeNorm.Y*relativeHorz.Y/relativeNorm.X;
				relativeHorz.Z=0.0;
			}else if(!isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y)){
				relativeHorz.X=0.0;
				relativeHorz.Y=1.0;
				relativeHorz.Z=0.0;
			}else if(isEqZero(relativeNorm.X) && !isEqZero(relativeNorm.Y)){
				relativeHorz.X=-1.0;
				relativeHorz.Y=0.0;
				relativeHorz.Z=0.0;
			}else{
				// Norm vector (0,0,1)
				//relativeHorz.X=0.0;
				//relativeHorz.Y=1.0;
				relativeHorz=FVector::CrossProduct(relativeNorm,(CameraLocation-corner[i]));
				normUp=true;
			}
			relativeUp=FVector::CrossProduct(relativeNorm,relativeHorz);
			if(relativeUp.Z<-0.001 && normUp==false){
				//UE_LOG(LogTemp, Warning, TEXT("** swap up vector **"));
				relativeHorz=-relativeHorz;
				relativeUp=-relativeUp;
			}

			textRotationV = relativeNorm;
			textLocation = relativeLocation;//0.002*(CameraLocation - relativeLocation);
			textRotation = textRotationV.Rotation();
			if(isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y)){
				textRotation.Yaw=0.0;
			}
			//textRotation.Roll=0.0;
			//
			break;
	}

	for(int i=0;i<4;i++){
		UE_LOG(LogTemp, Warning, TEXT("relativeCorner : %.3f %.3f %.3f"), corner[i].X,corner[i].Y,corner[i].Z);
	}
	UE_LOG(LogTemp, Warning, TEXT("relativeNorm : %.3f %.3f %.3f"), relativeNorm.X,relativeNorm.Y,relativeNorm.Z);
			
	UE_LOG(LogTemp, Warning, TEXT("relativeHorz : %.3f %.3f %.3f"), relativeHorz.X,relativeHorz.Y,relativeHorz.Z);
	
	UE_LOG(LogTemp, Warning, TEXT("relativeUp : %.3f %.3f %.3f"), relativeUp.X,relativeUp.Y,relativeUp.Z);
	//==================================================
	FVector elementLocation;
	FVector old_left_ElementLocation;
	FVector old_right_ElementLocation;
	FVector old_inter_location;
	/*float** vertexLens;
	vertexLens=new float*[Z_COUNT];
	for(int i=0;i<Z_COUNT;i++){
		vertexLens[i]=new float[Y_COUNT+1];
	}*/
	for(int i=0;i<Z_COUNT;i++){
		if(i==0){
			vertexLens[i][0]=0.0;
			vertexLens[i][Y_COUNT]=0.0;
			old_left_ElementLocation=FVector(0.0,0.0,0.0);
			old_right_ElementLocation=corner[2]-corner[0];
		}
		float curAngle1_left=angle[0][1]+(angle[1][1]-angle[0][1])*((float)i/(float)Z_COUNT);
		float curAngle1_right=angle[2][1]+(angle[3][1]-angle[2][1])*((float)i/(float)Z_COUNT);
		float curAngle0_left=angle[0][0]+(angle[1][0]-angle[0][0])*((float)i/(float)Z_COUNT);
		float curAngle0_right=angle[2][0]+(angle[3][0]-angle[2][0])*((float)i/(float)Z_COUNT);
		for(int j=0;j<Y_COUNT;j++){
			//float curAngle0=angle[0][0]+(angle[2][0]-angle[0][0])*((float)j/(float)Y_COUNT);
			TraceStart=CameraLocation;
			TraceEnd=CameraLocation;
			float curAngle0=curAngle0_left+(curAngle0_right-curAngle0_left)*((float)j/(float)Y_COUNT);
			float curAngle1=curAngle1_left+(curAngle1_right-curAngle1_left)*((float)j/(float)Y_COUNT);
			TraceEnd.X += actorL * (cos(PAI * (CameraRotation.Pitch + curAngle1) / 180.0) * cos(PAI * (CameraRotation.Yaw + curAngle0) / 180.0));
			TraceEnd.Y += actorL * (cos(PAI * (CameraRotation.Pitch + curAngle1) / 180.0 ) * sin(PAI * (CameraRotation.Yaw + curAngle0) / 180.0));
			TraceEnd.Z += actorL * (sin(PAI * (CameraRotation.Pitch + curAngle1) / 180.0));
			//
			if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams)) {
				elementLocation = Hit.Location;		
				elementLocation += 0.01*Hit.ImpactNormal/Hit.ImpactNormal.Size();			
				//UE_LOG(LogTemp, Warning, TEXT("hit: %s"), *Hit.GetActor()->GetName());
				FString curHitInstanceName=Hit.GetActor()->GetName();
				if(cornerHitInstanceName != curHitInstanceName){
					//UE_LOG(LogTemp, Warning, TEXT("different instance"));
					/*TraceStart=Hit.Location;
					if(GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams)){
						elementLocation = Hit.Location;	
						curHitInstanceName=Hit.GetActor()->GetName();
					}else{
						elementLocation = TraceEnd;
						break;
					}*/
				}
			}
			else {
				elementLocation = TraceEnd;
			}
			FVector shiftV=elementLocation-relativeLocation;
			float nl=distanceInVector(relativeNorm,shiftV);
			float hl=distanceInVector(relativeHorz,shiftV);
			float ul=distanceInVector(relativeUp,shiftV);

			vertices.Add(FVector(nl,hl,ul));
			//vertices.Add(shiftV);
			normals.Add(FVector(1,0,0));
			//***********************************
			//UV0.Add( FVector2D((float)j/(float)Y_COUNT, (float)i/(float)Z_COUNT) );
			tangents.Add(FProcMeshTangent(0, 1, 0));
			vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
			//
			if(j==0){
				vertexLens[i][0]=(elementLocation-old_left_ElementLocation).Size();
				old_left_ElementLocation=elementLocation;
				old_inter_location=elementLocation;
			}else if(j==Y_COUNT-1){
				vertexLens[i][Y_COUNT]=(elementLocation-old_right_ElementLocation).Size();
				old_right_ElementLocation=elementLocation;
				//
				vertexLens[i][j]=(elementLocation-old_inter_location).Size();
			}else{
				vertexLens[i][j]=(elementLocation-old_inter_location).Size();
				old_inter_location=elementLocation;
			}
		}
	}
	//
	for(int i=0;i<Z_COUNT;i++){
		if(i>0){
			vertexLens[i][0]=(vertices[i*Y_COUNT+0]-vertices[(i-1)*Y_COUNT+0]).Size();
			vertexLens[i][Y_COUNT]=(vertices[i*Y_COUNT+Y_COUNT-1]-vertices[(i-1)*Y_COUNT+Y_COUNT-1]).Size();
		}else{
			vertexLens[i][0]=vertexLens[i][Y_COUNT]=0.0;
		}
		//
		for(int j=1;j<Y_COUNT;j++){
			vertexLens[i][j]=(vertices[i*Y_COUNT+j]-vertices[i*Y_COUNT+j-1]).Size();
		}
	}
	//leftLen=rightLen=0.0;
	for(int i=0;i<Z_COUNT;i++){
		if(i>0){
			vertexLens[i][0]+=vertexLens[i-1][0];
			vertexLens[i][Y_COUNT]+=vertexLens[i-1][Y_COUNT];
		}else{
			vertexLens[i][0]=vertexLens[i][Y_COUNT]=0.0;
		}
		for(int j=2;j<Y_COUNT;j++){
			vertexLens[i][j]+=vertexLens[i][j-1];
		}
	}

	//vertexLens[Z_COUNT-1][0], vertexLens[0][Y_COUNT-1] ,vertexLens[Z_COUNT-1][Y_COUNT], vertexLens[Z_COUNT-1][Y_COUNT-1]

	for(int i=0;i<Z_COUNT;i++){
		vertexLens[i][0] /= vertexLens[Z_COUNT-1][0];
		vertexLens[i][Y_COUNT] /= vertexLens[Z_COUNT-1][Y_COUNT];
		UV0.Add( FVector2D(0.0,vertexLens[i][0]) );
		for(int j=1;j<Y_COUNT;j++){
			float xRate = vertexLens[i][j]/vertexLens[i][Y_COUNT-1];
			UV0.Add( FVector2D(xRate, vertexLens[i][0] + xRate*(vertexLens[i][Y_COUNT]-vertexLens[i][0])) );
			//UE_LOG(LogTemp, Warning, TEXT("UV-%d-%d: %.3f %.3f"),i,j,xRate, vertexLens[i][0] + xRate*(vertexLens[i][Y_COUNT]-vertexLens[i][0]));
		}
	}
	//
	for (int i = 0;i < Y_COUNT - 1;i++) {
		for (int j = 0;j < Z_COUNT - 1;j++) {
			Triangles.Add(Z_COUNT*i + j);
			//Triangles.Add(Z_COUNT*i + j + 1);
			Triangles.Add(Z_COUNT*(i + 1) + j);
			Triangles.Add(Z_COUNT*i + j + 1);				
			//
			Triangles.Add(Z_COUNT*(i + 1) + j + 1);
			//Triangles.Add(Z_COUNT*(i + 1) + j);
			Triangles.Add(Z_COUNT*i + j + 1);
			Triangles.Add(Z_COUNT*(i + 1) + j);
		}
	}


	mesh->CreateMeshSection_LinearColor(0, vertices, Triangles, normals, UV0, vertexColors, tangents, true);

	// Enable collision data
	mesh->ContainsPhysicsTriMeshData(true);
	return true;
}


bool APugTextPawn::CreateTriangleCameraLight2(){
	vertices.Empty();
	Triangles.Empty();
	normals.Empty();
	UV0.Empty();
	tangents.Empty();
	vertexColors.Empty();

	float mid_0=(inp[0]+inp[2])*0.5;
	float mid_1=(inp[1]+inp[3])*0.5;
	angle[0][0]=inp[0]+(mid_0-inp[0])*0.01;
	angle[0][1]=inp[1]+(mid_1-inp[1])*0.01;
	angle[1][0]=inp[0]+(mid_0-inp[0])*0.01;
	angle[1][1]=inp[3]+(mid_1-inp[3])*0.01;
	angle[2][0]=inp[2]+(mid_0-inp[2])*0.01;
	angle[2][1]=inp[1]+(mid_1-inp[1])*0.01;
	angle[3][0]=inp[2]+(mid_0-inp[2])*0.01;
	angle[3][1]=inp[3]+(mid_1-inp[3])*0.01;

	FVector corner[4];
	FVector projectedCorner[4];
	FString cornerHitInstanceName;
	FVector cornerNorm[4];
	FVector relativeNorm, relativeHorz, relativeUp, relativeLocation;
	bool hitFlag[4];

	bool bIsMatinee = false;

	float actorL = 1000000000.0;
	FCollisionQueryParams TraceParams(FName(TEXT("TraceUsableActor")), true, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnFaceIndex = true;

	FHitResult Hit(ForceInit);
	FVector TraceEnd;
	FVector TraceStart;
	TraceStart = CameraLocation;

	APawn* Pawn = FUE4CVServer::Get().GetPawn();
	APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
	FVector2D screenPos;

	for(int i=0;i<4;i++)
		hitFlag[i]=false;
	for(int i=0;i<4;i++){
		screenPos.X = angle[i][0];
		screenPos.Y = angle[i][1];
		if(PlayerController!=nullptr){
			PlayerController->GetHitResultAtScreenPosition(screenPos, ECC_Visibility, true, Hit);
			corner[i]=Hit.Location;
			cornerNorm[i]=Hit.ImpactNormal;
			if(Hit.GetActor() == nullptr)
				hitFlag[i]=false;
			else
				hitFlag[i]=true;
		}else{
			return false;
		}
		
		/*	TraceEnd = CameraLocation;
			TraceEnd.X += actorL * (cos(PAI * (CameraRotation.Pitch+angle[i][1]) / 180.0) * cos(PAI * (CameraRotation.Yaw+angle[i][0]) / 180.0));
			TraceEnd.Y += actorL * (cos(PAI * (CameraRotation.Pitch+angle[i][1]) / 180.0) * sin(PAI * (CameraRotation.Yaw+angle[i][0]) / 180.0));
			TraceEnd.Z += actorL * (sin(PAI * (CameraRotation.Pitch+angle[i][1]) / 180.0));
			// ++ // UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %.3f %.3f %.3f"), i, TraceEnd.X,TraceEnd.Y,TraceEnd.Z);
			bool hitflag = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
			corner[i]=Hit.Location;
			cornerNorm[i]=Hit.ImpactNormal;
			hitFlag[i]=hitflag;
			//cornerHitInstanceName=Hit.GetActor()->GetName();
		*/
	}
	if(hitFlag[0]*hitFlag[1]*hitFlag[2]*hitFlag[3]==0){
		//UE_LOG(LogTemp, Warning, TEXT("NO HIT!!!"));
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("NO HIT"));
		return false;
	}
	//
	for(int i=0;i<4;i++){
			//UE_LOG(LogTemp, Warning, TEXT("angle %d: %.3f %.3f"), i, angle[i][0], angle[i][1]);
			//UE_LOG(LogTemp, Warning, TEXT("corner %d: %.3f %.3f %.3f"), i, corner[i].X,corner[i].Y,corner[i].Z);
			relativeLocation=corner[i];
			relativeNorm=cornerNorm[i];
			/*if(FVector::DotProduct(relativeNorm,(CameraLocation-corner[i]))<0.0){
				UE_LOG(LogTemp, Warning, TEXT("** swap norm vector **"));
				relativeNorm=-relativeNorm;
			}*/
			bool normUp=false;
			if(!isEqZero(relativeNorm.X) && !isEqZero(relativeNorm.Y)){
				relativeHorz.Y=fabs(relativeNorm.X / sqrt(relativeNorm.X*relativeNorm.X+relativeNorm.Y*relativeNorm.Y) );
				relativeHorz.X=-relativeNorm.Y*relativeHorz.Y/relativeNorm.X;
				relativeHorz.Z=0.0;
			}else if(!isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y)){
				relativeHorz.X=0.0;
				relativeHorz.Y=1.0;
				relativeHorz.Z=0.0;
			}else if(isEqZero(relativeNorm.X) && !isEqZero(relativeNorm.Y)){
				relativeHorz.X=-1.0;
				relativeHorz.Y=0.0;
				relativeHorz.Z=0.0;
			}else{
				// Norm vector (0,0,1)
				//relativeHorz.X=0.0;
				//relativeHorz.Y=1.0;
				relativeHorz=FVector::CrossProduct(relativeNorm,(CameraLocation-corner[i]));
				normUp=true;
			}
			relativeUp=FVector::CrossProduct(relativeNorm,relativeHorz);
			if(relativeUp.Z<-0.001 && normUp==false){
				//UE_LOG(LogTemp, Warning, TEXT("** swap up vector **"));
				relativeHorz=-relativeHorz;
				relativeUp=-relativeUp;
			}

			textRotationV = relativeNorm;
			textLocation = relativeLocation;//0.002*(CameraLocation - relativeLocation);
			textRotation = textRotationV.Rotation();
			if(isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y)){
				textRotation.Yaw=0.0;
			}
			//
			break;
	}
	//for(int i=0;i<4;i++){
	//	UE_LOG(LogTemp, Warning, TEXT("Corner : %.3f %.3f %.3f"), corner[i].X,corner[i].Y,corner[i].Z);
	//}
	//UE_LOG(LogTemp, Warning, TEXT("relativeNorm : %.3f %.3f %.3f"), relativeNorm.X,relativeNorm.Y,relativeNorm.Z);
			
	//UE_LOG(LogTemp, Warning, TEXT("relativeHorz : %.3f %.3f %.3f"), relativeHorz.X,relativeHorz.Y,relativeHorz.Z);
	
	//UE_LOG(LogTemp, Warning, TEXT("relativeUp : %.3f %.3f %.3f"), relativeUp.X,relativeUp.Y,relativeUp.Z);
	//
	if(isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y) ){
		relativeUp=FVector::CrossProduct(corner[2]-corner[0],relativeNorm);
		relativeHorz=FVector::CrossProduct(relativeUp,relativeNorm);
		FVector dirV;
		float betweenAngle;
		//
		float left1=distanceInVector(relativeHorz,corner[0]-corner[2]);
		float left2=distanceInVector(relativeHorz,corner[1]-corner[2]);
		//UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), left1,left2);
		if(left1<0.0 || left2<0.0)
			return false;
		if(left1>left2){
			dirV=(corner[2]-corner[0]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,-relativeHorz)/(dirV.Size()*relativeHorz.Size());
			corner[0]+=(dirV/dirV.Size())*((left1-left2)/betweenAngle);
		}else{
			dirV=(corner[3]-corner[1]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,-relativeHorz)/(dirV.Size()*relativeHorz.Size());
			corner[1]+=(dirV/dirV.Size())*((left2-left1)/betweenAngle);
		}
		//
		float right1=distanceInVector(-relativeHorz,corner[2]-corner[0]);
		float right2=distanceInVector(-relativeHorz,corner[3]-corner[0]);
		//UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), right1,right2);
		if(right1<0.0 || right2<0.0)
			return false;
		if(right1>right2){
			dirV=(corner[0]-corner[2]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,relativeHorz)/(dirV.Size()*relativeHorz.Size());
			corner[2]+=(dirV/dirV.Size())*((right1-right2)/betweenAngle);
		}else{
			dirV=(corner[1]-corner[3]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,relativeHorz)/(dirV.Size()*relativeHorz.Size());
			corner[3]+=(dirV/dirV.Size())*((right2-right1)/betweenAngle);
		}
		//
		float up1=distanceInVector(relativeUp,corner[0]-corner[1]);
		float up2=distanceInVector(relativeUp,corner[2]-corner[1]);
		//UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), up1,up2);
		if(up1<0.0 || up2<0.0)
			return false;
		if(up1>up2){
			dirV=(corner[1]-corner[0]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,-relativeUp)/(dirV.Size()*relativeUp.Size());
			corner[0]+=(dirV/dirV.Size())*((up1-up2)/betweenAngle);
		}else{
			dirV=(corner[3]-corner[2]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,-relativeUp)/(dirV.Size()*relativeUp.Size());
			corner[2]+=(dirV/dirV.Size())*((up2-up1)/betweenAngle);
		}
		
		//
		float bottom1=distanceInVector(-relativeUp,corner[1]-corner[0]);
		float bottom2=distanceInVector(-relativeUp,corner[3]-corner[0]);
		//UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), bottom1,bottom2);
		if(bottom1<0.0 || bottom2<0.0)
			return false;
		if(bottom1>bottom2){
			dirV=(corner[0]-corner[1]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,relativeUp)/(dirV.Size()*relativeUp.Size());
			corner[1]+=(dirV/dirV.Size())*((bottom1-bottom2)/betweenAngle);
		}else{
			dirV=(corner[2]-corner[3]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,relativeUp)/(dirV.Size()*relativeUp.Size());
			corner[3]+=(dirV/dirV.Size())*((bottom2-bottom1)/betweenAngle);
		}
		
	}else{
		FVector dirV;
		float betweenAngle;
		//
		float left1=distanceInVector(relativeHorz,corner[0]-corner[2]);
		float left2=distanceInVector(relativeHorz,corner[1]-corner[2]);
		UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), left1,left2);
		if(left1<0.0 || left2<0.0)
			return false;
		if(left1>left2){
			dirV=(corner[2]-corner[0]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,-relativeHorz)/(dirV.Size()*relativeHorz.Size());
			corner[0]+=(dirV/dirV.Size())*((left1-left2)/betweenAngle);
		}else{
			dirV=(corner[3]-corner[1]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,-relativeHorz)/(dirV.Size()*relativeHorz.Size());
			corner[1]+=(dirV/dirV.Size())*((left2-left1)/betweenAngle);
		}
		//
		float right1=distanceInVector(-relativeHorz,corner[2]-corner[0]);
		float right2=distanceInVector(-relativeHorz,corner[3]-corner[0]);
		//UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), right1,right2);
		if(right1<0.0 || right2<0.0)
			return false;
		if(right1>right2){
			dirV=(corner[0]-corner[2]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,relativeHorz)/(dirV.Size()*relativeHorz.Size());
			corner[2]+=(dirV/dirV.Size())*((right1-right2)/betweenAngle);
		}else{
			dirV=(corner[1]-corner[3]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,relativeHorz)/(dirV.Size()*relativeHorz.Size());
			corner[3]+=(dirV/dirV.Size())*((right2-right1)/betweenAngle);
		}
		//
		float up1=distanceInVector(relativeUp,corner[0]-corner[1]);
		float up2=distanceInVector(relativeUp,corner[2]-corner[1]);
		UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), up1,up2);
		if(up1<0.0 || up2<0.0)
			return false;
		if(up1>up2){
			dirV=(corner[1]-corner[0]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,-relativeUp)/(dirV.Size()*relativeUp.Size());
			corner[0]+=(dirV/dirV.Size())*((up1-up2)/betweenAngle);
		}else{
			dirV=(corner[3]-corner[2]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,-relativeUp)/(dirV.Size()*relativeUp.Size());
			corner[2]+=(dirV/dirV.Size())*((up2-up1)/betweenAngle);
		}
		//
		
		float bottom1=distanceInVector(-relativeUp,corner[1]-corner[0]);
		float bottom2=distanceInVector(-relativeUp,corner[3]-corner[0]);
		//UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), bottom1,bottom2);
		if(bottom1<0.0 || bottom2<0.0)
			return false;
		if(bottom1>bottom2){
			dirV=(corner[0]-corner[1]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,relativeUp)/(dirV.Size()*relativeUp.Size());
			corner[1]+=(dirV/dirV.Size())*((bottom1-bottom2)/betweenAngle);
		}else{
			dirV=(corner[2]-corner[3]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,relativeUp)/(dirV.Size()*relativeUp.Size());
			corner[3]+=(dirV/dirV.Size())*((bottom2-bottom1)/betweenAngle);
		}
		

	}

	if(resizeFlag<5){
		//srand(time(NULL));
		//int resizeFlag=rand()%10;
		if( (corner[0]-corner[1]).Size() / (corner[0]-corner[2]).Size() >1.0 ){
			corner[1]+=(corner[0]-corner[1])*(float(resizeFlag+1)/10.0);
			corner[3]+=(corner[2]-corner[3])*(float(resizeFlag+1)/10.0);
		}
	}
	//
	//UE_LOG(LogTemp, Warning, TEXT("trac"));
	//CameraRotationV = CameraRotation.Rotation();
	//angle[0][0]=acos(FVector::DotProduct(corner[0]-CameraLocation,CameraRotationV)/((corner[0]-CameraLocation).Size()*CameraRotationV.Size()));
	/*FRotator corner0Rotator=(corner[0]-CameraLocation).Rotation();
	FRotator corner1Rotator=(corner[1]-CameraLocation).Rotation();
	FRotator corner2Rotator=(corner[2]-CameraLocation).Rotation();
	FRotator corner3Rotator=(corner[3]-CameraLocation).Rotation();
	angle[0][0]=corner0Rotator.Yaw-CameraRotation.Yaw;
	angle[0][1]=corner0Rotator.Pitch-CameraRotation.Pitch;
	angle[1][0]=corner1Rotator.Yaw-CameraRotation.Yaw;
	angle[1][1]=corner1Rotator.Pitch-CameraRotation.Pitch;
	angle[2][0]=corner2Rotator.Yaw-CameraRotation.Yaw;
	angle[2][1]=corner2Rotator.Pitch-CameraRotation.Pitch;
	angle[3][0]=corner3Rotator.Yaw-CameraRotation.Yaw;
	angle[3][1]=corner3Rotator.Pitch-CameraRotation.Pitch;
	//
	for(int i=0;i<4;i++){
			TraceEnd = CameraLocation+(corner[i]-CameraLocation)*1.5;
			// ++ // 
			//UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %.3f %.3f %.3f"), i, TraceEnd.X,TraceEnd.Y,TraceEnd.Z);
			bool hitflag = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
			if(false == hitflag)
				return false;
			corner[i]=Hit.Location;
			UE_LOG(LogTemp, Warning, TEXT("corner[ %d ]: %.3f"), i, corner[i].X);
			cornerNorm[i]=Hit.ImpactNormal;
			//UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %.3f"), i, cornerNorm[i].X);
			hitFlag[i]=hitflag;
			//UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %d"), i, hitflag);
			cornerHitInstanceName=Hit.GetActor()->GetName();
			//UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %s"), i, *cornerHitInstanceName);
	}*/
	FVector2D screenP[4];
	for(int i=0;i<4;++i){
		//FVector worldP=FVector(cord1, cord2, cord3);
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->ProjectWorldLocationToScreen(corner[i],screenP[i]);
	}


	for(int i=0;i<4;i++){
			//UE_LOG(LogTemp, Warning, TEXT("angle %d: %.3f %.3f"), i, angle[i][0], angle[i][1]);
			//UE_LOG(LogTemp, Warning, TEXT("corner %d: %.3f %.3f %.3f"), i, corner[i].X,corner[i].Y,corner[i].Z);
			relativeLocation=corner[i];
			relativeNorm=cornerNorm[i];
			/*if(FVector::DotProduct(relativeNorm,(CameraLocation-corner[i]))<0.0){
				UE_LOG(LogTemp, Warning, TEXT("** swap norm vector **"));
				relativeNorm=-relativeNorm;
			}*/
			bool normUp=false;
			if(!isEqZero(relativeNorm.X) && !isEqZero(relativeNorm.Y)){
				relativeHorz.Y=fabs(relativeNorm.X / sqrt(relativeNorm.X*relativeNorm.X+relativeNorm.Y*relativeNorm.Y) );
				relativeHorz.X=-relativeNorm.Y*relativeHorz.Y/relativeNorm.X;
				relativeHorz.Z=0.0;
			}else if(!isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y)){
				relativeHorz.X=0.0;
				relativeHorz.Y=1.0;
				relativeHorz.Z=0.0;
			}else if(isEqZero(relativeNorm.X) && !isEqZero(relativeNorm.Y)){
				relativeHorz.X=-1.0;
				relativeHorz.Y=0.0;
				relativeHorz.Z=0.0;
			}else{
				// Norm vector (0,0,1)
				//relativeHorz.X=0.0;
				//relativeHorz.Y=1.0;
				relativeHorz=FVector::CrossProduct(relativeNorm,(CameraLocation-corner[i]));
				normUp=true;
			}
			relativeUp=FVector::CrossProduct(relativeNorm,relativeHorz);
			if(relativeUp.Z<-0.001 && normUp==false){
				//UE_LOG(LogTemp, Warning, TEXT("** swap up vector **"));
				relativeHorz=-relativeHorz;
				relativeUp=-relativeUp;
			}

			textRotationV = relativeNorm;
			textLocation = relativeLocation;//0.002*(CameraLocation - relativeLocation);
			textRotation = textRotationV.Rotation();
			if(isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y)){
				textRotation.Yaw=0.0;
			}
			//
			break;
	}


	//==================================================
	FVector elementLocation;
	FVector elementNormal=FVector(1.0,0.0,0.0);
	FVector old_left_ElementLocation;
	FVector old_right_ElementLocation;
	FVector old_inter_location;
	/*float** vertexLens;
	vertexLens=new float*[Z_COUNT];
	for(int i=0;i<Z_COUNT;i++){
		vertexLens[i]=new float[Y_COUNT+1];
	}*/

	/*
	for(int i=0;i<Z_COUNT;i++){
		if(i==0){
			vertexLens[i][0]=0.0;
			vertexLens[i][Y_COUNT]=0.0;
			old_left_ElementLocation=FVector(0.0,0.0,0.0);
			old_right_ElementLocation=corner[2]-corner[0];
		}
		float curAngle1_left=angle[0][1]+(angle[1][1]-angle[0][1])*((float)i/(float)Z_COUNT);
		float curAngle1_right=angle[2][1]+(angle[3][1]-angle[2][1])*((float)i/(float)Z_COUNT);
		float curAngle0_left=angle[0][0]+(angle[1][0]-angle[0][0])*((float)i/(float)Z_COUNT);
		float curAngle0_right=angle[2][0]+(angle[3][0]-angle[2][0])*((float)i/(float)Z_COUNT);
		for(int j=0;j<Y_COUNT;j++){
			//float curAngle0=angle[0][0]+(angle[2][0]-angle[0][0])*((float)j/(float)Y_COUNT);
			TraceStart=CameraLocation;
			TraceEnd=CameraLocation;
			float curAngle0=curAngle0_left+(curAngle0_right-curAngle0_left)*((float)j/(float)Y_COUNT);
			float curAngle1=curAngle1_left+(curAngle1_right-curAngle1_left)*((float)j/(float)Y_COUNT);
			TraceEnd.X += actorL * (cos(PAI * (CameraRotation.Pitch + curAngle1) / 180.0) * cos(PAI * (CameraRotation.Yaw + curAngle0) / 180.0));
			TraceEnd.Y += actorL * (cos(PAI * (CameraRotation.Pitch + curAngle1) / 180.0 ) * sin(PAI * (CameraRotation.Yaw + curAngle0) / 180.0));
			TraceEnd.Z += actorL * (sin(PAI * (CameraRotation.Pitch + curAngle1) / 180.0));
			//
			if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams)) {
				elementLocation = Hit.Location;		
				elementLocation += gapLength*Hit.ImpactNormal/Hit.ImpactNormal.Size();
				elementNormal = Hit.ImpactNormal;	
				//UE_LOG(LogTemp, Warning, TEXT("hit: %s"), *Hit.GetActor()->GetName());
				FString curHitInstanceName=Hit.GetActor()->GetName();
				if(cornerHitInstanceName != curHitInstanceName){
					//UE_LOG(LogTemp, Warning, TEXT("different instance"));
					
				}
			}
			else {
				elementLocation = TraceEnd;
			}
			FVector shiftV=elementLocation-relativeLocation;
			//vertices.Add(FVector(nl,hl,ul));
			vertices.Add(shiftV);
			normals.Add(elementNormal);
			//
			//UV0.Add( FVector2D((float)j/(float)Y_COUNT, (float)i/(float)Z_COUNT) );
			tangents.Add(FProcMeshTangent(0, 1, 0));
			vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
			//
			if(j==0){
				vertexLens[i][0]=(elementLocation-old_left_ElementLocation).Size();
				old_left_ElementLocation=elementLocation;
				old_inter_location=elementLocation;
			}else if(j==Y_COUNT-1){
				vertexLens[i][Y_COUNT]=(elementLocation-old_right_ElementLocation).Size();
				old_right_ElementLocation=elementLocation;
				//
				vertexLens[i][j]=(elementLocation-old_inter_location).Size();
			}else{
				vertexLens[i][j]=(elementLocation-old_inter_location).Size();
				old_inter_location=elementLocation;
			}
		}
	}

	*/
	float minAngle0,maxAngle0,minAngle1,maxAngle1;
	minAngle0=minAngle1=1000.0;
	maxAngle0=maxAngle1=-1000.0;
	for(int i=0;i<4;i++){
		if(angle[i][0]<minAngle0)
			minAngle0=angle[i][0];
		if(angle[i][0]>maxAngle0)
			maxAngle0=angle[i][0];
		if(angle[i][1]<minAngle1)
			minAngle1=angle[i][1];
		if(angle[i][1]>maxAngle1)
			maxAngle1=angle[i][1];
	}
	FVector normQuad = relativeNorm;
	float QuadLen=10.0;
	int OutQuad=0;
	bool currentInQuad=false;
	/*
	bool beforeQuad,afterQuad;
	for(int i=0;i<Z_COUNT;i++){
		FVector edgeV1, edgeV2, interV;
		FVector oldEdgeV1, oldEdgeV2, oldInterV;
		edgeV1=corner[0]+((float)i)*(corner[1]-corner[0])/((float)Z_COUNT);
		edgeV2=corner[2]+((float)i)*(corner[3]-corner[2])/((float)Z_COUNT);
		// ++ // UE_LOG(LogTemp, Warning, TEXT("edgev1: %.3f %.3f %.3f"),edgeV1.X, edgeV1.Y, edgeV1.Z);
		if(i>0){
			vertexLens[i][0]=(edgeV1-oldEdgeV1).Size();
			vertexLens[i][Y_COUNT]=(edgeV2-oldEdgeV2).Size();
		}
		else{
			vertexLens[i][0]=0.0;
			vertexLens[i][Y_COUNT]=0.0;
		}
		oldEdgeV1=edgeV1;
		oldEdgeV2=edgeV2;	

		OutQuad=0;
		beforeQuad=true;
		afterQuad=false;

		for(int j=0;j<Y_COUNT;j++){
			interV=edgeV1+((float)j)*(edgeV2-edgeV1)/((float)Y_COUNT);
			//
			TraceStart=interV+normQuad*QuadLen;
			TraceEnd=interV-50.0*normQuad;

			//TraceStart=CameraLocation;
			//TraceEnd=CameraLocation+1.1*(interV-CameraLocation);


			if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams)) {
				elementLocation = Hit.Location;		
				elementLocation += gapLength*Hit.ImpactNormal/Hit.ImpactNormal.Size();
				elementNormal = Hit.ImpactNormal;		
				//UE_LOG(LogTemp, Warning, TEXT("hit: %s"), *Hit.GetActor()->GetName());
				FString curHitInstanceName=Hit.GetActor()->GetName();
				if(cornerHitInstanceName != curHitInstanceName){
					//UE_LOG(LogTemp, Warning, TEXT("different instance"));
					//TraceStart=Hit.Location;
					//if(GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams)){
					//	elementLocation = Hit.Location;	
					//	curHitInstanceName=Hit.GetActor()->GetName();
					//}else{
					//	elementLocation = TraceEnd;
					//	break;
					//}
				}
			}
			else {
				if(j>0)
					elementLocation = oldInterV-0.01*(corner[0]-corner[2]).Size()*relativeHorz/relativeHorz.Size(); //interV;//TraceEnd;
				else
					elementLocation=interV;
			}
			
			FVector shiftV=elementLocation-relativeLocation;
			
			//===
			//vertices.Add(FVector(nl,hl,ul));
			vertices.Add(shiftV);
			normals.Add(elementNormal);
			//
			//UV0.Add( FVector2D((float)j/(float)Y_COUNT, (float)i/(float)Z_COUNT) );
			tangents.Add(FProcMeshTangent(0, 1, 0));
			vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
			//
			if(j>0){
				vertexLens[i][j]=(elementLocation-oldInterV).Size();
			}
			oldInterV=elementLocation;
		}
	}*/
	FVector avgNormal=FVector(0.0,0.0,0.0);
	for(int i=0;i<Z_COUNT;++i){
		if(i==0){
			vertexLens[i][0]=0.0;
			vertexLens[i][Y_COUNT]=0.0;
			old_left_ElementLocation=FVector(0.0,0.0,0.0);
			old_right_ElementLocation=corner[2]-corner[0];
		}
		//screenPos.Y=float(angle[0][1])+((float)i/(float)Z_COUNT)*(angle[1][1]-angle[0][1]);
		FVector2D leftPos=screenP[0]+((float)i/(float)Z_COUNT)*(screenP[1]-screenP[0]);
		FVector2D rightPos=screenP[2]+((float)i/(float)Z_COUNT)*(screenP[3]-screenP[2]);
		for(int j=0;j<Y_COUNT;++j){
			//screenPos.X=float(angle[0][0])+((float)j/(float)Y_COUNT)*(angle[2][0]-angle[0][0]);
			FVector2D interPos=leftPos+((float)j/(float)Y_COUNT)*(rightPos-leftPos);
			PlayerController->GetHitResultAtScreenPosition(interPos, ECC_Visibility, true, Hit);
			elementLocation = Hit.Location;
			//elementLocation += gapLength*Hit.ImpactNormal/Hit.ImpactNormal.Size();
			//cornerNorm[i]=Hit.ImpactNormal;
			avgNormal+=Hit.ImpactNormal;
			FVector shiftV=elementLocation-relativeLocation;
			//vertices.Add(FVector(nl,hl,ul));
			vertices.Add(shiftV);
			normals.Add(FVector(1,0,0));
			//***********************************
			//UV0.Add( FVector2D((float)j/(float)Y_COUNT, (float)i/(float)Z_COUNT) );
			tangents.Add(FProcMeshTangent(0, 1, 0));
			vertexColors.Add(FLinearColor(0.95, 0.95, 0.95, 1.0));
			//
			if(j==0){
				vertexLens[i][0]=(elementLocation-old_left_ElementLocation).Size();
				old_left_ElementLocation=elementLocation;
				old_inter_location=elementLocation;
			}else if(j==Y_COUNT-1){
				vertexLens[i][Y_COUNT]=(elementLocation-old_right_ElementLocation).Size();
				old_right_ElementLocation=elementLocation;
				//
				vertexLens[i][j]=(elementLocation-old_inter_location).Size();
			}else{
				vertexLens[i][j]=(elementLocation-old_inter_location).Size();
				old_inter_location=elementLocation;
			}
		}
	}
	avgNormal/=avgNormal.Size();
	for(int i=0;i<Z_COUNT*Y_COUNT;++i){
		vertices[i]+=gapLength*avgNormal;
	}


	//
	for(int i=0;i<Z_COUNT;++i){
		if(i>0){
			vertexLens[i][0]=(vertices[i*Y_COUNT+0]-vertices[(i-1)*Y_COUNT+0]).Size();
			vertexLens[i][Y_COUNT]=(vertices[i*Y_COUNT+Y_COUNT-1]-vertices[(i-1)*Y_COUNT+Y_COUNT-1]).Size();
		}else{
			vertexLens[i][0]=vertexLens[i][Y_COUNT]=0.0;
		}
		//
		for(int j=1;j<Y_COUNT;++j){
			vertexLens[i][j]=(vertices[i*Y_COUNT+j]-vertices[i*Y_COUNT+j-1]).Size();
		}
	}
	//leftLen=rightLen=0.0;
	for(int i=0;i<Z_COUNT;++i){
		if(i>0){
			vertexLens[i][0]+=vertexLens[i-1][0];
			vertexLens[i][Y_COUNT]+=vertexLens[i-1][Y_COUNT];
		}else{
			vertexLens[i][0]=vertexLens[i][Y_COUNT]=0.0;
		}
		for(int j=2;j<Y_COUNT;++j){
			vertexLens[i][j]+=vertexLens[i][j-1];
		}
	}

	//vertexLens[Z_COUNT-1][0], vertexLens[0][Y_COUNT-1] ,vertexLens[Z_COUNT-1][Y_COUNT], vertexLens[Z_COUNT-1][Y_COUNT-1]

	for(int i=0;i<Z_COUNT;++i){
		vertexLens[i][0] /= vertexLens[Z_COUNT-1][0];
		vertexLens[i][Y_COUNT] /= vertexLens[Z_COUNT-1][Y_COUNT];
		UV0.Add( FVector2D(0.0,vertexLens[i][0]) );
		for(int j=1;j<Y_COUNT;++j){
			float xRate = vertexLens[i][j]/vertexLens[i][Y_COUNT-1];
			UV0.Add( FVector2D(xRate, vertexLens[i][0] + xRate*(vertexLens[i][Y_COUNT]-vertexLens[i][0])) );
			//UE_LOG(LogTemp, Warning, TEXT("UV-%d-%d: %.3f %.3f"),i,j,xRate, vertexLens[i][0] + xRate*(vertexLens[i][Y_COUNT]-vertexLens[i][0]));
		}
	}
	//
	// ===
	if(needCalculateBBOX){
		for(int i=0;i<bboxNum;++i){
			//UE_LOG(LogUnrealCV, Warning, TEXT("   bn %d"),i);
			float corner2D[4][2];
			corner2D[0][0]=bbox2D[i][0];
			corner2D[0][1]=bbox2D[i][1];
			corner2D[1][0]=bbox2D[i][0];
			corner2D[1][1]=bbox2D[i][3];
			corner2D[2][0]=bbox2D[i][2];
			corner2D[2][1]=bbox2D[i][1];
			corner2D[3][0]=bbox2D[i][2];
			corner2D[3][1]=bbox2D[i][3];
			float cur_value;
			int targetY,targetZ;
			for(int j=0;j<4;++j){
				//find min distance between 4 corner point of corner2D[j]
				
				//find 3D coordinate

				//store in bboxAngle[i][j]
				//UE_LOG(LogUnrealCV, Warning, TEXT("   NOOOOO     %.2f %.2f"),corner2D[j][0], corner2D[j][1]);
				float cornerRate_y = corner2D[j][0]/float(textureW);
				float cornerRate_z = corner2D[j][1]/float(textureH);
				//UE_LOG(LogUnrealCV, Warning, TEXT("  BBOX   %.3f %.3f %d %d"), cornerRate_y, cornerRate_z, textureW, textureH);
				float yRate,zRate;
				bool foundT=false;
				for(int zi=1;zi<Z_COUNT;++zi){
					//if(cornerRate_y<)
					for(int yi=1;yi<Y_COUNT;++yi){
						yRate = vertexLens[zi][yi]/vertexLens[zi][Y_COUNT-1];
						zRate = vertexLens[zi][0] + yRate*(vertexLens[zi][Y_COUNT]-vertexLens[zi][0]);
						if(cornerRate_y < yRate && cornerRate_z < zRate){
							targetZ=zi;
							targetY=yi;
							foundT=true;
							break;
						}
					}
					if(foundT)
						break;
				}
				if(!foundT){
					UE_LOG(LogUnrealCV, Warning, TEXT("NOOOOOOOO     %.2f %.2f"),cornerRate_y, cornerRate_z);
					targetZ=1;
					targetY=1;	
				}
				if(targetZ<1)
					targetZ=1;
				else if(targetZ>Z_COUNT-1)
					targetZ=Z_COUNT-1;
				if(targetY<1)
					targetY=1;
				else if(targetY>Y_COUNT-1)
					targetY=Y_COUNT-1;
				//}else{
					float range[4][2];
					if(targetY==1)
						yRate=0.0;
					else
						yRate=vertexLens[targetZ-1][targetY-1]/vertexLens[targetZ-1][Y_COUNT-1];
					zRate = vertexLens[targetZ-1][0] + yRate*(vertexLens[targetZ-1][Y_COUNT]-vertexLens[targetZ-1][0]);
					range[0][0]=yRate;
					range[0][1]=zRate;
					if(targetY==1)
						yRate=0.0;
					else
						yRate=vertexLens[targetZ][targetY-1]/vertexLens[targetZ][Y_COUNT-1];
					zRate = vertexLens[targetZ][0] + yRate*(vertexLens[targetZ][Y_COUNT]-vertexLens[targetZ][0]);
					range[1][0]=yRate;
					range[1][1]=zRate;
					yRate=vertexLens[targetZ-1][targetY]/vertexLens[targetZ-1][Y_COUNT-1];
					zRate = vertexLens[targetZ-1][0] + yRate*(vertexLens[targetZ-1][Y_COUNT]-vertexLens[targetZ-1][0]);
					range[2][0]=yRate;
					range[2][1]=zRate;
					yRate=vertexLens[targetZ][targetY]/vertexLens[targetZ][Y_COUNT-1];
					zRate = vertexLens[targetZ][0] + yRate*(vertexLens[targetZ][Y_COUNT]-vertexLens[targetZ][0]);
					range[3][0]=yRate;
					range[3][1]=zRate;
					//
					UE_LOG(LogUnrealCV, Warning, TEXT("    B %d"),(vertices.Num()));
					float curRate=(range[0][1]-cornerRate_z)/(range[0][1]-range[1][1]);
					FVector interL=vertices[Y_COUNT*(targetZ-1)+(targetY-1)]+curRate*(vertices[Y_COUNT*(targetZ)+(targetY-1)]-vertices[Y_COUNT*(targetZ-1)+(targetY-1)]);
					curRate=(range[2][1]-cornerRate_z)/(range[2][1]-range[3][1]);
					//UE_LOG(LogUnrealCV, Warning, TEXT("    B"));
					FVector interR=vertices[Y_COUNT*(targetZ-1)+(targetY)]+curRate*(vertices[Y_COUNT*(targetZ)+(targetY)]-vertices[Y_COUNT*(targetZ-1)+(targetY)]);
					curRate=(range[0][0]-cornerRate_y)/(range[0][0]-range[2][0]);
					FVector interM=interL+curRate*(interR-interL);
					interM+=relativeLocation;
					bboxAngle[i][j][0]=interM.X;
					bboxAngle[i][j][1]=interM.Y;
					bboxAngle[i][j][2]=interM.Z;

					FVector upv=(1.0-float(corner2D[j][0])/float(textureW))*corner[0]+(float(corner2D[j][0])/float(textureW))*corner[2];
					FVector downv=(1.0-float(corner2D[j][0])/float(textureW))*corner[1]+(float(corner2D[j][0])/float(textureW))*corner[3];
					interM = (1.0-float(corner2D[j][1])/float(textureH))*upv+(float(corner2D[j][1])/float(textureH))*downv;
					bboxAngle[i][j][0]=interM.X;
					bboxAngle[i][j][1]=interM.Y;
					bboxAngle[i][j][2]=interM.Z;

					//UE_LOG(LogUnrealCV, Warning, TEXT("  BBOX   %.3f %.3f %.3f"), interM.X, interM.Y, interM.Z);
				//}
				//
			}
		}
	}
	//====================
	if(needCalculateCharBBOX){
		for(int i=0;i<charNum;++i){
			//UE_LOG(LogUnrealCV, Warning, TEXT("   bn %d"),i);
			float corner2D[4][2];
			corner2D[0][0]=char2D[i][0];
			corner2D[0][1]=char2D[i][1];
			corner2D[1][0]=char2D[i][0];
			corner2D[1][1]=char2D[i][3];
			corner2D[2][0]=char2D[i][2];
			corner2D[2][1]=char2D[i][1];
			corner2D[3][0]=char2D[i][2];
			corner2D[3][1]=char2D[i][3];
			float cur_value;
			int targetY,targetZ;
			for(int j=0;j<4;++j){
				//find min distance between 4 corner point of corner2D[j]
				
				//find 3D coordinate

				//store in bboxAngle[i][j]
				//UE_LOG(LogUnrealCV, Warning, TEXT("   NOOOOO     %.2f %.2f"),corner2D[j][0], corner2D[j][1]);
				float cornerRate_y = corner2D[j][0]/float(textureW);
				float cornerRate_z = corner2D[j][1]/float(textureH);
				//UE_LOG(LogUnrealCV, Warning, TEXT("  BBOX   %.3f %.3f %d %d"), cornerRate_y, cornerRate_z, textureW, textureH);
				float yRate,zRate;
				bool foundT=false;
				for(int zi=1;zi<Z_COUNT;++zi){
					//if(cornerRate_y<)
					for(int yi=1;yi<Y_COUNT;++yi){
						yRate = vertexLens[zi][yi]/vertexLens[zi][Y_COUNT-1];
						zRate = vertexLens[zi][0] + yRate*(vertexLens[zi][Y_COUNT]-vertexLens[zi][0]);
						if(cornerRate_y < yRate && cornerRate_z < zRate){
							targetZ=zi;
							targetY=yi;
							foundT=true;
							break;
						}
					}
					if(foundT)
						break;
				}
				if(!foundT){
					UE_LOG(LogUnrealCV, Warning, TEXT("NOOOOOOOO     %.2f %.2f"),cornerRate_y, cornerRate_z);
					targetZ=1;
					targetY=1;	
				}
				if(targetZ<1)
					targetZ=1;
				else if(targetZ>Z_COUNT-1)
					targetZ=Z_COUNT-1;
				if(targetY<1)
					targetY=1;
				else if(targetY>Y_COUNT-1)
					targetY=Y_COUNT-1;
				//}else{
					float range[4][2];
					if(targetY==1)
						yRate=0.0;
					else
						yRate=vertexLens[targetZ-1][targetY-1]/vertexLens[targetZ-1][Y_COUNT-1];
					zRate = vertexLens[targetZ-1][0] + yRate*(vertexLens[targetZ-1][Y_COUNT]-vertexLens[targetZ-1][0]);
					range[0][0]=yRate;
					range[0][1]=zRate;
					if(targetY==1)
						yRate=0.0;
					else
						yRate=vertexLens[targetZ][targetY-1]/vertexLens[targetZ][Y_COUNT-1];
					zRate = vertexLens[targetZ][0] + yRate*(vertexLens[targetZ][Y_COUNT]-vertexLens[targetZ][0]);
					range[1][0]=yRate;
					range[1][1]=zRate;
					yRate=vertexLens[targetZ-1][targetY]/vertexLens[targetZ-1][Y_COUNT-1];
					zRate = vertexLens[targetZ-1][0] + yRate*(vertexLens[targetZ-1][Y_COUNT]-vertexLens[targetZ-1][0]);
					range[2][0]=yRate;
					range[2][1]=zRate;
					yRate=vertexLens[targetZ][targetY]/vertexLens[targetZ][Y_COUNT-1];
					zRate = vertexLens[targetZ][0] + yRate*(vertexLens[targetZ][Y_COUNT]-vertexLens[targetZ][0]);
					range[3][0]=yRate;
					range[3][1]=zRate;
					//
					UE_LOG(LogUnrealCV, Warning, TEXT("    B %d"),(vertices.Num()));
					float curRate=(range[0][1]-cornerRate_z)/(range[0][1]-range[1][1]);
					FVector interL=vertices[Y_COUNT*(targetZ-1)+(targetY-1)]+curRate*(vertices[Y_COUNT*(targetZ)+(targetY-1)]-vertices[Y_COUNT*(targetZ-1)+(targetY-1)]);
					curRate=(range[2][1]-cornerRate_z)/(range[2][1]-range[3][1]);
					//UE_LOG(LogUnrealCV, Warning, TEXT("    B"));
					FVector interR=vertices[Y_COUNT*(targetZ-1)+(targetY)]+curRate*(vertices[Y_COUNT*(targetZ)+(targetY)]-vertices[Y_COUNT*(targetZ-1)+(targetY)]);
					curRate=(range[0][0]-cornerRate_y)/(range[0][0]-range[2][0]);
					FVector interM=interL+curRate*(interR-interL);
					interM+=relativeLocation;
					charAngle[i][j][0]=interM.X;
					charAngle[i][j][1]=interM.Y;
					charAngle[i][j][2]=interM.Z;

					FVector upv=(1.0-float(corner2D[j][0])/float(textureW))*corner[0]+(float(corner2D[j][0])/float(textureW))*corner[2];
					FVector downv=(1.0-float(corner2D[j][0])/float(textureW))*corner[1]+(float(corner2D[j][0])/float(textureW))*corner[3];
					interM = (1.0-float(corner2D[j][1])/float(textureH))*upv+(float(corner2D[j][1])/float(textureH))*downv;
					charAngle[i][j][0]=interM.X;
					charAngle[i][j][1]=interM.Y;
					charAngle[i][j][2]=interM.Z;

					//UE_LOG(LogUnrealCV, Warning, TEXT("  BBOX   %.3f %.3f %.3f"), interM.X, interM.Y, interM.Z);
				//}
				//
			}
		}
	}
	//====================

	//
	for (int i = 0;i < Y_COUNT - 1;++i) {
		for (int j = 0;j < Z_COUNT - 1;++j) {
			Triangles.Add(Z_COUNT*i + j);
			//Triangles.Add(Z_COUNT*i + j + 1);
			Triangles.Add(Z_COUNT*(i + 1) + j);
			Triangles.Add(Z_COUNT*i + j + 1);			
			//
			Triangles.Add(Z_COUNT*(i + 1) + j + 1);
			//Triangles.Add(Z_COUNT*(i + 1) + j);
			Triangles.Add(Z_COUNT*i + j + 1);
			Triangles.Add(Z_COUNT*(i + 1) + j);
		}
	}

	UE_LOG(LogUnrealCV, Warning, TEXT("  vertexColors: %d"), vertexColors.Num());
	//for(int i=0;i<vertexColors.Num();++i){
	//	UE_LOG(LogUnrealCV, Warning, TEXT("    vc: %.2f %.2f %.2f %.2f"), vertexColors[i].R, vertexColors[i].G, vertexColors[i].B, vertexColors[i].A);
	//}
	mesh->CreateMeshSection_LinearColor(0, vertices, Triangles, normals, UV0, vertexColors, tangents, true);

	// Enable collision data
	mesh->ContainsPhysicsTriMeshData(true);
	return true;
}

bool APugTextPawn::CreateTriangleCameraLight_for_preStereo(){
	imgVertices.Empty();
	imgNormals.Empty();

	float mid_0=(inp[0]+inp[2])*0.5;
	float mid_1=(inp[1]+inp[3])*0.5;
	angle[0][0]=inp[0]+(mid_0-inp[0])*0.01;
	angle[0][1]=inp[1]+(mid_1-inp[1])*0.01;
	angle[1][0]=inp[0]+(mid_0-inp[0])*0.01;
	angle[1][1]=inp[3]+(mid_1-inp[3])*0.01;
	angle[2][0]=inp[2]+(mid_0-inp[2])*0.01;
	angle[2][1]=inp[1]+(mid_1-inp[1])*0.01;
	angle[3][0]=inp[2]+(mid_0-inp[2])*0.01;
	angle[3][1]=inp[3]+(mid_1-inp[3])*0.01;

	FVector corner[4];
	FVector projectedCorner[4];
	FString cornerHitInstanceName;
	FVector cornerNorm[4];
	FVector relativeNorm, relativeHorz, relativeUp, relativeLocation;
	bool hitFlag[4];

	bool bIsMatinee = false;

	float actorL = 1000000000.0;
	FCollisionQueryParams TraceParams(FName(TEXT("TraceUsableActor")), true, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnFaceIndex = true;

	FHitResult Hit(ForceInit);
	FVector TraceEnd;
	FVector TraceStart;
	TraceStart = CameraLocation;

	APawn* Pawn = FUE4CVServer::Get().GetPawn();
	APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
	FVector2D screenPos;

	for(int i=0;i<4;i++)
		hitFlag[i]=false;
	for(int i=0;i<4;i++){
		screenPos.X = angle[i][0];
		screenPos.Y = angle[i][1];
		if(PlayerController!=nullptr){
			PlayerController->GetHitResultAtScreenPosition(screenPos, ECC_Visibility, true, Hit);
			corner[i]=Hit.Location;
			cornerNorm[i]=Hit.ImpactNormal;
			if(Hit.GetActor() == nullptr)
				hitFlag[i]=false;
			else
				hitFlag[i]=true;
		}else{
			return false;
		}
		
		/*	TraceEnd = CameraLocation;
			TraceEnd.X += actorL * (cos(PAI * (CameraRotation.Pitch+angle[i][1]) / 180.0) * cos(PAI * (CameraRotation.Yaw+angle[i][0]) / 180.0));
			TraceEnd.Y += actorL * (cos(PAI * (CameraRotation.Pitch+angle[i][1]) / 180.0) * sin(PAI * (CameraRotation.Yaw+angle[i][0]) / 180.0));
			TraceEnd.Z += actorL * (sin(PAI * (CameraRotation.Pitch+angle[i][1]) / 180.0));
			// ++ // UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %.3f %.3f %.3f"), i, TraceEnd.X,TraceEnd.Y,TraceEnd.Z);
			bool hitflag = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
			corner[i]=Hit.Location;
			cornerNorm[i]=Hit.ImpactNormal;
			hitFlag[i]=hitflag;
			//cornerHitInstanceName=Hit.GetActor()->GetName();
		*/
	}
	if(hitFlag[0]*hitFlag[1]*hitFlag[2]*hitFlag[3]==0){
		//UE_LOG(LogTemp, Warning, TEXT("NO HIT!!!"));
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("NO HIT"));
		return false;
	}
	//
	for(int i=0;i<4;i++){
			//UE_LOG(LogTemp, Warning, TEXT("angle %d: %.3f %.3f"), i, angle[i][0], angle[i][1]);
			//UE_LOG(LogTemp, Warning, TEXT("corner %d: %.3f %.3f %.3f"), i, corner[i].X,corner[i].Y,corner[i].Z);
			relativeLocation=corner[i];
			relativeNorm=cornerNorm[i];
			/*if(FVector::DotProduct(relativeNorm,(CameraLocation-corner[i]))<0.0){
				UE_LOG(LogTemp, Warning, TEXT("** swap norm vector **"));
				relativeNorm=-relativeNorm;
			}*/
			bool normUp=false;
			if(!isEqZero(relativeNorm.X) && !isEqZero(relativeNorm.Y)){
				relativeHorz.Y=fabs(relativeNorm.X / sqrt(relativeNorm.X*relativeNorm.X+relativeNorm.Y*relativeNorm.Y) );
				relativeHorz.X=-relativeNorm.Y*relativeHorz.Y/relativeNorm.X;
				relativeHorz.Z=0.0;
			}else if(!isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y)){
				relativeHorz.X=0.0;
				relativeHorz.Y=1.0;
				relativeHorz.Z=0.0;
			}else if(isEqZero(relativeNorm.X) && !isEqZero(relativeNorm.Y)){
				relativeHorz.X=-1.0;
				relativeHorz.Y=0.0;
				relativeHorz.Z=0.0;
			}else{
				// Norm vector (0,0,1)
				//relativeHorz.X=0.0;
				//relativeHorz.Y=1.0;
				relativeHorz=FVector::CrossProduct(relativeNorm,(CameraLocation-corner[i]));
				normUp=true;
			}
			relativeUp=FVector::CrossProduct(relativeNorm,relativeHorz);
			if(relativeUp.Z<-0.001 && normUp==false){
				//UE_LOG(LogTemp, Warning, TEXT("** swap up vector **"));
				relativeHorz=-relativeHorz;
				relativeUp=-relativeUp;
			}

			textRotationV = relativeNorm;
			textLocation = relativeLocation;//0.002*(CameraLocation - relativeLocation);
			textRotation = textRotationV.Rotation();
			if(isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y)){
				textRotation.Yaw=0.0;
			}
			//
			break;
	}
	//for(int i=0;i<4;i++){
	//	UE_LOG(LogTemp, Warning, TEXT("Corner : %.3f %.3f %.3f"), corner[i].X,corner[i].Y,corner[i].Z);
	//}
	//UE_LOG(LogTemp, Warning, TEXT("relativeNorm : %.3f %.3f %.3f"), relativeNorm.X,relativeNorm.Y,relativeNorm.Z);
			
	//UE_LOG(LogTemp, Warning, TEXT("relativeHorz : %.3f %.3f %.3f"), relativeHorz.X,relativeHorz.Y,relativeHorz.Z);
	
	//UE_LOG(LogTemp, Warning, TEXT("relativeUp : %.3f %.3f %.3f"), relativeUp.X,relativeUp.Y,relativeUp.Z);
	//
	if(isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y) ){
		relativeUp=FVector::CrossProduct(corner[2]-corner[0],relativeNorm);
		relativeHorz=FVector::CrossProduct(relativeUp,relativeNorm);
		FVector dirV;
		float betweenAngle;
		//
		float left1=distanceInVector(relativeHorz,corner[0]-corner[2]);
		float left2=distanceInVector(relativeHorz,corner[1]-corner[2]);
		//UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), left1,left2);
		if(left1<0.0 || left2<0.0)
			return false;
		if(left1>left2){
			dirV=(corner[2]-corner[0]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,-relativeHorz)/(dirV.Size()*relativeHorz.Size());
			corner[0]+=(dirV/dirV.Size())*((left1-left2)/betweenAngle);
		}else{
			dirV=(corner[3]-corner[1]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,-relativeHorz)/(dirV.Size()*relativeHorz.Size());
			corner[1]+=(dirV/dirV.Size())*((left2-left1)/betweenAngle);
		}
		//
		float right1=distanceInVector(-relativeHorz,corner[2]-corner[0]);
		float right2=distanceInVector(-relativeHorz,corner[3]-corner[0]);
		//UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), right1,right2);
		if(right1<0.0 || right2<0.0)
			return false;
		if(right1>right2){
			dirV=(corner[0]-corner[2]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,relativeHorz)/(dirV.Size()*relativeHorz.Size());
			corner[2]+=(dirV/dirV.Size())*((right1-right2)/betweenAngle);
		}else{
			dirV=(corner[1]-corner[3]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,relativeHorz)/(dirV.Size()*relativeHorz.Size());
			corner[3]+=(dirV/dirV.Size())*((right2-right1)/betweenAngle);
		}
		//
		float up1=distanceInVector(relativeUp,corner[0]-corner[1]);
		float up2=distanceInVector(relativeUp,corner[2]-corner[1]);
		//UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), up1,up2);
		if(up1<0.0 || up2<0.0)
			return false;
		if(up1>up2){
			dirV=(corner[1]-corner[0]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,-relativeUp)/(dirV.Size()*relativeUp.Size());
			corner[0]+=(dirV/dirV.Size())*((up1-up2)/betweenAngle);
		}else{
			dirV=(corner[3]-corner[2]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,-relativeUp)/(dirV.Size()*relativeUp.Size());
			corner[2]+=(dirV/dirV.Size())*((up2-up1)/betweenAngle);
		}
		
		//
		float bottom1=distanceInVector(-relativeUp,corner[1]-corner[0]);
		float bottom2=distanceInVector(-relativeUp,corner[3]-corner[0]);
		//UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), bottom1,bottom2);
		if(bottom1<0.0 || bottom2<0.0)
			return false;
		if(bottom1>bottom2){
			dirV=(corner[0]-corner[1]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,relativeUp)/(dirV.Size()*relativeUp.Size());
			corner[1]+=(dirV/dirV.Size())*((bottom1-bottom2)/betweenAngle);
		}else{
			dirV=(corner[2]-corner[3]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,relativeUp)/(dirV.Size()*relativeUp.Size());
			corner[3]+=(dirV/dirV.Size())*((bottom2-bottom1)/betweenAngle);
		}
		
	}else{
		FVector dirV;
		float betweenAngle;
		//
		float left1=distanceInVector(relativeHorz,corner[0]-corner[2]);
		float left2=distanceInVector(relativeHorz,corner[1]-corner[2]);
		UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), left1,left2);
		if(left1<0.0 || left2<0.0)
			return false;
		if(left1>left2){
			dirV=(corner[2]-corner[0]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,-relativeHorz)/(dirV.Size()*relativeHorz.Size());
			corner[0]+=(dirV/dirV.Size())*((left1-left2)/betweenAngle);
		}else{
			dirV=(corner[3]-corner[1]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,-relativeHorz)/(dirV.Size()*relativeHorz.Size());
			corner[1]+=(dirV/dirV.Size())*((left2-left1)/betweenAngle);
		}
		//
		float right1=distanceInVector(-relativeHorz,corner[2]-corner[0]);
		float right2=distanceInVector(-relativeHorz,corner[3]-corner[0]);
		//UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), right1,right2);
		if(right1<0.0 || right2<0.0)
			return false;
		if(right1>right2){
			dirV=(corner[0]-corner[2]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,relativeHorz)/(dirV.Size()*relativeHorz.Size());
			corner[2]+=(dirV/dirV.Size())*((right1-right2)/betweenAngle);
		}else{
			dirV=(corner[1]-corner[3]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,relativeHorz)/(dirV.Size()*relativeHorz.Size());
			corner[3]+=(dirV/dirV.Size())*((right2-right1)/betweenAngle);
		}
		//
		float up1=distanceInVector(relativeUp,corner[0]-corner[1]);
		float up2=distanceInVector(relativeUp,corner[2]-corner[1]);
		UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), up1,up2);
		if(up1<0.0 || up2<0.0)
			return false;
		if(up1>up2){
			dirV=(corner[1]-corner[0]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,-relativeUp)/(dirV.Size()*relativeUp.Size());
			corner[0]+=(dirV/dirV.Size())*((up1-up2)/betweenAngle);
		}else{
			dirV=(corner[3]-corner[2]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,-relativeUp)/(dirV.Size()*relativeUp.Size());
			corner[2]+=(dirV/dirV.Size())*((up2-up1)/betweenAngle);
		}
		//
		
		float bottom1=distanceInVector(-relativeUp,corner[1]-corner[0]);
		float bottom2=distanceInVector(-relativeUp,corner[3]-corner[0]);
		//UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), bottom1,bottom2);
		if(bottom1<0.0 || bottom2<0.0)
			return false;
		if(bottom1>bottom2){
			dirV=(corner[0]-corner[1]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,relativeUp)/(dirV.Size()*relativeUp.Size());
			corner[1]+=(dirV/dirV.Size())*((bottom1-bottom2)/betweenAngle);
		}else{
			dirV=(corner[2]-corner[3]);
			if(dirV.Size()==0.0)
				return false;
			betweenAngle=FVector::DotProduct(dirV,relativeUp)/(dirV.Size()*relativeUp.Size());
			corner[3]+=(dirV/dirV.Size())*((bottom2-bottom1)/betweenAngle);
		}
		

	}

	if(resizeFlag<5){
		//srand(time(NULL));
		//int resizeFlag=rand()%10;
		if( (corner[0]-corner[1]).Size() / (corner[0]-corner[2]).Size() >1.0 ){
			corner[1]+=(corner[0]-corner[1])*(float(resizeFlag+1)/10.0);
			corner[3]+=(corner[2]-corner[3])*(float(resizeFlag+1)/10.0);
		}
	}
	//
	//UE_LOG(LogTemp, Warning, TEXT("trac"));
	//CameraRotationV = CameraRotation.Rotation();
	//angle[0][0]=acos(FVector::DotProduct(corner[0]-CameraLocation,CameraRotationV)/((corner[0]-CameraLocation).Size()*CameraRotationV.Size()));
	/*FRotator corner0Rotator=(corner[0]-CameraLocation).Rotation();
	FRotator corner1Rotator=(corner[1]-CameraLocation).Rotation();
	FRotator corner2Rotator=(corner[2]-CameraLocation).Rotation();
	FRotator corner3Rotator=(corner[3]-CameraLocation).Rotation();
	angle[0][0]=corner0Rotator.Yaw-CameraRotation.Yaw;
	angle[0][1]=corner0Rotator.Pitch-CameraRotation.Pitch;
	angle[1][0]=corner1Rotator.Yaw-CameraRotation.Yaw;
	angle[1][1]=corner1Rotator.Pitch-CameraRotation.Pitch;
	angle[2][0]=corner2Rotator.Yaw-CameraRotation.Yaw;
	angle[2][1]=corner2Rotator.Pitch-CameraRotation.Pitch;
	angle[3][0]=corner3Rotator.Yaw-CameraRotation.Yaw;
	angle[3][1]=corner3Rotator.Pitch-CameraRotation.Pitch;
	//
	for(int i=0;i<4;i++){
			TraceEnd = CameraLocation+(corner[i]-CameraLocation)*1.5;
			// ++ // 
			//UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %.3f %.3f %.3f"), i, TraceEnd.X,TraceEnd.Y,TraceEnd.Z);
			bool hitflag = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
			if(false == hitflag)
				return false;
			corner[i]=Hit.Location;
			UE_LOG(LogTemp, Warning, TEXT("corner[ %d ]: %.3f"), i, corner[i].X);
			cornerNorm[i]=Hit.ImpactNormal;
			//UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %.3f"), i, cornerNorm[i].X);
			hitFlag[i]=hitflag;
			//UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %d"), i, hitflag);
			cornerHitInstanceName=Hit.GetActor()->GetName();
			//UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %s"), i, *cornerHitInstanceName);
	}*/
	FVector2D screenP[4];
	for(int i=0;i<4;++i){
		//FVector worldP=FVector(cord1, cord2, cord3);
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->ProjectWorldLocationToScreen(corner[i],screenP[i]);
	}


	for(int i=0;i<4;i++){
			//UE_LOG(LogTemp, Warning, TEXT("angle %d: %.3f %.3f"), i, angle[i][0], angle[i][1]);
			//UE_LOG(LogTemp, Warning, TEXT("corner %d: %.3f %.3f %.3f"), i, corner[i].X,corner[i].Y,corner[i].Z);
			relativeLocation=corner[i];
			relativeNorm=cornerNorm[i];
			/*if(FVector::DotProduct(relativeNorm,(CameraLocation-corner[i]))<0.0){
				UE_LOG(LogTemp, Warning, TEXT("** swap norm vector **"));
				relativeNorm=-relativeNorm;
			}*/
			bool normUp=false;
			if(!isEqZero(relativeNorm.X) && !isEqZero(relativeNorm.Y)){
				relativeHorz.Y=fabs(relativeNorm.X / sqrt(relativeNorm.X*relativeNorm.X+relativeNorm.Y*relativeNorm.Y) );
				relativeHorz.X=-relativeNorm.Y*relativeHorz.Y/relativeNorm.X;
				relativeHorz.Z=0.0;
			}else if(!isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y)){
				relativeHorz.X=0.0;
				relativeHorz.Y=1.0;
				relativeHorz.Z=0.0;
			}else if(isEqZero(relativeNorm.X) && !isEqZero(relativeNorm.Y)){
				relativeHorz.X=-1.0;
				relativeHorz.Y=0.0;
				relativeHorz.Z=0.0;
			}else{
				// Norm vector (0,0,1)
				//relativeHorz.X=0.0;
				//relativeHorz.Y=1.0;
				relativeHorz=FVector::CrossProduct(relativeNorm,(CameraLocation-corner[i]));
				normUp=true;
			}
			relativeUp=FVector::CrossProduct(relativeNorm,relativeHorz);
			if(relativeUp.Z<-0.001 && normUp==false){
				//UE_LOG(LogTemp, Warning, TEXT("** swap up vector **"));
				relativeHorz=-relativeHorz;
				relativeUp=-relativeUp;
			}

			textRotationV = relativeNorm;
			textLocation = relativeLocation;//0.002*(CameraLocation - relativeLocation);
			textRotation = textRotationV.Rotation();
			if(isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y)){
				textRotation.Yaw=0.0;
			}
			//
			break;
	}


	//==================================================
	FVector elementLocation;
	FVector elementNormal=FVector(1.0,0.0,0.0);
	FVector old_left_ElementLocation;
	FVector old_right_ElementLocation;
	FVector old_inter_location;
	/*float** vertexLens;
	vertexLens=new float*[Z_COUNT];
	for(int i=0;i<Z_COUNT;i++){
		vertexLens[i]=new float[Y_COUNT+1];
	}*/

	/*
	for(int i=0;i<Z_COUNT;i++){
		if(i==0){
			vertexLens[i][0]=0.0;
			vertexLens[i][Y_COUNT]=0.0;
			old_left_ElementLocation=FVector(0.0,0.0,0.0);
			old_right_ElementLocation=corner[2]-corner[0];
		}
		float curAngle1_left=angle[0][1]+(angle[1][1]-angle[0][1])*((float)i/(float)Z_COUNT);
		float curAngle1_right=angle[2][1]+(angle[3][1]-angle[2][1])*((float)i/(float)Z_COUNT);
		float curAngle0_left=angle[0][0]+(angle[1][0]-angle[0][0])*((float)i/(float)Z_COUNT);
		float curAngle0_right=angle[2][0]+(angle[3][0]-angle[2][0])*((float)i/(float)Z_COUNT);
		for(int j=0;j<Y_COUNT;j++){
			//float curAngle0=angle[0][0]+(angle[2][0]-angle[0][0])*((float)j/(float)Y_COUNT);
			TraceStart=CameraLocation;
			TraceEnd=CameraLocation;
			float curAngle0=curAngle0_left+(curAngle0_right-curAngle0_left)*((float)j/(float)Y_COUNT);
			float curAngle1=curAngle1_left+(curAngle1_right-curAngle1_left)*((float)j/(float)Y_COUNT);
			TraceEnd.X += actorL * (cos(PAI * (CameraRotation.Pitch + curAngle1) / 180.0) * cos(PAI * (CameraRotation.Yaw + curAngle0) / 180.0));
			TraceEnd.Y += actorL * (cos(PAI * (CameraRotation.Pitch + curAngle1) / 180.0 ) * sin(PAI * (CameraRotation.Yaw + curAngle0) / 180.0));
			TraceEnd.Z += actorL * (sin(PAI * (CameraRotation.Pitch + curAngle1) / 180.0));
			//
			if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams)) {
				elementLocation = Hit.Location;		
				elementLocation += gapLength*Hit.ImpactNormal/Hit.ImpactNormal.Size();
				elementNormal = Hit.ImpactNormal;	
				//UE_LOG(LogTemp, Warning, TEXT("hit: %s"), *Hit.GetActor()->GetName());
				FString curHitInstanceName=Hit.GetActor()->GetName();
				if(cornerHitInstanceName != curHitInstanceName){
					//UE_LOG(LogTemp, Warning, TEXT("different instance"));
					
				}
			}
			else {
				elementLocation = TraceEnd;
			}
			FVector shiftV=elementLocation-relativeLocation;
			//vertices.Add(FVector(nl,hl,ul));
			vertices.Add(shiftV);
			normals.Add(elementNormal);
			//
			//UV0.Add( FVector2D((float)j/(float)Y_COUNT, (float)i/(float)Z_COUNT) );
			tangents.Add(FProcMeshTangent(0, 1, 0));
			vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
			//
			if(j==0){
				vertexLens[i][0]=(elementLocation-old_left_ElementLocation).Size();
				old_left_ElementLocation=elementLocation;
				old_inter_location=elementLocation;
			}else if(j==Y_COUNT-1){
				vertexLens[i][Y_COUNT]=(elementLocation-old_right_ElementLocation).Size();
				old_right_ElementLocation=elementLocation;
				//
				vertexLens[i][j]=(elementLocation-old_inter_location).Size();
			}else{
				vertexLens[i][j]=(elementLocation-old_inter_location).Size();
				old_inter_location=elementLocation;
			}
		}
	}

	*/
	float minAngle0,maxAngle0,minAngle1,maxAngle1;
	minAngle0=minAngle1=1000.0;
	maxAngle0=maxAngle1=-1000.0;
	for(int i=0;i<4;i++){
		if(angle[i][0]<minAngle0)
			minAngle0=angle[i][0];
		if(angle[i][0]>maxAngle0)
			maxAngle0=angle[i][0];
		if(angle[i][1]<minAngle1)
			minAngle1=angle[i][1];
		if(angle[i][1]>maxAngle1)
			maxAngle1=angle[i][1];
	}
	FVector normQuad = relativeNorm;
	float QuadLen=10.0;
	int OutQuad=0;
	bool currentInQuad=false;
	/*
	bool beforeQuad,afterQuad;
	for(int i=0;i<Z_COUNT;i++){
		FVector edgeV1, edgeV2, interV;
		FVector oldEdgeV1, oldEdgeV2, oldInterV;
		edgeV1=corner[0]+((float)i)*(corner[1]-corner[0])/((float)Z_COUNT);
		edgeV2=corner[2]+((float)i)*(corner[3]-corner[2])/((float)Z_COUNT);
		// ++ // UE_LOG(LogTemp, Warning, TEXT("edgev1: %.3f %.3f %.3f"),edgeV1.X, edgeV1.Y, edgeV1.Z);
		if(i>0){
			vertexLens[i][0]=(edgeV1-oldEdgeV1).Size();
			vertexLens[i][Y_COUNT]=(edgeV2-oldEdgeV2).Size();
		}
		else{
			vertexLens[i][0]=0.0;
			vertexLens[i][Y_COUNT]=0.0;
		}
		oldEdgeV1=edgeV1;
		oldEdgeV2=edgeV2;	

		OutQuad=0;
		beforeQuad=true;
		afterQuad=false;

		for(int j=0;j<Y_COUNT;j++){
			interV=edgeV1+((float)j)*(edgeV2-edgeV1)/((float)Y_COUNT);
			//
			TraceStart=interV+normQuad*QuadLen;
			TraceEnd=interV-50.0*normQuad;

			//TraceStart=CameraLocation;
			//TraceEnd=CameraLocation+1.1*(interV-CameraLocation);


			if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams)) {
				elementLocation = Hit.Location;		
				elementLocation += gapLength*Hit.ImpactNormal/Hit.ImpactNormal.Size();
				elementNormal = Hit.ImpactNormal;		
				//UE_LOG(LogTemp, Warning, TEXT("hit: %s"), *Hit.GetActor()->GetName());
				FString curHitInstanceName=Hit.GetActor()->GetName();
				if(cornerHitInstanceName != curHitInstanceName){
					//UE_LOG(LogTemp, Warning, TEXT("different instance"));
					//TraceStart=Hit.Location;
					//if(GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams)){
					//	elementLocation = Hit.Location;	
					//	curHitInstanceName=Hit.GetActor()->GetName();
					//}else{
					//	elementLocation = TraceEnd;
					//	break;
					//}
				}
			}
			else {
				if(j>0)
					elementLocation = oldInterV-0.01*(corner[0]-corner[2]).Size()*relativeHorz/relativeHorz.Size(); //interV;//TraceEnd;
				else
					elementLocation=interV;
			}
			
			FVector shiftV=elementLocation-relativeLocation;
			
			//===
			//vertices.Add(FVector(nl,hl,ul));
			vertices.Add(shiftV);
			normals.Add(elementNormal);
			//
			//UV0.Add( FVector2D((float)j/(float)Y_COUNT, (float)i/(float)Z_COUNT) );
			tangents.Add(FProcMeshTangent(0, 1, 0));
			vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
			//
			if(j>0){
				vertexLens[i][j]=(elementLocation-oldInterV).Size();
			}
			oldInterV=elementLocation;
		}
	}*/
	FVector avgNormal=FVector(0.0,0.0,0.0);
	for(int i=0;i<Z_COUNT;++i){
		if(i==0){
			vertexLens[i][0]=0.0;
			vertexLens[i][Y_COUNT]=0.0;
			old_left_ElementLocation=FVector(0.0,0.0,0.0);
			old_right_ElementLocation=corner[2]-corner[0];
		}
		//screenPos.Y=float(angle[0][1])+((float)i/(float)Z_COUNT)*(angle[1][1]-angle[0][1]);
		FVector2D leftPos=screenP[0]+((float)i/(float)Z_COUNT)*(screenP[1]-screenP[0]);
		FVector2D rightPos=screenP[2]+((float)i/(float)Z_COUNT)*(screenP[3]-screenP[2]);
		for(int j=0;j<Y_COUNT;++j){
			//screenPos.X=float(angle[0][0])+((float)j/(float)Y_COUNT)*(angle[2][0]-angle[0][0]);
			FVector2D interPos=leftPos+((float)j/(float)Y_COUNT)*(rightPos-leftPos);
			PlayerController->GetHitResultAtScreenPosition(interPos, ECC_Visibility, true, Hit);
			elementLocation = Hit.Location;
			//elementLocation += gapLength*Hit.ImpactNormal/Hit.ImpactNormal.Size();
			//cornerNorm[i]=Hit.ImpactNormal;
			avgNormal+=Hit.ImpactNormal;
			FVector shiftV=elementLocation-relativeLocation;
			//vertices.Add(FVector(nl,hl,ul));
			imgVertices.Add(shiftV);
			imgNormals.Add(FVector(1,0,0));
			
			//
			if(j==0){
				vertexLens[i][0]=(elementLocation-old_left_ElementLocation).Size();
				old_left_ElementLocation=elementLocation;
				old_inter_location=elementLocation;
			}else if(j==Y_COUNT-1){
				vertexLens[i][Y_COUNT]=(elementLocation-old_right_ElementLocation).Size();
				old_right_ElementLocation=elementLocation;
				//
				vertexLens[i][j]=(elementLocation-old_inter_location).Size();
			}else{
				vertexLens[i][j]=(elementLocation-old_inter_location).Size();
				old_inter_location=elementLocation;
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT(" imgv num: %d"), imgVertices.Num());
	avgNormal/=avgNormal.Size();
	for(int i=0;i<Z_COUNT*Y_COUNT;++i){
		imgVertices[i]+=gapLength*avgNormal;
	}


	//
	for(int i=0;i<Z_COUNT;++i){
		if(i>0){
			vertexLens[i][0]=(imgVertices[i*Y_COUNT+0]-imgVertices[(i-1)*Y_COUNT+0]).Size();
			vertexLens[i][Y_COUNT]=(imgVertices[i*Y_COUNT+Y_COUNT-1]-imgVertices[(i-1)*Y_COUNT+Y_COUNT-1]).Size();
		}else{
			vertexLens[i][0]=vertexLens[i][Y_COUNT]=0.0;
		}
		//
		for(int j=1;j<Y_COUNT;++j){
			vertexLens[i][j]=(imgVertices[i*Y_COUNT+j]-imgVertices[i*Y_COUNT+j-1]).Size();
		}
	}
	//leftLen=rightLen=0.0;
	for(int i=0;i<Z_COUNT;++i){
		if(i>0){
			vertexLens[i][0]+=vertexLens[i-1][0];
			vertexLens[i][Y_COUNT]+=vertexLens[i-1][Y_COUNT];
		}else{
			vertexLens[i][0]=vertexLens[i][Y_COUNT]=0.0;
		}
		for(int j=2;j<Y_COUNT;++j){
			vertexLens[i][j]+=vertexLens[i][j-1];
		}
	}

	//vertexLens[Z_COUNT-1][0], vertexLens[0][Y_COUNT-1] ,vertexLens[Z_COUNT-1][Y_COUNT], vertexLens[Z_COUNT-1][Y_COUNT-1]

	for(int i=0;i<Z_COUNT;++i){
		vertexLens[i][0] /= vertexLens[Z_COUNT-1][0];
		vertexLens[i][Y_COUNT] /= vertexLens[Z_COUNT-1][Y_COUNT];
	}

	//============
	if(needCalculateBBOX){
		for(int i=0;i<bboxNum;++i){
			UE_LOG(LogUnrealCV, Warning, TEXT("   bn %d"),i);
			float corner2D[4][2];
			corner2D[0][0]=bbox2D[i][0];
			corner2D[0][1]=bbox2D[i][1];
			corner2D[1][0]=bbox2D[i][0];
			corner2D[1][1]=bbox2D[i][3];
			corner2D[2][0]=bbox2D[i][2];
			corner2D[2][1]=bbox2D[i][1];
			corner2D[3][0]=bbox2D[i][2];
			corner2D[3][1]=bbox2D[i][3];
			float cur_value;
			int targetY,targetZ;
			for(int j=0;j<4;++j){
				//find min distance between 4 corner point of corner2D[j]
				
				//find 3D coordinate

				//store in bboxAngle[i][j]
				UE_LOG(LogUnrealCV, Warning, TEXT("   NOOOOO     %.2f %.2f"),corner2D[j][0], corner2D[j][1]);
				float cornerRate_y = corner2D[j][0]/float(textureW);
				float cornerRate_z = corner2D[j][1]/float(textureH);
				//UE_LOG(LogUnrealCV, Warning, TEXT("  BBOX   %.3f %.3f %d %d"), cornerRate_y, cornerRate_z, textureW, textureH);
				float yRate,zRate;
				bool foundT=false;
				for(int zi=1;zi<Z_COUNT;++zi){
					//if(cornerRate_y<)
					for(int yi=1;yi<Y_COUNT;++yi){
						yRate = vertexLens[zi][yi]/vertexLens[zi][Y_COUNT-1];
						zRate = vertexLens[zi][0] + yRate*(vertexLens[zi][Y_COUNT]-vertexLens[zi][0]);
						if(cornerRate_y < yRate && cornerRate_z < zRate){
							targetZ=zi;
							targetY=yi;
							foundT=true;
							break;
						}
					}
					if(foundT)
						break;
				}
				if(!foundT){
					UE_LOG(LogUnrealCV, Warning, TEXT("NOOOOOOOO     %.2f %.2f"),cornerRate_y, cornerRate_z);
					targetZ=1;
					targetY=1;	
				}
				if(targetZ<1)
					targetZ=1;
				else if(targetZ>Z_COUNT-1)
					targetZ=Z_COUNT-1;
				if(targetY<1)
					targetY=1;
				else if(targetY>Y_COUNT-1)
					targetY=Y_COUNT-1;
				//}else{
					float range[4][2];
					if(targetY==1)
						yRate=0.0;
					else
						yRate=vertexLens[targetZ-1][targetY-1]/vertexLens[targetZ-1][Y_COUNT-1];
					zRate = vertexLens[targetZ-1][0] + yRate*(vertexLens[targetZ-1][Y_COUNT]-vertexLens[targetZ-1][0]);
					range[0][0]=yRate;
					range[0][1]=zRate;
					if(targetY==1)
						yRate=0.0;
					else
						yRate=vertexLens[targetZ][targetY-1]/vertexLens[targetZ][Y_COUNT-1];
					zRate = vertexLens[targetZ][0] + yRate*(vertexLens[targetZ][Y_COUNT]-vertexLens[targetZ][0]);
					range[1][0]=yRate;
					range[1][1]=zRate;
					yRate=vertexLens[targetZ-1][targetY]/vertexLens[targetZ-1][Y_COUNT-1];
					zRate = vertexLens[targetZ-1][0] + yRate*(vertexLens[targetZ-1][Y_COUNT]-vertexLens[targetZ-1][0]);
					range[2][0]=yRate;
					range[2][1]=zRate;
					yRate=vertexLens[targetZ][targetY]/vertexLens[targetZ][Y_COUNT-1];
					zRate = vertexLens[targetZ][0] + yRate*(vertexLens[targetZ][Y_COUNT]-vertexLens[targetZ][0]);
					range[3][0]=yRate;
					range[3][1]=zRate;
					//
					UE_LOG(LogUnrealCV, Warning, TEXT("    B %d"),(imgVertices.Num()));
					float curRate=(range[0][1]-cornerRate_z)/(range[0][1]-range[1][1]);
					FVector interL=imgVertices[Y_COUNT*(targetZ-1)+(targetY-1)]+curRate*(imgVertices[Y_COUNT*(targetZ)+(targetY-1)]-imgVertices[Y_COUNT*(targetZ-1)+(targetY-1)]);
					curRate=(range[2][1]-cornerRate_z)/(range[2][1]-range[3][1]);
					UE_LOG(LogUnrealCV, Warning, TEXT("    B"));
					FVector interR=imgVertices[Y_COUNT*(targetZ-1)+(targetY)]+curRate*(imgVertices[Y_COUNT*(targetZ)+(targetY)]-imgVertices[Y_COUNT*(targetZ-1)+(targetY)]);
					curRate=(range[0][0]-cornerRate_y)/(range[0][0]-range[2][0]);
					FVector interM=interL+curRate*(interR-interL);
					interM+=relativeLocation;
					bboxAngle[i][j][0]=interM.X;
					bboxAngle[i][j][1]=interM.Y;
					bboxAngle[i][j][2]=interM.Z;

					FVector upv=(1.0-float(corner2D[j][0])/float(textureW))*corner[0]+(float(corner2D[j][0])/float(textureW))*corner[2];
					FVector downv=(1.0-float(corner2D[j][0])/float(textureW))*corner[1]+(float(corner2D[j][0])/float(textureW))*corner[3];
					interM = (1.0-float(corner2D[j][1])/float(textureH))*upv+(float(corner2D[j][1])/float(textureH))*downv;
					bboxAngle[i][j][0]=interM.X;
					bboxAngle[i][j][1]=interM.Y;
					bboxAngle[i][j][2]=interM.Z;

					UE_LOG(LogUnrealCV, Warning, TEXT("  BBOX   %.3f %.3f %.3f"), interM.X, interM.Y, interM.Z);
				//}
				//
			}
		}
	}

	return true;
}

int max(int a,int b){
	if(a>b)
		return a;
	else
		return b;
}
int min(int a,int b){
	if(a<b)
		return a;
	else
		return b;
}

float CrossProduct2D(FVector2D v1, FVector2D v2){
	return v1.X*v2.Y-v2.X*v1.Y;
}
/*
bool APugTextPawn::CreateStereoText(){
	return false;
}*/

bool APugTextPawn::isOnTheLeft(FVector2D p1, FVector2D p2, FVector2D p){
	float temp = (p1.X-p2.X)/(p1.Y-p2.Y)*(p.Y-p2.Y)+p2.X;
	if(temp>p.X){
		return true;
	}
	return false;
}

bool APugTextPawn::CreateStereoText(){
	vertices.Empty();
	Triangles.Empty();
	normals.Empty();
	UV0.Empty();
	tangents.Empty();
	vertexColors.Empty();

	/*
	imgVertices.Empty();
	imgNormals.Empty();
	float mid_0=(inp[0]+inp[2])*0.5;
	float mid_1=(inp[1]+inp[3])*0.5;
	angle[0][0]=inp[0]+(mid_0-inp[0])*0.01;
	angle[0][1]=inp[1]+(mid_1-inp[1])*0.01;
	angle[1][0]=inp[0]+(mid_0-inp[0])*0.01;
	angle[1][1]=inp[3]+(mid_1-inp[3])*0.01;
	angle[2][0]=inp[2]+(mid_0-inp[2])*0.01;
	angle[2][1]=inp[1]+(mid_1-inp[1])*0.01;
	angle[3][0]=inp[2]+(mid_0-inp[2])*0.01;
	angle[3][1]=inp[3]+(mid_1-inp[3])*0.01;

	FVector corner[4];
	FVector projectedCorner[4];
	FString cornerHitInstanceName;
	FVector cornerNorm[4];
	FVector relativeNorm, relativeHorz, relativeUp, relativeLocation;
	bool hitFlag[4];

	bool bIsMatinee = false;

	float actorL = 1000000000.0;
	FCollisionQueryParams TraceParams(FName(TEXT("TraceUsableActor")), true, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnFaceIndex = true;

	FHitResult Hit(ForceInit);
	FVector TraceEnd;
	FVector TraceStart;
	TraceStart = CameraLocation;

	for(int i=0;i<4;++i)
		hitFlag[i]=false;
	for(int i=0;i<4;++i){
			TraceEnd = CameraLocation;
			TraceEnd.X += actorL * (cos(PAI * (CameraRotation.Pitch+angle[i][1]) / 180.0) * cos(PAI * (CameraRotation.Yaw+angle[i][0]) / 180.0));
			TraceEnd.Y += actorL * (cos(PAI * (CameraRotation.Pitch+angle[i][1]) / 180.0) * sin(PAI * (CameraRotation.Yaw+angle[i][0]) / 180.0));
			TraceEnd.Z += actorL * (sin(PAI * (CameraRotation.Pitch+angle[i][1]) / 180.0));
			// ++ // UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %.3f %.3f %.3f"), i, TraceEnd.X,TraceEnd.Y,TraceEnd.Z);
			bool hitflag = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
			corner[i]=Hit.Location;
			cornerNorm[i]=Hit.ImpactNormal;
			hitFlag[i]=hitflag;
			//cornerHitInstanceName=Hit.GetActor()->GetName();
	}
	if(hitFlag[0]*hitFlag[1]*hitFlag[2]*hitFlag[3]==0){
		//+UE_LOG(LogTemp, Warning, TEXT("NO HIT!!!"));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("NO HIT"));
		return false;
	}
	//
	for(int i=0;i<4;++i){
			//+UE_LOG(LogTemp, Warning, TEXT("angle %d: %.3f %.3f"), i, angle[i][0], angle[i][1]);
			//+UE_LOG(LogTemp, Warning, TEXT("corner %d: %.3f %.3f %.3f"), i, corner[i].X,corner[i].Y,corner[i].Z);
			relativeLocation=corner[i];
			relativeNorm=cornerNorm[i];
			bool normUp=false;
			if(!isEqZero(relativeNorm.X) && !isEqZero(relativeNorm.Y)){
				relativeHorz.Y=fabs(relativeNorm.X / sqrt(relativeNorm.X*relativeNorm.X+relativeNorm.Y*relativeNorm.Y) );
				relativeHorz.X=-relativeNorm.Y*relativeHorz.Y/relativeNorm.X;
				relativeHorz.Z=0.0;
			}else if(!isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y)){
				relativeHorz.X=0.0;
				relativeHorz.Y=1.0;
				relativeHorz.Z=0.0;
			}else if(isEqZero(relativeNorm.X) && !isEqZero(relativeNorm.Y)){
				relativeHorz.X=-1.0;
				relativeHorz.Y=0.0;
				relativeHorz.Z=0.0;
			}else{
				// Norm vector (0,0,1)
				//relativeHorz.X=0.0;
				//relativeHorz.Y=1.0;
				relativeHorz=FVector::CrossProduct(relativeNorm,(CameraLocation-corner[i]));
				normUp=true;
			}
			relativeUp=FVector::CrossProduct(relativeNorm,relativeHorz);
			if(relativeUp.Z<-0.001 && normUp==false){
				//+UE_LOG(LogTemp, Warning, TEXT("** swap up vector **"));
				relativeHorz=-relativeHorz;
				relativeUp=-relativeUp;
			}

			textRotationV = relativeNorm;
			textLocation = relativeLocation;//0.002*(CameraLocation - relativeLocation);
			textRotation = textRotationV.Rotation();
			if(isEqZero(relativeNorm.X) && isEqZero(relativeNorm.Y)){
				textRotation.Yaw=0.0;
			}
			//
			break;
	}
	//+for(int i=0;i<4;i++){
		//+UE_LOG(LogTemp, Warning, TEXT("Corner : %.3f %.3f %.3f"), corner[i].X,corner[i].Y,corner[i].Z);
	//+}
	//+UE_LOG(LogTemp, Warning, TEXT("relativeNorm : %.3f %.3f %.3f"), relativeNorm.X,relativeNorm.Y,relativeNorm.Z);
			
	//+UE_LOG(LogTemp, Warning, TEXT("relativeHorz : %.3f %.3f %.3f"), relativeHorz.X,relativeHorz.Y,relativeHorz.Z);
	
	//+UE_LOG(LogTemp, Warning, TEXT("relativeUp : %.3f %.3f %.3f"), relativeUp.X,relativeUp.Y,relativeUp.Z);
	//
	float up1=distanceInVector(relativeUp,corner[0]-corner[1]);
	float up2=distanceInVector(relativeUp,corner[2]-corner[1]);
	//+UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), up1,up2);
	FVector dirV;
	float betweenAngle;
	if(up1<0.0 || up2<0.0)
		return false;
	if(up1>up2){
		dirV=(corner[1]-corner[0]);
		if(dirV.Size()==0.0)
			return false;
		betweenAngle=FVector::DotProduct(dirV,-relativeUp)/(dirV.Size()*relativeUp.Size());
		corner[0]+=(dirV/dirV.Size())*((up1-up2)/betweenAngle);
	}else{
		dirV=(corner[3]-corner[2]);
		if(dirV.Size()==0.0)
			return false;
		betweenAngle=FVector::DotProduct(dirV,-relativeUp)/(dirV.Size()*relativeUp.Size());
		corner[2]+=(dirV/dirV.Size())*((up2-up1)/betweenAngle);
	}
	//
	float left1=distanceInVector(relativeHorz,corner[0]-corner[2]);
	float left2=distanceInVector(relativeHorz,corner[1]-corner[2]);
	//+UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), left1,left2);
	if(left1<0.0 || left2<0.0)
		return false;
	if(left1>left2){
		dirV=(corner[2]-corner[0]);
		if(dirV.Size()==0.0)
			return false;
		betweenAngle=FVector::DotProduct(dirV,-relativeHorz)/(dirV.Size()*relativeHorz.Size());
		corner[0]+=(dirV/dirV.Size())*((left1-left2)/betweenAngle);
	}else{
		dirV=(corner[3]-corner[1]);
		if(dirV.Size()==0.0)
			return false;
		betweenAngle=FVector::DotProduct(dirV,-relativeHorz)/(dirV.Size()*relativeHorz.Size());
		corner[1]+=(dirV/dirV.Size())*((left2-left1)/betweenAngle);
	}
	//
	float bottom1=distanceInVector(-relativeUp,corner[1]-corner[0]);
	float bottom2=distanceInVector(-relativeUp,corner[3]-corner[0]);
	//+UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), bottom1,bottom2);
	if(bottom1<0.0 || bottom2<0.0)
		return false;
	if(bottom1>bottom2){
		dirV=(corner[0]-corner[1]);
		if(dirV.Size()==0.0)
			return false;
		betweenAngle=FVector::DotProduct(dirV,relativeUp)/(dirV.Size()*relativeUp.Size());
		corner[1]+=(dirV/dirV.Size())*((bottom1-bottom2)/betweenAngle);
	}else{
		dirV=(corner[2]-corner[3]);
		if(dirV.Size()==0.0)
			return false;
		betweenAngle=FVector::DotProduct(dirV,relativeUp)/(dirV.Size()*relativeUp.Size());
		corner[3]+=(dirV/dirV.Size())*((bottom2-bottom1)/betweenAngle);
	}
	//
	float right1=distanceInVector(-relativeHorz,corner[2]-corner[0]);
	float right2=distanceInVector(-relativeHorz,corner[3]-corner[0]);
	//+UE_LOG(LogTemp, Warning, TEXT("up1: %.3f %.3f"), right1,right2);
	if(right1<0.0 || right2<0.0)
		return false;
	if(right1>right2){
		dirV=(corner[0]-corner[2]);
		if(dirV.Size()==0.0)
			return false;
		betweenAngle=FVector::DotProduct(dirV,relativeHorz)/(dirV.Size()*relativeHorz.Size());
		corner[2]+=(dirV/dirV.Size())*((right1-right2)/betweenAngle);
	}else{
		dirV=(corner[1]-corner[3]);
		if(dirV.Size()==0.0)
			return false;
		betweenAngle=FVector::DotProduct(dirV,relativeHorz)/(dirV.Size()*relativeHorz.Size());
		corner[3]+=(dirV/dirV.Size())*((right2-right1)/betweenAngle);
	}
	//
	//+UE_LOG(LogTemp, Warning, TEXT("trac"));
	//CameraRotationV = CameraRotation.Rotation();
	//angle[0][0]=acos(FVector::DotProduct(corner[0]-CameraLocation,CameraRotationV)/((corner[0]-CameraLocation).Size()*CameraRotationV.Size()));
	FRotator corner0Rotator=(corner[0]-CameraLocation).Rotation();
	FRotator corner1Rotator=(corner[1]-CameraLocation).Rotation();
	FRotator corner2Rotator=(corner[2]-CameraLocation).Rotation();
	FRotator corner3Rotator=(corner[3]-CameraLocation).Rotation();
	angle[0][0]=corner0Rotator.Yaw-CameraRotation.Yaw;
	angle[0][1]=corner0Rotator.Pitch-CameraRotation.Pitch;
	angle[1][0]=corner1Rotator.Yaw-CameraRotation.Yaw;
	angle[1][1]=corner1Rotator.Pitch-CameraRotation.Pitch;
	angle[2][0]=corner2Rotator.Yaw-CameraRotation.Yaw;
	angle[2][1]=corner2Rotator.Pitch-CameraRotation.Pitch;
	angle[3][0]=corner3Rotator.Yaw-CameraRotation.Yaw;
	angle[3][1]=corner3Rotator.Pitch-CameraRotation.Pitch;
	//
	for(int i=0;i<4;++i){
			TraceEnd = CameraLocation+(corner[i]-CameraLocation)*1.5;
			// ++ // 
			//+UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %.3f %.3f %.3f"), i, TraceEnd.X,TraceEnd.Y,TraceEnd.Z);
			bool hitflag = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
			if(false == hitflag)
				return false;
			corner[i]=Hit.Location;
			//+UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %.3f"), i, corner[i].X);
			cornerNorm[i]=Hit.ImpactNormal;
			//+UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %.3f"), i, cornerNorm[i].X);
			hitFlag[i]=hitflag;
			//+UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %d"), i, hitflag);
			cornerHitInstanceName=Hit.GetActor()->GetName();
			//+UE_LOG(LogTemp, Warning, TEXT("traceEnd %d: %s"), i, *cornerHitInstanceName);
	}

	UE_LOG(LogTemp, Warning, TEXT("Stereo text!!!"));
	//==================================================
	FVector elementLocation;
	FVector old_left_ElementLocation;
	FVector old_right_ElementLocation;
	FVector old_inter_location;
	FVector elementNormal=FVector(1.0,0.0,0.0);
	for(int i=0;i<Z_COUNT;++i){
		if(i==0){
			vertexLens[i][0]=0.0;
			vertexLens[i][Y_COUNT]=0.0;
			old_left_ElementLocation=FVector(0.0,0.0,0.0);
			old_right_ElementLocation=corner[2]-corner[0];
		}
		float curAngle1_left=angle[0][1]+(angle[1][1]-angle[0][1])*((float)i/(float)Z_COUNT);
		float curAngle1_right=angle[2][1]+(angle[3][1]-angle[2][1])*((float)i/(float)Z_COUNT);
		float curAngle0_left=angle[0][0]+(angle[1][0]-angle[0][0])*((float)i/(float)Z_COUNT);
		float curAngle0_right=angle[2][0]+(angle[3][0]-angle[2][0])*((float)i/(float)Z_COUNT);
		for(int j=0;j<Y_COUNT;++j){
			//float curAngle0=angle[0][0]+(angle[2][0]-angle[0][0])*((float)j/(float)Y_COUNT);
			TraceStart=CameraLocation;
			TraceEnd=CameraLocation;
			float curAngle0=curAngle0_left+(curAngle0_right-curAngle0_left)*((float)j/(float)Y_COUNT);
			float curAngle1=curAngle1_left+(curAngle1_right-curAngle1_left)*((float)j/(float)Y_COUNT);
			TraceEnd.X += actorL * (cos(PAI * (CameraRotation.Pitch + curAngle1) / 180.0) * cos(PAI * (CameraRotation.Yaw + curAngle0) / 180.0));
			TraceEnd.Y += actorL * (cos(PAI * (CameraRotation.Pitch + curAngle1) / 180.0 ) * sin(PAI * (CameraRotation.Yaw + curAngle0) / 180.0));
			TraceEnd.Z += actorL * (sin(PAI * (CameraRotation.Pitch + curAngle1) / 180.0));
			//
			if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams)) {
				elementLocation = Hit.Location;		
				elementLocation += gapLength*Hit.ImpactNormal/Hit.ImpactNormal.Size();
				elementNormal = Hit.ImpactNormal;		
				//UE_LOG(LogTemp, Warning, TEXT("hit: %s"), *Hit.GetActor()->GetName());
				FString curHitInstanceName=Hit.GetActor()->GetName();
				if(cornerHitInstanceName != curHitInstanceName){
					//UE_LOG(LogTemp, Warning, TEXT("different instance"));
					//TraceStart=Hit.Location;
					//if(GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams)){
					//	elementLocation = Hit.Location;	
					//	curHitInstanceName=Hit.GetActor()->GetName();
					//}else{
					//	elementLocation = TraceEnd;
					//	break;
					//}
				}
			}
			else {
				elementLocation = TraceEnd;
			}
			FVector shiftV=elementLocation-relativeLocation;
			//vertices.Add(FVector(nl,hl,ul));
			//vertices.Add(shiftV);
			//normals.Add(FVector(1,0,0));
			//tangents.Add(FProcMeshTangent(0, 1, 0));
			//vertexColors.Add(FLinearColor(0.75, 0.75, 0.75, 1.0));
			imgVertices.Add(shiftV);
			imgNormals.Add(elementNormal);
			//
			if(j==0){
				vertexLens[i][0]=(elementLocation-old_left_ElementLocation).Size();
				old_left_ElementLocation=elementLocation;
				old_inter_location=elementLocation;
			}else if(j==Y_COUNT-1){
				vertexLens[i][Y_COUNT]=(elementLocation-old_right_ElementLocation).Size();
				old_right_ElementLocation=elementLocation;
				//
				vertexLens[i][j]=(elementLocation-old_inter_location).Size();
			}else{
				vertexLens[i][j]=(elementLocation-old_inter_location).Size();
				old_inter_location=elementLocation;
			}
		}
	}
	//
	UE_LOG(LogTemp, Warning, TEXT("Stereo text!!1"));
	for(int i=0;i<Z_COUNT;++i){
		if(i>0){
			vertexLens[i][0]=(imgVertices[i*Y_COUNT+0]-imgVertices[(i-1)*Y_COUNT+0]).Size();
			vertexLens[i][Y_COUNT]=(imgVertices[i*Y_COUNT+Y_COUNT-1]-imgVertices[(i-1)*Y_COUNT+Y_COUNT-1]).Size();
		}else{
			vertexLens[i][0]=vertexLens[i][Y_COUNT]=0.0;
		}
		//
		for(int j=1;j<Y_COUNT;++j){
			vertexLens[i][j]=(imgVertices[i*Y_COUNT+j]-imgVertices[i*Y_COUNT+j-1]).Size();
		}
	}
	//leftLen=rightLen=0.0;
	for(int i=0;i<Z_COUNT;++i){
		if(i>0){
			vertexLens[i][0]+=vertexLens[i-1][0];
			vertexLens[i][Y_COUNT]+=vertexLens[i-1][Y_COUNT];
		}else{
			vertexLens[i][0]=vertexLens[i][Y_COUNT]=0.0;
		}
		for(int j=2;j<Y_COUNT;++j){
			vertexLens[i][j]+=vertexLens[i][j-1];
		}
	}
	*/
	//#####
	UE_LOG(LogTemp, Warning, TEXT("AAA1"));
	float** yRate;
	float** zRate;
	float yRate2, zRate2;
	yRate=new float*[Z_COUNT];
	zRate=new float*[Z_COUNT];
	for(int i=0;i<Z_COUNT;++i){
		yRate[i]=new float[Y_COUNT];
		zRate[i]=new float[Y_COUNT];
	}
	for(int i=1;i<Z_COUNT;++i){
		for(int j=1;j<Y_COUNT;++j){
			yRate[i][j]=vertexLens[i][j]/vertexLens[i][Y_COUNT-1];
			zRate[i][j]=vertexLens[i][0]+yRate[i][j]*(vertexLens[i][Y_COUNT]-vertexLens[i][0]);
		}
	}
	//
	UE_LOG(LogTemp, Warning, TEXT("BBBB"));
	FVector** pixel3DPos;
	pixel3DPos=new FVector*[textureH-1];
	for(int i=0;i<textureH-1;++i)
		pixel3DPos[i]=new FVector[textureW-1];
	for(int i=0;i<textureH-1;++i){
		for(int j=0;j<textureW-1;++j){
			if(1){//mRGBA[i*textureW+j]==1){
				float cornerRate_y=float(j)/float(textureW);
				float cornerRate_z=float(i)/float(textureH);
				bool foundT=false;
				int targetY,targetZ;
				for(int zi=1;zi<Z_COUNT;++zi){
					for(int yi=1;yi<Y_COUNT;++yi){
						if(cornerRate_y<=yRate[zi][yi] && cornerRate_z<=zRate[zi][yi]){
							targetZ=zi;
							targetY=yi;
							foundT=true;
							break;
						}
					}
					if(foundT)
						break;
				}
				if(!foundT){
					targetY=targetZ=1;
				}
				if(targetZ<1)
					targetZ=1;
				else if(targetZ>Z_COUNT-1)
					targetZ=Z_COUNT-1;
				if(targetY<1)
					targetY=1;
				else if(targetY>Y_COUNT-1)
					targetY=Y_COUNT-1;
				//
				float range[4][2];
				if(targetY==1)
					yRate2=0.0;
				else
					yRate2=vertexLens[targetZ-1][targetY-1]/vertexLens[targetZ-1][Y_COUNT-1];
				zRate2 = vertexLens[targetZ-1][0] + yRate2*(vertexLens[targetZ-1][Y_COUNT]-vertexLens[targetZ-1][0]);
				range[0][0]=yRate2;
				range[0][1]=zRate2;
				if(targetY==1)
					yRate2=0.0;
				else
					yRate2=vertexLens[targetZ][targetY-1]/vertexLens[targetZ][Y_COUNT-1];
				zRate2 = vertexLens[targetZ][0] + yRate2*(vertexLens[targetZ][Y_COUNT]-vertexLens[targetZ][0]);
				range[1][0]=yRate2;
				range[1][1]=zRate2;
				yRate2=vertexLens[targetZ-1][targetY]/vertexLens[targetZ-1][Y_COUNT-1];
				zRate2 = vertexLens[targetZ-1][0] + yRate2*(vertexLens[targetZ-1][Y_COUNT]-vertexLens[targetZ-1][0]);
				range[2][0]=yRate2;
				range[2][1]=zRate2;
				yRate2=vertexLens[targetZ][targetY]/vertexLens[targetZ][Y_COUNT-1];
				zRate2 = vertexLens[targetZ][0] + yRate2*(vertexLens[targetZ][Y_COUNT]-vertexLens[targetZ][0]);
				range[3][0]=yRate2;
				range[3][1]=zRate2;
				//
				//UE_LOG(LogUnrealCV, Warning, TEXT("    B %d"),(vertices.Num()));
				float curRate=(range[0][1]-cornerRate_z)/(range[0][1]-range[1][1]);
				FVector interL=imgVertices[Y_COUNT*(targetZ-1)+(targetY-1)]+curRate*(imgVertices[Y_COUNT*(targetZ)+(targetY-1)]-imgVertices[Y_COUNT*(targetZ-1)+(targetY-1)]);
				curRate=(range[2][1]-cornerRate_z)/(range[2][1]-range[3][1]);
				//UE_LOG(LogUnrealCV, Warning, TEXT("    B"));
				FVector interR=imgVertices[Y_COUNT*(targetZ-1)+(targetY)]+curRate*(imgVertices[Y_COUNT*(targetZ)+(targetY)]-imgVertices[Y_COUNT*(targetZ-1)+(targetY)]);
				curRate=(range[0][0]-cornerRate_y)/(range[0][0]-range[2][0]);
				FVector interM=interL+curRate*(interR-interL);
				pixel3DPos[i][j]=interM;
				vertices.Add(interM);
				normals.Add(FVector(0.0,0.0,1.0));
				tangents.Add(FProcMeshTangent(0, 1, 0));
				vertexColors.Add(FLinearColor(0.95, 0.95, 0.95, 1.0));
				UV0.Add(FVector2D(0.8*float(j)/float(textureW-1), 0.8*float(i)/float(textureH-1)));
				
				//===
			}
		}
	}
	//
	UE_LOG(LogTemp, Warning, TEXT("CCCC"));
	for(int i=0;i<textureH-2;++i){
		for(int j=0;j<textureW-2;++j){
			if(mRGBA[i*textureW+j]==1){
				Triangles.Add(i*(textureW-1)+j);
				Triangles.Add((i+1)*(textureW-1)+j);
				Triangles.Add(i*(textureW-1)+j+1);
				//
				Triangles.Add((i+1)*(textureW-1)+j+1);
				Triangles.Add(i*(textureW-1)+j+1);
				Triangles.Add((i+1)*(textureW-1)+j);
				if((i+1)*(textureW-1)+j+1 > vertices.Num())
					UE_LOG(LogTemp, Warning, TEXT("pixel BUG"));
				//cnt+=4;
			}

		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("another gap"));
	//mesh->CreateMeshSection_LinearColor(0, vertices, Triangles, normals, UV0, vertexColors, tangents, true);
	mesh->CreateMeshSection(0, vertices, Triangles, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), true);
	UE_LOG(LogTemp, Warning, TEXT("another gap"));
	// Enable collision data
	mesh->ContainsPhysicsTriMeshData(true);
	UE_LOG(LogTemp, Warning, TEXT("another gap"));
	/*for(int i=0;i<textureH;++i)
		delete[] pixels[i];
	delete[] pixels;
	for(int i=0;i<textureH;++i)
		delete[] pixel_normals[i];
	delete[] pixel_normals;*/
	return true;
}