// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PoolProjAGameModeBase.h"

APoolProjAGameModeBase::APoolProjAGameModeBase() {
	DefaultPawnClass = AMyPawn::StaticClass();
	ResetScore();
}

void APoolProjAGameModeBase::BeginPlay() {
	Super::BeginPlay();
	
	auto playerCount = 1;
	for (size_t i = 0; i < playerCount; i++) {
		UGameplayStatics::CreatePlayer(this);
	}

	if (GameUIWidget != nullptr) {
		GameUIInstance = CreateWidget<UUserWidget>(GetWorld(), GameUIWidget);
		if (GameUIInstance != nullptr) {
			GameUIInstance->AddToViewport();
		}
	}
};

void APoolProjAGameModeBase::CheckWinCondition() {
	auto hasWinner = FMath::Abs(Player1Score - Player2Score) > ABall::InstanceCount();
}

void APoolProjAGameModeBase::ResetScore() {
	Player1Score = 0;
	Player2Score = 0;
}



