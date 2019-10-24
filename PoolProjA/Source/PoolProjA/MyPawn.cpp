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
	static ConstructorHelpers::FObjectFinder<UStaticMesh> meshAsset(TEXT("StaticMesh'/Game/Models/Cube.Cube'"));
	MeshComponent->SetStaticMesh(meshAsset.Object);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> matAsset(TEXT("Material'/Game/Models/BaseColorMAT_Player.BaseColorMAT_Player'"));
	MeshComponent->SetMaterial(0, matAsset.Object);

	MeshComponent->SetMassOverrideInKg("", Mass, true);
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetNotifyRigidBodyCollision(true);

	MeshComponent->BodyInstance.bUseCCD = true;
	MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));

	MovementComponent = CreateDefaultSubobject<UMyPawnMovementComponent>("MovementComponent");

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 800.0f; // The camera follows at this distance behind the character
	CameraBoom->SetRelativeRotation(FRotator(-30, 0, 0));
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bInheritPitch = false;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyPawn::MoveRight);

	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &AMyPawn::Fire);
}

void AMyPawn::MoveForward(float Value) {
	if (Value != 0.0f) {
		MovementComponent->MoveForward(GetActorForwardVector() * Value);
	}
}

void AMyPawn::MoveRight(float Value) {

	static auto pc = Cast<APlayerController>(GetController());

	if (Value != 0.0f) {
		MovementComponent->MoveRight(Value * pc->InputYawScale);
		AddControllerYawInput(Value);
	}
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

void AMyPawn::DrawRay() {
	auto location = this->GetActorLocation();
	auto forward = this->GetActorForwardVector();

	DrawDebugLine(GetWorld(), location, location + forward * Sight,
		FColor(255, 0, 0), false, 0.5f, 0.f, 3.f);
}
