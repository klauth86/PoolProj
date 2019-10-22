// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyBall.h"

// Sets default values
AMyBall::AMyBall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> meshAsset(TEXT("StaticMesh'/Game/Models/Sphere.Sphere'"));
	Mesh->SetStaticMesh(meshAsset.Object);
	static ConstructorHelpers::FObjectFinder<UMaterial> matAsset(TEXT("Material'/Game/Models/Player_MAT.Player_MAT'"));
	Mesh->SetMaterial(0, matAsset.Object);

	Mesh->SetMassOverrideInKg("", Mass, true);
	Mesh->SetSimulatePhysics(true);
	Mesh->SetNotifyRigidBodyCollision(true);

	Mesh->BodyInstance.bUseCCD = true;
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));
}

