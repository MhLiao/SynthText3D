#pragma once
#include "CommandDispatcher.h"
#include "GTCaptureComponent.h"
#include "Runtime/Engine/Classes/Engine/GameViewportClient.h"

class FObjectCommandHandler : public FCommandHandler
{
public:
	FObjectCommandHandler(FCommandDispatcher* InCommandDispatcher) : FCommandHandler(InCommandDispatcher)
	{}
	void RegisterCommands();

	/** Get a list of all objects in the scene */
	FExecStatus GetObjects(const TArray<FString>& Args);
	/** Get the annotation color of an object (Notice: not the appearance color) */
	FExecStatus GetObjectColor(const TArray<FString>& Args);
	/** Set the annotation color of an object */
	FExecStatus SetObjectColor(const TArray<FString>& Args);
	/** Get the name of an object */
	FExecStatus GetObjectName(const TArray<FString>& Args);

	FExecStatus CurrentObjectHandler(const TArray<FString>& Args);

	/** Get object location */
	FExecStatus GetObjectLocation(const TArray<FString>& Args);

	/** Get object rotation */
	FExecStatus GetObjectRotation(const TArray<FString>& Args);

	/** Set object location */
	FExecStatus SetObjectLocation(const TArray<FString>& Args);

	/** Set object rotation */
	FExecStatus SetObjectRotation(const TArray<FString>& Args);

	/** Set angles */
	FExecStatus SetPugTextAngle(const TArray<FString>& Args);

	/** Show object */
	FExecStatus ShowObject(const TArray<FString>& Args);

	/** Hide object */
	FExecStatus HideObject(const TArray<FString>& Args);

	FExecStatus HidePugText(const TArray<FString>& Args);

	FExecStatus SetPugTextNeedMove(const TArray<FString>& Args);

	FExecStatus SetPugTextTexture1(const TArray<FString>& Args);

	FExecStatus SetPugTextTexture2(const TArray<FString>& Args);

	FExecStatus SetPugTextEmission(const TArray<FString>& Args);

	FExecStatus SetPugTextLightColor(const TArray<FString>& Args);

	FExecStatus SetPugTextGapLength(const TArray<FString>& Args);

	FExecStatus SetPugTextParam(const TArray<FString>& Args);

	FExecStatus setPugText3DGeomFlag(const TArray<FString>& Args);

	FExecStatus setPugText3DMaterial(const TArray<FString>& Args);

	FExecStatus setPugText3DGeomFlag_withCurveGapPercent(const TArray<FString>& Args);

	FExecStatus SetPugTextCurvedFlag(const TArray<FString>& Args);

	FExecStatus SetPugTextcurveGapPercent(const TArray<FString>& Args);

	FExecStatus SetPugTextBBox2D(const TArray<FString>& Args);
	FExecStatus SetPugTextChar2D(const TArray<FString>& Args);

	FExecStatus GetPugTextLocation(const TArray<FString>& Args);
	FExecStatus GetPugTextRotation(const TArray<FString>& Args);

	FExecStatus GetPugTextBBox3D(const TArray<FString>& Args);

	FExecStatus GetPugTextBBoxProjected(const TArray<FString>& Args);
	FExecStatus GetPugTextCharProjected(const TArray<FString>& Args);

	FExecStatus GetPugTextBBoxCornerRotation(const TArray<FString>& Args);
};
