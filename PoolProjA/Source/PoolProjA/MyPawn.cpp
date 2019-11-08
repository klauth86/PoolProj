// Fill out your copyright notice in the Description page of Project Settings.

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

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("MeshComponent");
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> smeshAsset(TEXT("SkeletalMesh'/Game/Models/EyeBall/EyeBall.EyeBall'"));
	MeshComponent->SetSkeletalMesh(smeshAsset.Object);
	MeshComponent->SetupAttachment(RootComponent);

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

	DecoratorComponent = CreateDefaultSubobject<UDecoratorComponent>(TEXT("DecoratorComponent"));
	DecoratorComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	auto tmp = 30.f * FMath::Cos(FMath::DegreesToRadians(30));
	auto origin = FVector(tmp * FMath::Cos(FMath::DegreesToRadians(30)), 0, tmp * FMath::Sin(FMath::DegreesToRadians(30)));

	DecoratorComponent->SetRelativeLocation(origin);
	DecoratorComponent->Origin = origin;
	DecoratorComponent->OscDirection = FVector(-FMath::Cos(FMath::DegreesToRadians(30)), 0, FMath::Sin(FMath::DegreesToRadians(30)));

	static ConstructorHelpers::FObjectFinder<USoundBase> sfxAsset(TEXT("SoundWave'/Game/SFX/LostPoint.LostPoint'"));
	FallSfx = sfxAsset.Object;
}



void AMyPawn::BeginPlay() {
	Super::BeginPlay();
	SetupBodyInstance();

	StartLocation = GetActorLocation();
	StartRotation = GetActorRotation();
}



void AMyPawn::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if (GetActorLocation().SizeSquared() > 90000) {
		if (auto myGameMode = APoolProjAGameModeBase::GetCurrentGameMode()) {
			if (Instances[0] == this) {
				myGameMode->Player1Score--;
			}
			else {
				myGameMode->Player2Score--;
			}

			SetActorLocation(StartLocation);
			StopMovement();
			UGameplayStatics::PlaySound2D(this, FallSfx);

			auto controller = Cast<APlayerController>(GetController());
			float calcValue = (StartRotation.Yaw - GetActorRotation().Yaw)/controller->InputYawScale;
			AddControllerYawInput(calcValue);
			ServerSetYaw(StartRotation.Yaw);
		}
	}
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
	auto controllerId = UGameplayStatics::GetPlayerControllerID(controller);
	Instances.Add(controllerId, this);
	DecoratorComponent->SetActive(controllerId == 0);


	PlayerInputComponent->BindAxis("MoveForward", this, &AMyPawn::CommonMoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyPawn::CommonMoveRight);
	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &AMyPawn::CommonStartFire);
	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &AMyPawn::CommonStopFire);
}



void AMyPawn::CommonMoveForward(float Value) {
	if (auto myGameMode = APoolProjAGameModeBase::GetCurrentGameMode()) {
		if (Instances.Contains(myGameMode->ActiveControllerId)) {
			auto pawn = Instances[myGameMode->ActiveControllerId];
			pawn->MoveForward(Value);
		}
	}
}

void AMyPawn::MoveForward(float Value) {
	if (Value != 0.0f && State == MyPawnState::ACTIVE) {		
		auto tolerance = 1e-6f;
		if (CollisionComponent->GetPhysicsAngularVelocityInDegrees().SizeSquared() > tolerance) {
			StopMovement();
		}
		MovementComponent->MoveForward(ArrowComponent->GetForwardVector() * Value);
	}
}



void AMyPawn::CommonMoveRight(float Value) {
	if (auto myGameMode = APoolProjAGameModeBase::GetCurrentGameMode()) {
		if (Instances.Contains(myGameMode->ActiveControllerId)) {
			auto pawn = Instances[myGameMode->ActiveControllerId];
			pawn->MoveRight(Value);
		}
	}
}

void AMyPawn::MoveRight(float Value) {
	if (Value != 0.0f && State == MyPawnState::ACTIVE) {
		auto tolerance = 1e-6f;
		if (CollisionComponent->GetPhysicsAngularVelocityInDegrees().SizeSquared() > tolerance) {
			StopMovement();
		}

		AddControllerYawInput(Value);
		ServerSetYaw(GetControlRotation().Yaw);
	}
}



void AMyPawn::CommonStartFire_Implementation() {
	if (auto myGameMode = APoolProjAGameModeBase::GetCurrentGameMode()) {
		if (Instances.Contains(myGameMode->ActiveControllerId)) {
			auto pawn = Instances[myGameMode->ActiveControllerId];
			pawn->StartFire();
		}
	}
}

bool AMyPawn::CommonStartFire_Validate() {
	return true;
}

void AMyPawn::StartFire() {
	if (State == MyPawnState::ACTIVE) {
		State = MyPawnState::LAUNCHED;
		CollisionComponent->AddForce(ForceAmount * ArrowComponent->GetForwardVector());
	}
}



void AMyPawn::CommonStopFire_Implementation() {
	if (auto myGameMode = APoolProjAGameModeBase::GetCurrentGameMode()) {
		if (Instances.Contains(myGameMode->ActiveControllerId)) {
			auto pawn = Instances[myGameMode->ActiveControllerId];
			pawn->StopFire();
		}
	}
}

bool AMyPawn::CommonStopFire_Validate() {
	return true;
}

void AMyPawn::StopFire() 	{
	if (State == MyPawnState::LAUNCHED) {
		
		auto myGameMode = APoolProjAGameModeBase::GetCurrentGameMode();

		State = MyPawnState::ACTIVE;
		StopMovement();
		DecoratorComponent->SetActive(false);
		UE_LOG(LogTemp, Warning, TEXT("%d"), myGameMode->ActiveControllerId);

		auto next = (myGameMode->ActiveControllerId + 1) % 2;
		
		Instances[next]->StopMovement();
		Instances[next]->DecoratorComponent->SetActive(true);
		UE_LOG(LogTemp, Warning, TEXT("%d"), next);

		myGameMode->ActiveControllerId = next;
	}
}



void AMyPawn::StopMovement() {
	CollisionComponent->DestroyPhysicsState();
	SetYaw();
	CollisionComponent->BodyInstance = FBodyInstance();
	SetupBodyInstance();
	CollisionComponent->CreatePhysicsState();
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