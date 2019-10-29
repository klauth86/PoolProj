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

	RootComponent = CollisionComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionComponent"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> meshAsset(TEXT("StaticMesh'/Game/Models/EyeBall/CollisionMesh.CollisionMesh'"));
	CollisionComponent->SetStaticMesh(meshAsset.Object);
	CollisionComponent->SetVisibility(false);

	CollisionComponent->SetMassOverrideInKg("", Mass, true);
	CollisionComponent->SetSimulatePhysics(true);
	CollisionComponent->SetNotifyRigidBodyCollision(true);

	CollisionComponent->BodyInstance.bUseCCD = true;
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAll"));

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("MeshComponent");
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> smeshAsset(TEXT("SkeletalMesh'/Game/Models/EyeBall/EyeBall.EyeBall'"));
	MeshComponent->SetSkeletalMesh(smeshAsset.Object);
	MeshComponent->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UAnimBlueprint> smeshAnimBP(TEXT("AnimBlueprint'/Game/Models/EyeBall/EyeBall_Anim_BP.EyeBall_Anim_BP'"));
	MeshComponent->SetAnimInstanceClass(smeshAnimBP.Object->GeneratedClass);

	MovementComponent = CreateDefaultSubobject<UMyPawnMovementComponent>("MovementComponent");

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 50.0f; // The camera follows at this distance behind the character
	CameraBoom->SetRelativeRotation(FRotator(-10, 0, 0));
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bInheritPitch = false;
	CameraBoom->bDoCollisionTest = false;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	auto controller = Cast<APlayerController>(GetController());
	auto id = UGameplayStatics::GetPlayerControllerID(controller);
	UE_LOG(LogTemp, Warning, TEXT("%d ID"), id);
	if (id == 0) {
		PlayerInputComponent->BindAxis("MoveForward", this, &AMyPawn::MoveForward);
		PlayerInputComponent->BindAxis("MoveRight", this, &AMyPawn::MoveRight);

		PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &AMyPawn::Fire);
	}
	else {
		PlayerInputComponent->BindAxis("MoveForward2", this, &AMyPawn::MoveForward);
		PlayerInputComponent->BindAxis("MoveRight2", this, &AMyPawn::MoveRight);

		PlayerInputComponent->BindAction("Fire2", EInputEvent::IE_Pressed, this, &AMyPawn::Fire);
	}
}

void AMyPawn::MoveForward(float Value) {
	if (Value != 0.0f && State == MyPawnState::ACTIVE) {
		MovementComponent->MoveForward(GetActorForwardVector() * Value);
	}
}

void AMyPawn::MoveRight(float Value) {
	if (Value != 0.0f && State == MyPawnState::ACTIVE) {
		AddControllerYawInput(Value);
		ServerSetYaw(GetControlRotation().Yaw);
	}
}

void AMyPawn::Fire_Implementation() {
	if (State == MyPawnState::ACTIVE) {
		State = MyPawnState::LAUNCHED;
		CollisionComponent->AddForce(ForceAmount * GetActorForwardVector());
	}
	else if (State == MyPawnState::LAUNCHED) {
		State = MyPawnState::DAMPING;
		StartDamping();
	}
}

bool AMyPawn::Fire_Validate() {
	return true;
}



void AMyPawn::DrawRay() {
	auto location = this->GetActorLocation();
	auto forward = this->GetActorForwardVector();

	DrawDebugLine(GetWorld(), location, location + forward * Sight,
		FColor(255, 0, 0), false, 0.5f, 0.f, 3.f);
}



void AMyPawn::ServerSetYaw_Implementation(float value) {
	Yaw = value;
	SetYaw();
}

bool AMyPawn::ServerSetYaw_Validate(float value) {
	return true;
}

void AMyPawn::OnRep_SetYaw() {
	SetYaw();
}

void AMyPawn::SetYaw() {
	auto rotation = ResetRotation ? FRotator::ZeroRotator : GetActorRotation();
	rotation.Yaw = Yaw;
	SetActorRotation(rotation);
}

void AMyPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyPawn, Yaw);
	DOREPLIFETIME(AMyPawn, ResetRotation);
	DOREPLIFETIME(AMyPawn, State);
}

void AMyPawn::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	auto tolerance = 1e-6f;

	if (Role == ROLE_Authority) {
		if (State == MyPawnState::DAMPING && FMath::Abs(CollisionComponent->GetPhysicsLinearVelocity().SizeSquared()) < tolerance) {
			State = MyPawnState::ACTIVE;
			StopDamping();
		}
	}
}

void AMyPawn::StartDamping() {
	auto body = CollisionComponent->BodyInstance;
	body.LinearDamping *= 1000000;
	body.AngularDamping *= 1000000;
	body.UpdateDampingProperties();
}

void AMyPawn::StopDamping() {
	CollisionComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
	CollisionComponent->SetPhysicsAngularVelocity(FVector::ZeroVector);
	
	auto body = CollisionComponent->BodyInstance;
	body.LinearDamping /= 1000000;
	body.AngularDamping /= 1000000;
	body.UpdateDampingProperties();

	UE_LOG(LogTemp, Warning, TEXT("StopDamping"));

	ResetRotation = true;
	Yaw = Yaw;
	SetYaw();
}

void AMyPawn::UpdateStats() {

}
