// Fill out your copyright notice in the Description page of Project Settings.


#include "DecoratorComponent.h"

void UDecoratorComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateDesiredArmLocation(DeltaTime);
}

void UDecoratorComponent::ApplyWorldOffset(const FVector & InOffset, bool bWorldShift) {
	Super::ApplyWorldOffset(InOffset, bWorldShift);
}

void UDecoratorComponent::OnRegister() {
	Super::OnRegister();
	UpdateDesiredArmLocation(0.f);
}

void UDecoratorComponent::UpdateDesiredArmLocation(float DeltaTime) {
	FVector position = GetComponentLocation() + TargetOffset;
	FRotator rotation = FRotator::ZeroRotator;
}