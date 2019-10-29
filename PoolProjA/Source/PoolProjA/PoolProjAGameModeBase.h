// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

#include "Ball.h"
#include "MyPawn.h"
#include "Blueprint/UserWidget.h"

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

	void ResetScore();

	UPROPERTY(BlueprintReadOnly)
		int Player1Score;

	UPROPERTY(BlueprintReadOnly)
		int Player2Score;
protected:

	void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI",
		Meta = (BleprintProtected = "true"))
		TSubclassOf<class UUserWidget> GameUIWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI",
		Meta = (BleprintProtected = "true"))
		class UUserWidget* GameUIInstance;
};
