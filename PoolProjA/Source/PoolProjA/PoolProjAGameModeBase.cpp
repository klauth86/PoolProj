// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PoolProjAGameModeBase.h"
#include "MyPawn.h"

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
};

void APoolProjAGameModeBase::CheckWinCondition() {
	auto hasWinner = ABall::InstanceCount() == 0;
	if (hasWinner) {

		ActiveControllerId = -1;

		if (Player1Score > Player2Score) {
			Winner = FName("Player 1");
		}
		else {
			Winner = FName("Player 2");
		}
	}
}

void APoolProjAGameModeBase::ResetScore() {
	Player1Score = 0;
	Player2Score = 0;
	ActiveControllerId = 0;
}



