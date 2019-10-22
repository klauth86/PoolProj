// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyPawn.h"

// Sets default values
AMyPawn::AMyPawn() {
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	NetPriority = 3.0f;

	bCollideWhenPlacing = false;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	bUseControllerRotationYaw = true;

	RootComponent = MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> meshAsset(TEXT("StaticMesh'/Game/Models/Sphere.Sphere'"));
	MeshComponent->SetStaticMesh(meshAsset.Object);
	static ConstructorHelpers::FObjectFinder<UMaterial> matAsset(TEXT("Material'/Game/Models/BaseColorMAT_Player.BaseColorMAT_Player'"));
	MeshComponent->SetMaterial(0, matAsset.Object);

	MeshComponent->SetMassOverrideInKg("", Mass, true);
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetNotifyRigidBodyCollision(true);

	MeshComponent->BodyInstance.bUseCCD = true;
	MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
	MeshComponent->AlwaysLoadOnClient = true;

	MovementComponent = CreateDefaultSubobject<UMyPawnMovementComponent>("MovementComponent");
}

// Called when the game starts or when spawned
void AMyPawn::BeginPlay() {
	Super::BeginPlay();
	InitPlayerCameraManager();
}

// Called to bind functionality to input
void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APawn::AddControllerYawInput);

	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &AMyPawn::Fire);
}

void AMyPawn::MoveForward(float Value) {
	if (Value != 0.0f) {
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMyPawn::InitPlayerCameraManager() {
	//static const FName NAME_FreeCam_Default = FName(TEXT("FreeCam_Default"));

	//APlayerController* const PC = CastChecked<APlayerController>(Controller);
	//PC->PlayerCameraManager->CameraStyle = NAME_FreeCam_Default;
	//PC->PlayerCameraManager->FreeCamOffset = FVector(0, 0, 300);
	//PC->PlayerCameraManager->FreeCamDistance = 600;

	//auto cache = PC->PlayerCameraManager->CameraCache;
	//cache.POV.Rotation = FRotator(0, -30, 0);
	//PC->PlayerCameraManager->CameraCache = cache;
}

void AMyPawn::Fire() {
	if (!IsInFireMode) {
		MeshComponent->AddForce(ForceAmount * GetActorForwardVector());
		IsInFireMode = true;
	}
	else {
		MeshComponent->SetMobility(EComponentMobility::Type::Static);
		IsInFireMode = false;

		MeshComponent->SetMobility(EComponentMobility::Type::Movable);
		SetActorRotation(FRotator::ZeroRotator);
	}
}
