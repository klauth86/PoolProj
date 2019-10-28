// Fill out your copyright notice in the Description page of Project Settings.

#include "LocalMPViewportClient.h"
#include "Engine.h"

bool ULocalMPViewportClient::InputKey(FViewport* Viewport, int32 ControllerId, FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad) {
	
	UE_LOG(LogTemp, Warning, TEXT("Player input"))
	if (IgnoreInput() || bGamepad || Key.IsMouseButton()) {
		return Super::InputKey(Viewport, ControllerId, Key, EventType, AmountDepressed, bGamepad);
	}
	else {
		// Propagate keyboard events to all players
		UEngine* const Engine = GetOuterUEngine();
		int32 const NumPlayers = Engine ? Engine->GetNumGamePlayers(this) : 0;
		bool bRetVal = false;
		for (int32 i = 0; i < NumPlayers; i++) {
			bRetVal = Super::InputKey(Viewport, i, Key, EventType, AmountDepressed, bGamepad) || bRetVal;
			UE_LOG(LogTemp, Warning, TEXT("%d player input"), i)
		}

		return bRetVal;
	}
}