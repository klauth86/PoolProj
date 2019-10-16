// Fill out your copyright notice in the Description page of Project Settings.

#include "Hole.h"
#include "MyBall.h"
#include "Engine/GameEngine.h"

// Sets default values
AHole::AHole()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> meshAsset(TEXT("StaticMesh'/Game/Models/Sphere.Sphere'"));
	Mesh->SetStaticMesh(meshAsset.Object);
	static ConstructorHelpers::FObjectFinder<UMaterial> matAsset(TEXT("Material'/Game/Models/Hole_MAT.Hole_MAT'"));
	Mesh->SetMaterial(0, matAsset.Object);

	Mesh->BodyInstance.bUseCCD = true;
	Mesh->SetCollisionProfileName(TEXT("OverlapAll"));

	Mesh->SetRelativeScale3D(FVector(2, 2, 2));

	Mesh->SetMobility(EComponentMobility::Type::Static);

	this->OnActorBeginOverlap.AddDynamic(this, &AHole::CallOnActorBeginOverlap);
}

void AHole::CallOnActorBeginOverlap(AActor* actor, AActor* otherActor) {
	auto ball = Cast<AMyBall>(otherActor);
	if (ball) {
		ball->Destroy();
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Goal!!!"));
	}
}
