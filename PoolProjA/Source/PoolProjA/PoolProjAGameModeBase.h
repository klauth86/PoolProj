// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

#include "MyPawn.h"

#include "PoolProjAGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class POOLPROJA_API APoolProjAGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:	

	APoolProjAGameModeBase();

	void CheckWinCondition();

protected:

	void BeginPlay() override;
};
