// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModeWithUI.h"

void AGameModeWithUI::BeginPlay() {
	Super::BeginPlay();

	if (GameUIWidget != nullptr) {
		GameUIInstance = CreateWidget<UUserWidget>(GetWorld(), GameUIWidget);
		if (GameUIInstance != nullptr) {
			GameUIInstance->AddToViewport();
		}
	}
}

