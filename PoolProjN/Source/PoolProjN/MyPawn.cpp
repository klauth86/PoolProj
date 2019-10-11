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

	Mesh->SetSimulatePhysics(true);
	Mesh->SetNotifyRigidBodyCollision(true);


	PawnMovement = CreateDefaultSubobject<UMyPawnMovementComponent>("CharacterMovement");
	if (PawnMovement) {
		PawnMovement->UpdatedComponent = Mesh;
	}

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character
	CameraBoom->TargetOffset = FVector(0, 0, 150);
	CameraBoom->bUsePawnControlRotation = false; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->RelativeRotation = FRotator(-30, 0, 0);
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	if (ArrowComponent) {
		ArrowComponent->ArrowColor = FColor(150, 200, 255);
		ArrowComponent->bTreatAsASprite = true;
		ArrowComponent->bIsScreenSizeScaled = true;
	}
#endif // WITH_EDITORONLY_DATA
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

}

// Called to bind functionality to input
void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APawn::AddControllerYawInput);
}

void AMyPawn::MoveForward(float Value) {
	if (Value != 0.0f) {
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMyPawn::FaceRotation(FRotator NewControlRotation, float DeltaTime) {
	// Only if we actually are going to use any component of rotation.
	if (bUseControllerRotationPitch || bUseControllerRotationYaw || bUseControllerRotationRoll) {
		const FRotator CurrentRotation = GetActorRotation();

		if (!bUseControllerRotationPitch) {
			NewControlRotation.Pitch = CurrentRotation.Pitch;
		}

		if (!bUseControllerRotationYaw) {
			NewControlRotation.Yaw = CurrentRotation.Yaw;
		}

		if (!bUseControllerRotationRoll) {
			NewControlRotation.Roll = CurrentRotation.Roll;
		}

#if ENABLE_NAN_DIAGNOSTIC
		if (NewControlRotation.ContainsNaN()) {
			logOrEnsureNanError(TEXT("APawn::FaceRotation about to apply NaN-containing rotation to actor! New:(%s), Current:(%s)"), *NewControlRotation.ToString(), *CurrentRotation.ToString());
		}
#endif

#if ENABLE_NAN_DIAGNOSTIC
		if (NewRotation.ContainsNaN()) {
			logOrEnsureNanError(TEXT("AActor::SetActorRotation found NaN in FRotator NewRotation"));
			NewRotation = FRotator::ZeroRotator;
		}
#endif
		if (RootComponent) {
			RootComponent->MoveComponent(FVector::ZeroVector, NewControlRotation, true, nullptr, MOVECOMP_NoFlags, ETeleportType::None);
		}
	}
}
