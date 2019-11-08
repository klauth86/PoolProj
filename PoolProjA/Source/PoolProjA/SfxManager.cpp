// Fill out your copyright notice in the Description page of Project Settings.


#include "SfxManager.h"

void USfxManager::SetSfx(AssetsStorageManager* sm) :USfxManager() {
	if (sm) {
		BackgroundSfx = sm->GetBackgroundSfx();
		GainPointSfx = sm->GetGainPointSfx();
		LostPointSfx = sm->GetLostPointSfx();
		HitSfx = sm->GetHitSfx();
		GameOverSfx = sm->GetGameOverSfx();
	}
}

void USfxManager::Subscribe(APoolProjAGameModeBase* Parent) {

}

void USfxManager::Unsubscribe(APoolProjAGameModeBase* Parent) {

}

void USfxManager::PlayBackgroundSfx(bool is2D, FVector location) {
	PlaySfx(BackgroundSfx, is2D, location);
}

void USfxManager::PlayGainPointSfx(bool is2D, FVector location) {
	PlaySfx(GainPointSfx, is2D, location);
}

void USfxManager::PlayLostPointSfx(bool is2D, FVector location) {
	PlaySfx(LostPointSfx, is2D, location);
}

void USfxManager::PlayHitSfx(bool is2D, FVector location) {
	PlaySfx(HitSfx, is2D, location);
}

void USfxManager::PlayGameOverSfx(bool is2D, FVector location) {
	PlaySfx(GameOverSfx, is2D, location);
}

void USfxManager::PlaySfx(USoundBase* sfx, bool is2D, FVector location) {
	if (sfx) {
		if (is2D) {
			UGameplayStatics::PlaySound2D(this, sfx);
		}
		else {
			UGameplayStatics::PlaySoundAtLocation(this, sfx, location);
		}
	}
}
