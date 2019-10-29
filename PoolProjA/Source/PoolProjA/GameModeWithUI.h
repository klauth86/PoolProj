// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "GameModeWithUI.generated.h"

/**
 * 
 */
UCLASS()
class POOLPROJA_API AGameModeWithUI : public AGameModeBase
{
	GENERATED_BODY()	
	
protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI",
		Meta = (BleprintProtected = "true"))
		TSubclassOf<class UUserWidget> GameUIWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI",
		Meta = (BleprintProtected = "true"))
		class UUserWidget* GameUIInstance;	
};
