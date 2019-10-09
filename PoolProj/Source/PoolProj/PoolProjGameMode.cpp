// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "PoolProjGameMode.h"
#include "PoolProjCharacter.h"
#include "UObject/ConstructorHelpers.h"

APoolProjGameMode::APoolProjGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
