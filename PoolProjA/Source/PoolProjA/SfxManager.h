// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "PoolProjAGameModeBase.h"

#include "AssetsStorageManager.h"

#include "SfxManager.generated.h"

/**
 * 
 */
UCLASS()
class POOLPROJA_API USfxManager : public UObject
{
	GENERATED_BODY()
	
public:	
	UFUNCTION()
		void SetSfx(AssetsStorageManager* sm);

	UFUNCTION()
		void Subscribe(APoolProjAGameModeBase* Parent);

	UFUNCTION()
		void Unsubscribe(APoolProjAGameModeBase* Parent);

private:
	UFUNCTION()
		void PlayBackgroundSfx(bool is2D, FVector location = FVector::ZeroVector);

	UFUNCTION()
		void PlayGainPointSfx(bool is2D, FVector location = FVector::ZeroVector);

	UFUNCTION()
		void PlayLostPointSfx(bool is2D, FVector location = FVector::ZeroVector);

	UFUNCTION()
		void PlayHitSfx(bool is2D, FVector location = FVector::ZeroVector);

	UFUNCTION()
		void PlayGameOverSfx(bool is2D, FVector location = FVector::ZeroVector);

	UFUNCTION()
		void PlaySfx(USoundBase* sfx, bool is2D, FVector location = FVector::ZeroVector);

	UPROPERTY()
		USoundBase* BackgroundSfx;
	UPROPERTY()
		USoundBase* GainPointSfx;
	UPROPERTY()
		USoundBase* LostPointSfx;
	UPROPERTY()
		USoundBase* HitSfx;
	UPROPERTY()
		USoundBase* GameOverSfx;
};
