// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PoolProjCGameModeBase.h"
#include "MyPawn.h"

APoolProjCGameModeBase::APoolProjCGameModeBase() {
	DefaultPawnClass = AMyPawn::StaticClass();
}


