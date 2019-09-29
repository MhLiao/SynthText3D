#include "UnrealCVPrivate.h"
#include "ObjectHandler.h"
#include "ObjectPainter.h"
#include "ProceduralMeshComponent.h"
#include "CineCameraActor.h"
//
#include "PugTextPawn.h"
#include <time.h>


FExecStatus GetObjectMobility(const TArray<FString>& Args);

void FObjectCommandHandler::RegisterCommands()
{
	FDispatcherDelegate Cmd;
	FString Help;

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetObjects);
	Help = "Get the name of all objects";
	CommandDispatcher->BindCommand(TEXT("vget /objects"), Cmd, Help);

	// The order matters
	// Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::CurrentObjectHandler); // Redirect to current
	// CommandDispatcher->BindCommand(TEXT("[str] /object/_/[str]"), Cmd, "Get current object");

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetObjectColor);
	Help = "Get the labeling color of an object (used in object instance mask)";
	CommandDispatcher->BindCommand(TEXT("vget /object/[str]/color"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetObjectColor);
	Help = "Set the labeling color of an object [r, g, b]";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/color [uint] [uint] [uint]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetObjectName);
	Help = "[debug] Get the object name";
	CommandDispatcher->BindCommand(TEXT("vget /object/[str]/name"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetObjectLocation);
	Help = "Get object location [x, y, z]";
	CommandDispatcher->BindCommand(TEXT("vget /object/[str]/location"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetObjectRotation);
	Help = "Get object rotation [pitch, yaw, roll]";
	CommandDispatcher->BindCommand(TEXT("vget /object/[str]/rotation"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetObjectLocation);
	Help = "Set object location [x, y, z]";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/location [float] [float] [float]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetObjectRotation);
	Help = "Set object rotation [pitch, yaw, roll]";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/rotation [float] [float] [float]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetPugTextAngle);
	Help = "Set Angles of PugTextPawn";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/angle [uint] [float] [float] [float] [float]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateStatic(GetObjectMobility);
	Help = "Is the object static or movable?";
	CommandDispatcher->BindCommand(TEXT("vget /object/[str]/mobility"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::ShowObject);
	Help = "Show object";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/show"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::HideObject);
	Help = "Hide object";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/hide"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::HidePugText);
	Help = "Hide PugTextPawn";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/texthide [uint] [uint]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetPugTextNeedMove);
	Help = "set the needMove Flag of PugTextPawn";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/needMove [uint] [uint]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetPugTextTexture1);
	Help = "set the texture1 of PugTextPawn";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/texture1 [uint] [str]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetPugTextTexture2);
	Help = "set the texture2 of PugTextPawn";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/texture2 [uint] [str]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetPugTextEmission);
	Help = "set the emission of PugTextPawn";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/emission [uint] [float]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetPugTextLightColor);
	Help = "set the light color of PugTextPawn";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/lightColor [uint] [float] [float] [float]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetPugTextGapLength);
	Help = "set the seperate height of PugTextPawn";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/gapLength [uint] [float]"), Cmd, Help);

	// [pug num] [angle*4] [texture1] [texture2] [needmove] [emission] [lightcolor*3] [seperateheight] [geomFlag] [curvedGapPercent]
	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetPugTextParam);
	Help = "set all the params of PugTextPawn";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/param [uint] [float] [float] [float] [float] [str] [str] [uint] [float] [float] [float] [float] [float] [uint] [float]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::setPugText3DGeomFlag);
	Help = "set the 3d geometry flag of PugTextPawn";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/3DGeomFlag [uint] [uint]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::setPugText3DMaterial);
	Help = "set the 3d material of PugTextPawn";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/3DMat [uint] [uint]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::setPugText3DGeomFlag_withCurveGapPercent);
	Help = "set the 3d geometry flag of PugTextPawn";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/3DGeomFlag [uint] [uint] [float]"), Cmd, Help);

	//no need to call this
	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetPugTextCurvedFlag);
	Help = "set the curve flag of PugTextPawn";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/curveFlag [uint] [uint]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetPugTextcurveGapPercent);
	Help = "set the curve gap percent of PugTextPawn";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/curveGapPercent [uint] [float]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetPugTextBBox2D);
	Help = "set the bbox2D of PugTextPawn";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/bbox2D [uint] [uint] [uint] [float] [float] [float] [float]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetPugTextChar2D);
	Help = "set the bbox2D of char of PugTextPawn";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/char2D [uint] [uint] [uint] [float] [float] [float] [float]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetPugTextLocation);
	Help = "Get object location [x, y, z]";
	CommandDispatcher->BindCommand(TEXT("vget /object/PugTextPawn/location [uint]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetPugTextRotation);
	Help = "Get object rotation [pitch, yaw, roll]";
	CommandDispatcher->BindCommand(TEXT("vget /object/PugTextPawn/rotation [uint]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetPugTextBBox3D);
	Help = "Get object bbox 3D coordinate";
	CommandDispatcher->BindCommand(TEXT("vget /object/PugTextPawn/bbox3D [uint] [uint]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetPugTextBBoxProjected);
	Help = "Get object bbox projected 2D coordinate";
	CommandDispatcher->BindCommand(TEXT("vget /object/PugTextPawn/bboxProjected [uint] [uint]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetPugTextCharProjected);
	Help = "Get object char bbox projected 2D coordinate";
	CommandDispatcher->BindCommand(TEXT("vget /object/PugTextPawn/charProjected [uint] [uint]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetPugTextBBoxCornerRotation);
	Help = "Get object bbox 3D coordinate";
	CommandDispatcher->BindCommand(TEXT("vget /object/PugTextPawn/bboxCornerRotation [uint] [uint]"), Cmd, Help);
}

FExecStatus FObjectCommandHandler::GetObjects(const TArray<FString>& Args)
{
	return FObjectPainter::Get().GetObjectList();
}

FExecStatus FObjectCommandHandler::SetObjectColor(const TArray<FString>& Args)
{
	// ObjectName, R, G, B, A = 255
	// The color format is RGBA
	if (Args.Num() == 4)
	{
		FString ObjectName = Args[0];
		uint32 R = FCString::Atoi(*Args[1]), G = FCString::Atoi(*Args[2]), B = FCString::Atoi(*Args[3]), A = 255; // A = FCString::Atoi(*Args[4]);
		FColor NewColor(R, G, B, A);

		return FObjectPainter::Get().SetActorColor(ObjectName, NewColor);
	}

	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::GetObjectColor(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FString ObjectName = Args[0];
		return FObjectPainter::Get().GetActorColor(ObjectName);
	}

	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::GetObjectName(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		return FExecStatus::OK(Args[0]);
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::CurrentObjectHandler(const TArray<FString>& Args)
{
	// At least one parameter
	if (Args.Num() >= 2)
	{
		FString Uri = "";
		// Get the name of current object
		FHitResult HitResult;
		// The original version for the shooting game use CameraComponent
		APawn* Pawn = FUE4CVServer::Get().GetPawn();
		FVector StartLocation = Pawn->GetActorLocation();
		// FRotator Direction = GetActorRotation();
		FRotator Direction = Pawn->GetControlRotation();

		FVector EndLocation = StartLocation + Direction.Vector() * 10000;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Pawn);

		APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
		if (PlayerController != nullptr)
		{
			FHitResult TraceResult(ForceInit);
			PlayerController->GetHitResultUnderCursor(ECollisionChannel::ECC_WorldDynamic, false, TraceResult);
			FString TraceString;
			if (TraceResult.GetActor() != nullptr)
			{
				TraceString += FString::Printf(TEXT("Trace Actor %s."), *TraceResult.GetActor()->GetName());
			}
			if (TraceResult.GetComponent() != nullptr)
			{
				TraceString += FString::Printf(TEXT("Trace Comp %s."), *TraceResult.GetComponent()->GetName());
			}
			// TheHud->TraceResultText = TraceString;
			// Character->ConsoleOutputDevice->Log(TraceString);
		}
		// TODO: This is not working well.

		if (this->GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
		{
			AActor* HitActor = HitResult.GetActor();

			// UE_LOG(LogUnrealCV, Warning, TEXT("%s"), *HitActor->GetActorLabel());
			// Draw a bounding box of the hitted object and also output the name of it.
			FString ActorName = HitActor->GetHumanReadableName();
			FString Method = Args[0], Property = Args[1];
			Uri = FString::Printf(TEXT("%s /object/%s/%s"), *Method, *ActorName, *Property); // Method name

			for (int32 ArgIndex = 2; ArgIndex < Args.Num(); ArgIndex++) // Vargs
			{
				Uri += " " + Args[ArgIndex];
			}
			FExecStatus ExecStatus = CommandDispatcher->Exec(Uri);
			return ExecStatus;
		}
		else
		{
			return FExecStatus::Error("Can not find current object");
		}
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::GetObjectLocation(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FString ObjectName = Args[0];
		AActor* Object = FObjectPainter::Get().GetObject(ObjectName);
		if (Object == NULL)
		{
			return FExecStatus::Error(FString::Printf(TEXT("Can not find object %s"), *ObjectName));
		}

		FVector Location = Object->GetActorLocation();
		return FExecStatus::OK(FString::Printf(TEXT("Location %.2f %.2f %.2f"), Location.X, Location.Y, Location.Z));
	}

	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::GetObjectRotation(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FString ObjectName = Args[0];
		AActor* Object = FObjectPainter::Get().GetObject(ObjectName);
		if (Object == NULL)
		{
			return FExecStatus::Error(FString::Printf(TEXT("Can not find object %s"), *ObjectName));
		}

		// TODO: support quaternion
		FRotator Rotation = Object->GetActorRotation();
		return FExecStatus::OK(FString::Printf(TEXT("%.2f %.2f %.2f"), Rotation.Pitch, Rotation.Yaw, Rotation.Roll));
	}
	return FExecStatus::InvalidArgument;
}

/** There is no guarantee this will always succeed, for example, hitting a wall */
FExecStatus FObjectCommandHandler::SetObjectLocation(const TArray<FString>& Args)
{
	if (Args.Num() == 4)
	{
		FString ObjectName = Args[0];
		AActor* Object = FObjectPainter::Get().GetObject(ObjectName);
		if (Object == NULL)
		{
			return FExecStatus::Error(FString::Printf(TEXT("Can not find object %s"), *ObjectName));
		}

		// TODO: Check whether this object is movable
		float X = FCString::Atof(*Args[1]), Y = FCString::Atof(*Args[2]), Z = FCString::Atof(*Args[3]);
		FVector NewLocation = FVector(X, Y, Z);
		bool Success = Object->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);

		if (Success)
		{
			return FExecStatus::OK();
		}
		else
		{
			return FExecStatus::Error(FString::Printf(TEXT("Failed to move object %s"), *ObjectName));
		}
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::SetObjectRotation(const TArray<FString>& Args)
{
	if (Args.Num() == 4)
	{
		FString ObjectName = Args[0];
		AActor* Object = FObjectPainter::Get().GetObject(ObjectName);
		if (Object == NULL)
		{
			return FExecStatus::Error(FString::Printf(TEXT("Can not find object %s"), *ObjectName));
		}

		// TODO: Check whether this object is movable
		float Pitch = FCString::Atof(*Args[1]), Yaw = FCString::Atof(*Args[2]), Roll = FCString::Atof(*Args[3]);
		FRotator Rotator = FRotator(Pitch, Yaw, Roll);
		bool Success = Object->SetActorRotation(Rotator);
		return FExecStatus::OK();
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::SetPugTextAngle(const TArray<FString>& Args){
	if (Args.Num() == 6)
	{
		/*FString ObjectName = Args[0];
		AActor* Object = FObjectPainter::Get().GetObject(ObjectName);
		if (Object == NULL)
		{
			return FExecStatus::Error(FString::Printf(TEXT("Can not find object %s"), *ObjectName));
		}*/


		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[1]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}
		Cast<APugTextPawn>(FoundActors[actorNum])->inp[0]=FCString::Atof(*Args[2]);
		Cast<APugTextPawn>(FoundActors[actorNum])->inp[1]=FCString::Atof(*Args[3]);
		Cast<APugTextPawn>(FoundActors[actorNum])->inp[2]=FCString::Atof(*Args[4]);
		Cast<APugTextPawn>(FoundActors[actorNum])->inp[3]=FCString::Atof(*Args[5]);
		return FExecStatus::OK();
	}
	return FExecStatus::Error(FString::Printf(TEXT("absent args!")));
}

FExecStatus GetObjectMobility(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FString ObjectName = Args[0];
		AActor* Object = FObjectPainter::Get().GetObject(ObjectName);
		if (Object == NULL)
		{
			return FExecStatus::Error(FString::Printf(TEXT("Can not find object %s"), *ObjectName));
		}

		FString MobilityName = "";
		EComponentMobility::Type Mobility = Object->GetRootComponent()->Mobility.GetValue();
		switch (Mobility)
		{
		case EComponentMobility::Type::Movable: MobilityName = "Movable"; break;
		case EComponentMobility::Type::Static: MobilityName = "Static"; break;
		case EComponentMobility::Type::Stationary: MobilityName = "Stationary"; break;
		default: MobilityName = "Unknown";
		}
		return FExecStatus::OK(MobilityName);
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::ShowObject(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FString ObjectName = Args[0];
		AActor* Object = FObjectPainter::Get().GetObject(ObjectName);
		if (Object == NULL)
		{
			return FExecStatus::Error(FString::Printf(TEXT("Can not find object %s"), *ObjectName));
		}

		Object->SetActorHiddenInGame(false);
		return FExecStatus::OK();
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::HideObject(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FString ObjectName = Args[0];
		AActor* Object = FObjectPainter::Get().GetObject(ObjectName);
		if (Object == NULL)
		{
			return FExecStatus::Error(FString::Printf(TEXT("Can not find object %s"), *ObjectName));
		}

		Object->SetActorHiddenInGame(true);
		return FExecStatus::OK();
	}
	return FExecStatus::InvalidArgument;
}


FExecStatus FObjectCommandHandler::HidePugText(const TArray<FString>& Args)
{
	if (Args.Num() == 3)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[1]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}
		if(FCString::Atof(*Args[2]) == 0){
			FoundActors[actorNum]->SetActorHiddenInGame(false);
		}else{
			FoundActors[actorNum]->SetActorHiddenInGame(true);
		}
		
		return FExecStatus::OK();

	}
	return FExecStatus::InvalidArgument;
}

//SetPugTextNeedMove
FExecStatus FObjectCommandHandler::SetPugTextNeedMove(const TArray<FString>& Args)
{
	if (Args.Num() == 3)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[1]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}
		if(FCString::Atof(*Args[2]) == 0){
			Cast<APugTextPawn>(FoundActors[actorNum])->PrimaryActorTick.bCanEverTick=false;//needMove=false;
			Cast<APugTextPawn>(FoundActors[actorNum])->needMove=false;
		}
		else{
			Cast<APugTextPawn>(FoundActors[actorNum])->PrimaryActorTick.bCanEverTick=true;//needMove=true;
			Cast<APugTextPawn>(FoundActors[actorNum])->needMove=true;
		}
		return FExecStatus::OK();

	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::SetPugTextTexture1(const TArray<FString>& Args)
{
	if (Args.Num() == 3)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[1]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}
		Cast<APugTextPawn>(FoundActors[actorNum])->texture1_name=Args[2];
		return FExecStatus::OK();

	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::SetPugTextTexture2(const TArray<FString>& Args)
{
	if (Args.Num() == 3)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[1]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}
		Cast<APugTextPawn>(FoundActors[actorNum])->texture2_name=Args[2];
		return FExecStatus::OK();

	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::SetPugTextEmission(const TArray<FString>& Args)
{
	if (Args.Num() == 3)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[1]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}
		Cast<APugTextPawn>(FoundActors[actorNum])->Emission = FCString::Atof(*Args[2]);
		return FExecStatus::OK();

	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::SetPugTextGapLength(const TArray<FString>& Args)
{
	if (Args.Num() == 3)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[1]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}
		Cast<APugTextPawn>(FoundActors[actorNum])->gapLength = FCString::Atof(*Args[2]);
		return FExecStatus::OK();

	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::SetPugTextLightColor(const TArray<FString>& Args)
{
	if (Args.Num() == 5)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[1]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}
		//Cast<APugTextPawn>(FoundActors[actorNum])->lightColor[0]=FCString::Atof(*Args[2]);
		//Cast<APugTextPawn>(FoundActors[actorNum])->lightColor[1]=FCString::Atof(*Args[3]);
		//Cast<APugTextPawn>(FoundActors[actorNum])->lightColor[2]=FCString::Atof(*Args[4]);

		FLinearColor lcolor=FLinearColor(FCString::Atof(*Args[2]),FCString::Atof(*Args[3]),FCString::Atof(*Args[4]),1.0);
		Cast<APugTextPawn>(FoundActors[actorNum])->lightColor = lcolor;
		return FExecStatus::OK();

	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::SetPugTextParam(const TArray<FString>& Args){
	if (Args.Num() == 16)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[1]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}
		Cast<APugTextPawn>(FoundActors[actorNum])->inp[0]=FCString::Atof(*Args[2]);
		Cast<APugTextPawn>(FoundActors[actorNum])->inp[1]=FCString::Atof(*Args[3]);
		Cast<APugTextPawn>(FoundActors[actorNum])->inp[2]=FCString::Atof(*Args[4]);
		Cast<APugTextPawn>(FoundActors[actorNum])->inp[3]=FCString::Atof(*Args[5]);
		Cast<APugTextPawn>(FoundActors[actorNum])->texture1_name=Args[6];
		Cast<APugTextPawn>(FoundActors[actorNum])->texture2_name=Args[7];
		if(FCString::Atof(*Args[8]) == 0){
			Cast<APugTextPawn>(FoundActors[actorNum])->PrimaryActorTick.bCanEverTick=false;//needMove=false;
			Cast<APugTextPawn>(FoundActors[actorNum])->needMove=false;
		}
		else{
			Cast<APugTextPawn>(FoundActors[actorNum])->PrimaryActorTick.bCanEverTick=true;//needMove=true;
			Cast<APugTextPawn>(FoundActors[actorNum])->needMove=true;
		}
		Cast<APugTextPawn>(FoundActors[actorNum])->Emission = FCString::Atof(*Args[9]);
		FLinearColor lcolor=FLinearColor(FCString::Atof(*Args[10]),FCString::Atof(*Args[11]),FCString::Atof(*Args[12]),1.0);
		Cast<APugTextPawn>(FoundActors[actorNum])->lightColor = lcolor;
		Cast<APugTextPawn>(FoundActors[actorNum])->gapLength = FCString::Atof(*Args[13]);
		//
		if(FCString::Atoi(*Args[14]) == 0){
			Cast<APugTextPawn>(FoundActors[actorNum])->geomFlag = false;
		}else{
			Cast<APugTextPawn>(FoundActors[actorNum])->geomFlag = true;
		}
		if(FCString::Atof(*Args[15]) <= 0.001){
			Cast<APugTextPawn>(FoundActors[actorNum])->needCurved = false;
		}else{
			Cast<APugTextPawn>(FoundActors[actorNum])->needCurved = true;
			Cast<APugTextPawn>(FoundActors[actorNum])->curveGapPercent = FCString::Atof(*Args[3]);
		}
		return FExecStatus::OK();
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::setPugText3DGeomFlag(const TArray<FString>& Args){
	if (Args.Num() == 3)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[1]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}
		if(FCString::Atoi(*Args[2]) == 0){
			Cast<APugTextPawn>(FoundActors[actorNum])->geomFlag = false;
		}else{
			Cast<APugTextPawn>(FoundActors[actorNum])->geomFlag = true;
		}
		return FExecStatus::OK();	
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::setPugText3DMaterial(const TArray<FString>& Args){
	if (Args.Num() == 3)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[1]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}

		Cast<APugTextPawn>(FoundActors[actorNum])->geomMaterialNum = FCString::Atoi(*Args[2]);
		return FExecStatus::OK();	
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::setPugText3DGeomFlag_withCurveGapPercent(const TArray<FString>& Args){
	if (Args.Num() == 4)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[1]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}
		if(FCString::Atoi(*Args[2]) == 0){
			Cast<APugTextPawn>(FoundActors[actorNum])->geomFlag = false;
		}else{
			Cast<APugTextPawn>(FoundActors[actorNum])->geomFlag = true;
		}
		if(FCString::Atof(*Args[3]) <= 0.001){
			Cast<APugTextPawn>(FoundActors[actorNum])->needCurved = false;
		}else{
			Cast<APugTextPawn>(FoundActors[actorNum])->needCurved = true;
			Cast<APugTextPawn>(FoundActors[actorNum])->curveGapPercent = FCString::Atof(*Args[3]);
		}
		return FExecStatus::OK();	
	}
	return FExecStatus::InvalidArgument;
}


FExecStatus FObjectCommandHandler::SetPugTextcurveGapPercent(const TArray<FString>& Args){
	if (Args.Num() == 3)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[1]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}
		Cast<APugTextPawn>(FoundActors[actorNum])->curveGapPercent = FCString::Atof(*Args[2]);
		return FExecStatus::OK();	
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::SetPugTextCurvedFlag(const TArray<FString>& Args){
	if (Args.Num() == 3)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[1]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}
		if(FCString::Atoi(*Args[2]) == 0){
			Cast<APugTextPawn>(FoundActors[actorNum])->needCurved = false;
		}else{
			Cast<APugTextPawn>(FoundActors[actorNum])->needCurved = true;
		}
		return FExecStatus::OK();	
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::SetPugTextBBox2D(const TArray<FString>& Args){
	if (Args.Num() == 8)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[1]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}
		
		int bn=FCString::Atoi(*Args[3]);
		if(bn-1==FCString::Atoi(*Args[2])){
			Cast<APugTextPawn>(FoundActors[actorNum])->bboxNum=bn;
			Cast<APugTextPawn>(FoundActors[actorNum])->needCalculateBBOX=true;
		}else{
			Cast<APugTextPawn>(FoundActors[actorNum])->needCalculateBBOX=false;
		}
		bn=FCString::Atoi(*Args[2]);
		Cast<APugTextPawn>(FoundActors[actorNum])->bbox2D[bn][0]=FCString::Atof(*Args[4]);
		Cast<APugTextPawn>(FoundActors[actorNum])->bbox2D[bn][1]=FCString::Atof(*Args[5]);
		Cast<APugTextPawn>(FoundActors[actorNum])->bbox2D[bn][2]=FCString::Atof(*Args[6]);
		Cast<APugTextPawn>(FoundActors[actorNum])->bbox2D[bn][3]=FCString::Atof(*Args[7]);
		srand(time(NULL));
		//int resizeFlag=rand()%10;
		Cast<APugTextPawn>(FoundActors[actorNum])->resizeFlag=10;//rand()%10;
		return FExecStatus::OK();
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::SetPugTextChar2D(const TArray<FString>& Args){
	if (Args.Num() == 8)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[1]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}
		
		int bn=FCString::Atoi(*Args[3]);
		if(bn-1==FCString::Atoi(*Args[2])){
			Cast<APugTextPawn>(FoundActors[actorNum])->charNum=bn;
			Cast<APugTextPawn>(FoundActors[actorNum])->needCalculateCharBBOX=true;
		}else{
			Cast<APugTextPawn>(FoundActors[actorNum])->needCalculateCharBBOX=false;
		}
		bn=FCString::Atoi(*Args[2]);
		Cast<APugTextPawn>(FoundActors[actorNum])->char2D[bn][0]=FCString::Atof(*Args[4]);
		Cast<APugTextPawn>(FoundActors[actorNum])->char2D[bn][1]=FCString::Atof(*Args[5]);
		Cast<APugTextPawn>(FoundActors[actorNum])->char2D[bn][2]=FCString::Atof(*Args[6]);
		Cast<APugTextPawn>(FoundActors[actorNum])->char2D[bn][3]=FCString::Atof(*Args[7]);
		srand(time(NULL));
		//int resizeFlag=rand()%10;
		//Cast<APugTextPawn>(FoundActors[actorNum])->resizeFlag=10;//rand()%10;
		return FExecStatus::OK();
	}
	return FExecStatus::InvalidArgument;
}


FExecStatus FObjectCommandHandler::GetPugTextLocation(const TArray<FString>& Args){
	if (Args.Num() == 1)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[0]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}

		FVector Location = Cast<APugTextPawn>(FoundActors[actorNum])->GetActorLocation();
		return FExecStatus::OK(FString::Printf(TEXT("Location %.2f %.2f %.2f"), Location.X, Location.Y, Location.Z));
	}

	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::GetPugTextRotation(const TArray<FString>& Args){
	if (Args.Num() == 1)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[0]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}

		FRotator Rotation = Cast<APugTextPawn>(FoundActors[actorNum])->GetActorRotation();
		return FExecStatus::OK(FString::Printf(TEXT("%.2f,%.2f,%.2f"), Rotation.Pitch, Rotation.Yaw, Rotation.Roll));
	}

	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::GetPugTextBBox3D(const TArray<FString>& Args){
	if (Args.Num() == 2)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[0]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}

		//FRotator Rotation = Cast<APugTextPawn>(FoundActors[actorNum])->GetActorRotation();
		int bboxNum=FCString::Atoi(*Args[1]);
		float cord1=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][0][0];
		float cord2=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][0][1];
		float cord3=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][0][2];
		//
		float cord4=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][1][0];
		float cord5=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][1][1];
		float cord6=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][1][2];
		//
		float cord7=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][2][0];
		float cord8=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][2][1];
		float cord9=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][2][2];
		//
		float cord10=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][3][0];
		float cord11=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][3][1];
		float cord12=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][3][2];

		return FExecStatus::OK(FString::Printf(TEXT("%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f"), cord1, cord2, cord3, cord4, cord5, cord6, cord7, cord8, cord9, cord10, cord11, cord12));
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::GetPugTextBBoxProjected(const TArray<FString>& Args){
	if (Args.Num() == 2)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[0]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}

		//FRotator Rotation = Cast<APugTextPawn>(FoundActors[actorNum])->GetActorRotation();
		int bboxNum=FCString::Atoi(*Args[1]);
		float cord1=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][0][0];
		float cord2=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][0][1];
		float cord3=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][0][2];
		//
		float cord4=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][1][0];
		float cord5=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][1][1];
		float cord6=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][1][2];
		//
		float cord7=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][2][0];
		float cord8=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][2][1];
		float cord9=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][2][2];
		//
		float cord10=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][3][0];
		float cord11=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][3][1];
		float cord12=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][3][2];

		FVector worldP=FVector(cord1, cord2, cord3);
		FVector2D screenP1=FVector2D(0.0,0.0);
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->ProjectWorldLocationToScreen(worldP,screenP1);
		//
		worldP=FVector(cord4, cord5, cord6);
		FVector2D screenP2=FVector2D(0.0,0.0);
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->ProjectWorldLocationToScreen(worldP,screenP2);
		//
		worldP=FVector(cord7, cord8, cord9);
		FVector2D screenP3=FVector2D(0.0,0.0);
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->ProjectWorldLocationToScreen(worldP,screenP3);
		//
		worldP=FVector(cord10, cord11, cord12);
		FVector2D screenP4=FVector2D(0.0,0.0);
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->ProjectWorldLocationToScreen(worldP,screenP4);
		//
		
		return FExecStatus::OK(FString::Printf(TEXT("%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f"), screenP1.X, screenP1.Y, screenP2.X, screenP2.Y, screenP3.X, screenP3.Y, screenP4.X, screenP4.Y));
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::GetPugTextCharProjected(const TArray<FString>& Args){
	if (Args.Num() == 2)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[0]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}

		//FRotator Rotation = Cast<APugTextPawn>(FoundActors[actorNum])->GetActorRotation();
		int bboxNum=FCString::Atoi(*Args[1]);
		float cord1=Cast<APugTextPawn>(FoundActors[actorNum])->charAngle[bboxNum][0][0];
		float cord2=Cast<APugTextPawn>(FoundActors[actorNum])->charAngle[bboxNum][0][1];
		float cord3=Cast<APugTextPawn>(FoundActors[actorNum])->charAngle[bboxNum][0][2];
		//
		float cord4=Cast<APugTextPawn>(FoundActors[actorNum])->charAngle[bboxNum][1][0];
		float cord5=Cast<APugTextPawn>(FoundActors[actorNum])->charAngle[bboxNum][1][1];
		float cord6=Cast<APugTextPawn>(FoundActors[actorNum])->charAngle[bboxNum][1][2];
		//
		float cord7=Cast<APugTextPawn>(FoundActors[actorNum])->charAngle[bboxNum][2][0];
		float cord8=Cast<APugTextPawn>(FoundActors[actorNum])->charAngle[bboxNum][2][1];
		float cord9=Cast<APugTextPawn>(FoundActors[actorNum])->charAngle[bboxNum][2][2];
		//
		float cord10=Cast<APugTextPawn>(FoundActors[actorNum])->charAngle[bboxNum][3][0];
		float cord11=Cast<APugTextPawn>(FoundActors[actorNum])->charAngle[bboxNum][3][1];
		float cord12=Cast<APugTextPawn>(FoundActors[actorNum])->charAngle[bboxNum][3][2];

		FVector worldP=FVector(cord1, cord2, cord3);
		FVector2D screenP1=FVector2D(0.0,0.0);
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->ProjectWorldLocationToScreen(worldP,screenP1);
		//
		worldP=FVector(cord4, cord5, cord6);
		FVector2D screenP2=FVector2D(0.0,0.0);
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->ProjectWorldLocationToScreen(worldP,screenP2);
		//
		worldP=FVector(cord7, cord8, cord9);
		FVector2D screenP3=FVector2D(0.0,0.0);
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->ProjectWorldLocationToScreen(worldP,screenP3);
		//
		worldP=FVector(cord10, cord11, cord12);
		FVector2D screenP4=FVector2D(0.0,0.0);
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->ProjectWorldLocationToScreen(worldP,screenP4);
		//
		
		return FExecStatus::OK(FString::Printf(TEXT("%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f"), screenP1.X, screenP1.Y, screenP2.X, screenP2.Y, screenP3.X, screenP3.Y, screenP4.X, screenP4.Y));
	}
	return FExecStatus::InvalidArgument;
}


