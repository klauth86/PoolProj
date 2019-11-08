// Fill out your copyright notice in the Description page of Project Settings.

#include "Ball.h"
#include "MyPawn.h"

int ABall::instanceCount = 0;

// Sets default values
ABall::ABall() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bReplicateMovement = true;
	NetPriority = 2.0f;

	RootComponent = Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionComponent"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> meshAsset(TEXT("StaticMesh'/Game/Models/EyeBall/CollisionMesh.CollisionMesh'"));
	Mesh->SetStaticMesh(meshAsset.Object);
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> matAsset(TEXT("Material'/Game/Models/BaseColorMAT_Ball.BaseColorMAT_Ball'"));
	Mesh->SetMaterial(0, matAsset.Object);

	Mesh->SetSimulatePhysics(true);
	Mesh->SetNotifyRigidBodyCollision(true);

	Mesh->BodyInstance.bUseCCD = true;
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));

	static ConstructorHelpers::FObjectFinder<USoundBase> sfxAsset(TEXT("SoundWave'/Game/SFX/Strike.Strike'"));
	StrikeSfx = sfxAsset.Object;

	UE_LOG(LogTemp, Warning, TEXT("Ctor"));
}

void ABall::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
	instanceCount--;

	if (auto myGameMode = APoolProjAGameModeBase::GetCurrentGameMode()) {
		myGameMode->CheckWinCondition(instanceCount);
	}

	Mesh->OnComponentHit.RemoveDynamic(this, &ABall::OnCompHit);
	
	HitActors.Reset();

	UE_LOG(LogTemp, Warning, TEXT("~ABall %d"), instanceCount);
}

void ABall::BeginPlay() {

	UE_LOG(LogTemp, Warning, TEXT("BeginPlay"));

	Super::BeginPlay();

	Mesh->SetMassOverrideInKg("", Mass, true);
	Mesh->OnComponentHit.AddDynamic(this, &ABall::OnCompHit);
}

void ABall::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if (GetActorLocation().SizeSquared() > 90000) {
		Destroy();
		UE_LOG(LogTemp, Warning, TEXT("TickDestroy"));
	}

	TArray<AActor*> endedHits;
	for (auto item : HitActors) {
		auto distance = (item.Key->GetActorLocation() - GetActorLocation()).SizeSquared();
		if (distance > item.Value + 0.01) {
			endedHits.Add(item.Key);
		}
	}

	for (auto actor : endedHits) {
		HitActors.Remove(actor);
	}
}

void ABall::OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	auto pawn = Cast<AMyPawn>(OtherActor);
	if (pawn && !HitActors.Contains(OtherActor)) {
		UGameplayStatics::PlaySoundAtLocation(this, StrikeSfx, (GetActorLocation() + OtherActor->GetActorLocation())/2);
		HitActors.Add(OtherActor, (OtherActor->GetActorLocation() - GetActorLocation()).SizeSquared());
		return;
	}

	auto ball = Cast<ABall>(OtherActor);
	if (ball && !HitActors.Contains(OtherActor) && ball->Mesh->GetPhysicsLinearVelocity().SizeSquared() > Mesh->GetPhysicsLinearVelocity().SizeSquared()) {
		UGameplayStatics::PlaySoundAtLocation(this, StrikeSfx, (GetActorLocation() + OtherActor->GetActorLocation()) / 2);
		HitActors.Add(OtherActor, (OtherActor->GetActorLocation() - GetActorLocation()).SizeSquared());
		return;
	}
}
