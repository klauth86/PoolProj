// Fill out your copyright notice in the Description page of Project Settings.
DECLARE_EVENT(APoolProjAGameModeBase, GainPointEvent)
DECLARE_EVENT(APoolProjAGameModeBase, LostPointEvent)
DECLARE_EVENT_OneParam(APoolProjAGameModeBase, HitEvent, FVector)
DECLARE_EVENT(APoolProjAGameModeBase, GameOverEvent)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

#include "GameModeWithUI.h"
#include "Managers/AssetsStorageManager.h"
#include "SfxManager.h"

#include "PoolProjAGameModeBase.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class POOLPROJA_API APoolProjAGameModeBase : public AGameModeWithUI
{
	GENERATED_BODY()

public:	

#pragma region MANAGERS

	AssetsStorageManager AssetsManager;
	USfxManager* SfxManager;

#pragma endregion


#pragma region EVENTS

	GainPointEvent OnGainPointEvent;
	LostPointEvent OnLostPointEvent;
	HitEvent OnHitEvent;
	GameOverEvent OnGameOverEvent;

#pragma endregion

	APoolProjAGameModeBase();

	void CheckWinCondition(int ballCount);

	void ResetScore();

	UPROPERTY(BlueprintReadOnly)
		int Player1Score;

	UPROPERTY(BlueprintReadOnly)
		int Player2Score;

	UPROPERTY(BlueprintReadOnly)
		int ActiveControllerId;

	UPROPERTY(BlueprintReadOnly)
		FName Winner;

	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:

	void BeginPlay() override;

#pragma region STATIC

public:
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "APoolProjAGameModeBase")
	static APoolProjAGameModeBase* GetCurrentGameMode() { return _currentGameMode; };

protected:

	static APoolProjAGameModeBase* _currentGameMode;

#pragma endregion
};
