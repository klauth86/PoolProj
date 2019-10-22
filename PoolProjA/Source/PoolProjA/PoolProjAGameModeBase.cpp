// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PoolProjAGameModeBase.h"
#include "MyPawn.h"

APoolProjAGameModeBase::APoolProjAGameModeBase() {
	DefaultPawnClass = AMyPawn::StaticClass();
}


