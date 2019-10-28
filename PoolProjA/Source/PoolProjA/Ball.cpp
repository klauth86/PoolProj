// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Ball.h"

int ABall::instanceCount = 0;

// Sets default values
ABall::ABall() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bReplicateMovement = true;
	NetPriority = 2.0f;

	RootComponent = Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> meshAsset(TEXT("StaticMesh'/Game/Models/Sphere.Sphere'"));
	Mesh->SetStaticMesh(meshAsset.Object);
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> matAsset(TEXT("Material'/Game/Models/BaseColorMAT_Ball.BaseColorMAT_Ball'"));
	Mesh->SetMaterial(0, matAsset.Object);

	Mesh->SetMassOverrideInKg("", Mass, true);
	Mesh->SetSimulatePhysics(true);
	Mesh->SetNotifyRigidBodyCollision(true);

	Mesh->BodyInstance.bUseCCD = true;
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));

	instanceCount++;
}

void ABall::Hit() {
	instanceCount--;
	Destroy();
}