FExecStatus FObjectCommandHandler::GetPugTextBBoxCornerRotation(const TArray<FString>& Args){
	if (Args.Num() == 2)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APugTextPawn::StaticClass(), FoundActors);
		//UE_LOG(LogTemp, Warning, TEXT(" Pug Num %d"), FoundActors.Num());
		if(FoundActors.Num()<1)
			return FExecStatus::Error(FString::Printf(TEXT(" No PugTextPawn!")));
		//UE_LOG(LogTemp, Warning, TEXT(" args1 %s"), Args[1]);
		int actorNum = FCString::Atoi(*Args[0]);
		if(actorNum>=FoundActors.Num()){
			//UE_LOG(LogTemp, Warning, TEXT(" args1 out of range!"));
			return FExecStatus::Error(FString::Printf(TEXT(" args1 out of range!")));
		}


		bool bIsMatinee = false;

		FVector CameraLocation;
		ACineCameraActor* CineCameraActor = nullptr;
		for (AActor* Actor : this->GetWorld()->GetCurrentLevel()->Actors)
		{
			// if (Actor && Actor->IsA(AMatineeActor::StaticClass())) // AMatineeActor is deprecated
			if (Actor && Actor->IsA(ACineCameraActor::StaticClass()))
			{
				bIsMatinee = true;
				CameraLocation = Actor->GetActorLocation();
				break;
			}
		}

		if (!bIsMatinee)
		{
			int32 CameraId = 0;//FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
			// APawn* Pawn = FUE4CVServer::Get().GetPawn();
			// CameraLocation = Pawn->GetActorLocation();
			UGTCaptureComponent* CaptureComponent = FCaptureManager::Get().GetCamera(CameraId);
			if (CaptureComponent == nullptr)
			{
				return FExecStatus::Error(FString::Printf(TEXT("Camera %d can not be found."), CameraId));
			}
			CameraLocation = CaptureComponent->GetComponentLocation();
		}

		//FRotator Rotation = Cast<APugTextPawn>(FoundActors[actorNum])->GetActorRotation();
		int bboxNum=FCString::Atoi(*Args[1]);
		float cord1=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][0][0];
		float cord2=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][0][1];
		float cord3=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][0][2];
		FVector cornerDir = FVector(cord1,cord2,cord3);
		cornerDir-=CameraLocation;
		FRotator cornerRotator=cornerDir.Rotation();
		cord1=cornerRotator.Pitch;
		cord2=cornerRotator.Yaw;
		cord3=cornerRotator.Roll;
		//
		float cord4=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][1][0];
		float cord5=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][1][1];
		float cord6=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][1][2];
		cornerDir = FVector(cord4,cord5,cord6);
		cornerDir-=CameraLocation;
		cornerRotator=cornerDir.Rotation();
		cord4=cornerRotator.Pitch;
		cord5=cornerRotator.Yaw;
		cord6=cornerRotator.Roll;
		//
		float cord7=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][2][0];
		float cord8=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][2][1];
		float cord9=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][2][2];
		cornerDir = FVector(cord7,cord8,cord9);
		cornerDir-=CameraLocation;
		cornerRotator=cornerDir.Rotation();
		cord7=cornerRotator.Pitch;
		cord8=cornerRotator.Yaw;
		cord9=cornerRotator.Roll;
		//
		float cord10=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][3][0];
		float cord11=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][3][1];
		float cord12=Cast<APugTextPawn>(FoundActors[actorNum])->bboxAngle[bboxNum][3][2];
		cornerDir = FVector(cord10,cord11,cord12);
		cornerDir-=CameraLocation;
		cornerRotator=cornerDir.Rotation();
		cord10=cornerRotator.Pitch;
		cord11=cornerRotator.Yaw;
		cord12=cornerRotator.Roll;

		return FExecStatus::OK(FString::Printf(TEXT("%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f"), cord1, cord2, cord3, cord4, cord5, cord6, cord7, cord8, cord9, cord10, cord11, cord12));
	}
	return FExecStatus::InvalidArgument;
}