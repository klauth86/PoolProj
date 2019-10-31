// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomStaticMeshComponent.h"

void UCustomStaticMeshComponent::CreatePhysicsState() {
	Super::OnCreatePhysicsState();
};

void UCustomStaticMeshComponent::DestroyPhysicsState() {
	Super::OnDestroyPhysicsState();
};

