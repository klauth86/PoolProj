// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyPawn.h"

// Sets default values
AMyPawn::AMyPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = true;

	RootComponent = Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> meshAsset(TEXT("StaticMesh'/Game/Models/Cube.Cube'"));
	Mesh->SetStaticMesh(meshAsset.Object);
	static ConstructorHelpers::FObjectFinder<UMaterial> matAsset(TEXT("Material'/Game/Models/Player_MAT.Player_MAT'"));
	Mesh->SetMaterial(0, matAsset.Object);

	Mesh->SetMassOverrideInKg("", Mass, true);
	Mesh->SetSimulatePhysics(true);
	Mesh->SetNotifyRigidBodyCollision(true);

	PawnMovement = CreateDefaultSubobject<UMyPawnMovementComponent>("CharacterMovement");

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(RootComponent);
	PlaceCamera();
}

// Called when the game starts or when spawned
void AMyPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PlaceCamera();
}

// Called to bind functionality to input
void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
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

void AMyPawn::PlaceCamera() {
	if (FollowCamera) {
		FollowCamera->RelativeLocation = FVector(-600, 0, 300);
		FollowCamera->RelativeRotation = FRotator(-30, 0, 0);
	}
}

void AMyPawn::Fire() {
	Mesh->AddForce(ForceAmount * GetActorForwardVector());
}
