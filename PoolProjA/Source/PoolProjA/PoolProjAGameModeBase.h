// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

#include "Ball.h"
#include "GameModeWithUI.h"

#include "PoolProjAGameModeBase.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class POOLPROJA_API APoolProjAGameModeBase : public AGameModeWithUI
{
	GENERATED_BODY()

public:	

	APoolProjAGameModeBase();

	void CheckWinCondition();

	void ResetScore();

	UPROPERTY(BlueprintReadOnly)
		int Player1Score;

	UPROPERTY(BlueprintReadOnly)
		int Player2Score;

	UPROPERTY(BlueprintReadOnly)
		int ActiveControllerId;

	UPROPERTY(BlueprintReadOnly)
		FName Winner;

protected:

	void BeginPlay() override;
};
