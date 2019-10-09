// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PoolProjNGameModeBase.h"
#include "MyPawn.h"

APoolProjNGameModeBase::APoolProjNGameModeBase() {
	DefaultPawnClass = AMyPawn::StaticClass();
}


