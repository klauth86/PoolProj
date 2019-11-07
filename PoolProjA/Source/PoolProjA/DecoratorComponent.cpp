// Fill out your copyright notice in the Description page of Project Settings.


#include "DecoratorComponent.h"

void UDecoratorComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	auto world = GetWorld();
	SetRelativeLocation(Origin + Magnitude * OscDirection * FMath::Cos(AngularVelocity * world->TimeSeconds));
}

UDecoratorComponent::UDecoratorComponent() {
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.bCanEverTick = true;

	SetCollisionProfileName(TEXT("NoCollision"));
	bIsActive = true;
}

void UDecoratorComponent::SetActive(bool newIsActive) {

	if (bIsActive == newIsActive)
		return;

	bIsActive = newIsActive;
	SetHiddenInGame(!newIsActive);
	SetComponentTickEnabled(newIsActive);
}