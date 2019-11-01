// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyPawn.h"

TMap<int, AMyPawn*> AMyPawn::Instances = TMap<int, AMyPawn*>();

// Sets default values
AMyPawn::AMyPawn() {
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	NetPriority = 3.0f;

	bCollideWhenPlacing = false;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	RootComponent = CollisionComponent = CreateDefaultSubobject<UCustomStaticMeshComponent>(TEXT("CollisionComponent"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> meshAsset(TEXT("StaticMesh'/Game/Models/EyeBall/CollisionMesh.CollisionMesh'"));
	CollisionComponent->SetStaticMesh(meshAsset.Object);
	CollisionComponent->SetVisibility(false);

	SetupBodyInstance();

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
	CameraBoom->TargetArmLength = 30.0f; // The camera follows at this distance behind the character
	CameraBoom->SetRelativeRotation(FRotator(-30, 0, 0));
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bInheritPitch = false;
	CameraBoom->bDoCollisionTest = false;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	ArrowComponent->SetRelativeRotation(FRotator(30, 0, 0));
}

void AMyPawn::SetupBodyInstance() {
	CollisionComponent->SetMassOverrideInKg("", Mass, true);
	CollisionComponent->SetSimulatePhysics(true);
	CollisionComponent->SetNotifyRigidBodyCollision(true);

	CollisionComponent->BodyInstance.bUseCCD = true;
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAll"));
}

void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	auto controller = Cast<APlayerController>(GetController());
	ControllerId = UGameplayStatics::GetPlayerControllerID(controller);
	Instances.Add(ControllerId, this);

	auto gameMode = Cast<APoolProjAGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (gameMode)
		GameMode = gameMode;

	if (ControllerId == 0) {
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
	if (GameMode->ActiveControllerId == ControllerId &&
		Value != 0.0f && State == MyPawnState::ACTIVE) {		
		auto tolerance = 1e-6f;
		if (CollisionComponent->GetPhysicsAngularVelocityInDegrees().SizeSquared() > tolerance) {
			StopMovement();
		}
		MovementComponent->MoveForward(ArrowComponent->GetForwardVector() * Value);
	}
}

void AMyPawn::MoveRight(float Value) {
	if (GameMode->ActiveControllerId == ControllerId &&
		Value != 0.0f && State == MyPawnState::ACTIVE) {
		auto tolerance = 1e-6f;
		if (CollisionComponent->GetPhysicsAngularVelocityInDegrees().SizeSquared() > tolerance) {
			StopMovement();
		}

		AddControllerYawInput(Value);
		ServerSetYaw(GetControlRotation().Yaw);
	}
}

void AMyPawn::Fire_Implementation() {
	if (GameMode->ActiveControllerId == ControllerId) {
		if (State == MyPawnState::ACTIVE) {
			State = MyPawnState::LAUNCHED;
			CollisionComponent->AddForce(ForceAmount * ArrowComponent->GetForwardVector());
		}
		else if (State == MyPawnState::LAUNCHED) {
			State = MyPawnState::ACTIVE;
			StopMovement();

			auto next = (GameMode->ActiveControllerId + 1) % 2;
			Instances[next]->StopMovement();
			GameMode->ActiveControllerId = next;
		}
	}
}

void AMyPawn::StopMovement() 	{
	CollisionComponent->DestroyPhysicsState();
	SetYaw();
	CollisionComponent->BodyInstance = FBodyInstance();
	SetupBodyInstance();
	CollisionComponent->CreatePhysicsState();
}

bool AMyPawn::Fire_Validate() {
	return true;
}



void AMyPawn::DrawRay() {
	auto location = this->GetActorLocation();
	auto forward = ArrowComponent->GetForwardVector();

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
	auto rotation = FRotator::ZeroRotator;
	rotation.Yaw = Yaw;
	SetActorRotation(rotation);
}

void AMyPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyPawn, Yaw);
	DOREPLIFETIME(AMyPawn, State);
}