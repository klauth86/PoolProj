// Fill out your copyright notice in the Description page of Project Settings.

#include "PoolProjAGameModeBase.h"
#include "MyPawn.h"

APoolProjAGameModeBase* APoolProjAGameModeBase::_currentGameMode = 0;

APoolProjAGameModeBase::APoolProjAGameModeBase() {
	DefaultPawnClass = AMyPawn::StaticClass();
	ResetScore();
}

void APoolProjAGameModeBase::BeginPlay() {
	Super::BeginPlay();
	
	_currentGameMode = this;

	auto existingNumPlayers = GetNumPlayers();
	auto targetNumPlayers = 2;
	for (size_t i = existingNumPlayers; i < targetNumPlayers; i++) {
		UGameplayStatics::CreatePlayer(this);
	}
};

void APoolProjAGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	_currentGameMode = 0;
}

void APoolProjAGameModeBase::CheckWinCondition(int ballCount) {
	if (ballCount == 0) {

		ActiveControllerId = -1;

		if (Player1Score > Player2Score) {
			Winner = FName("Player 1");
		}
		else {
			Winner = FName("Player 2");
		}

		UE_LOG(LogTemp, Warning, TEXT("CheckWinCondition %d"), ballCount);
	}
}

void APoolProjAGameModeBase::ResetScore() {
	Player1Score = 0;
	Player2Score = 0;
	ActiveControllerId = 0;
}



