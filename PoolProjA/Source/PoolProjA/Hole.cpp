// Fill out your copyright notice in the Description page of Project Settings.

#include "Hole.h"

AHole::AHole() {
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionComponent"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> meshAsset(TEXT("StaticMesh'/Game/Models/EyeBall/CollisionMesh.CollisionMesh'"));
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
		if (auto myGameMode = APoolProjAGameModeBase::GetCurrentGameMode()) {
			if (MyOwnerControllerId == 0) {
				myGameMode->Player2Score++;
			}
			else {
				myGameMode->Player1Score++;
			}
		}
		ball->Destroy();
	}
}

void AHole::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
	this->OnActorBeginOverlap.RemoveAll(this);
}