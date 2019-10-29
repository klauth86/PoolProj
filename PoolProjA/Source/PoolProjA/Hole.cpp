// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Hole.h"

// Sets default values
AHole::AHole() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> meshAsset(TEXT("StaticMesh'/Game/Models/Sphere.Sphere'"));
	Mesh->SetStaticMesh(meshAsset.Object);
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> matAsset(TEXT("Material'/Game/Models/BaseColorMAT_Hole.BaseColorMAT_Hole'"));
	Mesh->SetMaterial(0, matAsset.Object);

	Mesh->BodyInstance.bUseCCD = true;
	Mesh->SetCollisionProfileName(TEXT("OverlapAll"));

	Mesh->SetRelativeScale3D(FVector(2, 2, 2));

	Mesh->SetMobility(EComponentMobility::Type::Static);

	this->OnActorBeginOverlap.AddDynamic(this, &AHole::CallOnActorBeginOverlap);
}

void AHole::CallOnActorBeginOverlap(AActor* actor, AActor* otherActor) {
	auto ball = Cast<ABall>(otherActor);
	if (ball) {
		
		ball->Hit();

		auto world = GetWorld();
		if (world) {

			auto gameMode =
				UGameplayStatics::GetGameMode(world);
			auto myGameMode =
				Cast<APoolProjAGameModeBase>(gameMode);

			if (myGameMode != nullptr) {
					if (MyOwnerControllerId == 0) {
						myGameMode->Player2Score++;
					}
					else {
						myGameMode->Player1Score++;
					}

				myGameMode->CheckWinCondition();
			}
		}
	}
}