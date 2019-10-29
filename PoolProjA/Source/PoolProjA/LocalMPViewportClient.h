// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
#include "Engine/GameViewportClient.h"

#include "LocalMPViewportClient.generated.h"

/**
 * 
 */
UCLASS()
class POOLPROJA_API ULocalMPViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

	virtual bool InputKey(FViewport* Viewport, int32 ControllerId, FKey Key, EInputEvent EventType,
			float AmountDepressed = 1.f, bool bGamepad = false) override;
};
