// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PoolProjAGameModeBase.h"

APoolProjAGameModeBase::APoolProjAGameModeBase() {
	DefaultPawnClass = AMyPawn::StaticClass();
}

void APoolProjAGameModeBase::BeginPlay() {
	Super::BeginPlay();
	auto playerCount = 1;
	for (size_t i = 0; i < playerCount; i++) {
		UGameplayStatics::CreatePlayer(this);
	}
};

void APoolProjAGameModeBase::CheckWinCondition() {
	UE_LOG(LogTemp, Warning, TEXT("CheckWinCondition"));
}



