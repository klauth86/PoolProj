// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "PoolProjZGameMode.h"
#include "PoolProjZCharacter.h"
#include "UObject/ConstructorHelpers.h"

APoolProjZGameMode::APoolProjZGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter4"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
