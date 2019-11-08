// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetsStorageManager.h"

USoundBase* UAssetsStorageManager::GetBackgroundSfx() {
	ConstructorHelpers::FObjectFinder<USoundBase> sfxAsset(TEXT("SoundWave'/Game/SFX/BackgroundMusic.BackgroundMusic'"));
	return sfxAsset.Object;
}

USoundBase* UAssetsStorageManager::GetGainPointSfx() {
	ConstructorHelpers::FObjectFinder<USoundBase> sfxAsset(TEXT("SoundWave'/Game/SFX/GainPoint.GainPoint'"));
	return sfxAsset.Object;
}

USoundBase* UAssetsStorageManager::GetLostPointSfx() {
	ConstructorHelpers::FObjectFinder<USoundBase> sfxAsset(TEXT("SoundWave'/Game/SFX/LostPoint.LostPoint'"));
	return sfxAsset.Object;
}

USoundBase* UAssetsStorageManager::GetHitSfx() {
	ConstructorHelpers::FObjectFinder<USoundBase> sfxAsset(TEXT("SoundWave'/Game/SFX/Strike.Strike'"));
	return sfxAsset.Object;
}

USoundBase* UAssetsStorageManager::GetGameOverSfx() {
	ConstructorHelpers::FObjectFinder<USoundBase> sfxAsset(TEXT("SoundWave'/Game/SFX/GameOver.GameOver'"));
	return sfxAsset.Object;
}