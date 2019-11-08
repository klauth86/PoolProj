// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ConstructorHelpers.h"

UCLASS()
class POOLPROJA_API UAssetsStorageManager : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	USoundBase* GetBackgroundSfx();

	UFUNCTION()
	USoundBase* GetGainPointSfx();

	UFUNCTION()
	USoundBase* GetLostPointSfx();

	UFUNCTION()
	USoundBase* GetHitSfx();

	UFUNCTION()
	USoundBase* GetGameOverSfx();
};
