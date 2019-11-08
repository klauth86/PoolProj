// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/Material.h"
#include "Kismet/GameplayStatics.h"

#include "Engine/SkeletalMesh.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/ArrowComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"

#include "DecoratorComponent.h"
#include "CustomStaticMeshComponent.h"
#include "MyPawnMovementComponent.h"
#include "PoolProjAGameModeBase.h"

#include "MyPawn.generated.h"

UENUM(BlueprintType)
enum class MyPawnState : uint8 {
	ACTIVE UMETA(DisplayName = "ACTIVE"),
	LAUNCHED UMETA(DisplayName = "LAUNCHED"),
	PENDING UMETA(DisplayName = "PENDING")
};

UCLASS()
class POOLPROJA_API AMyPawn : public APawn {
	GENERATED_BODY()

public:
	AMyPawn();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = MyPawn)
		float Sight = 5000; // In Sm

	UPROPERTY(EditAnywhere, Category = MyPawn)
		float Mass = 1; // In Kg

	UPROPERTY(EditAnywhere, Category = MyPawn)
		float ForceAmount = 760000; // IN Newtons
	// https://billiards.colostate.edu/technical_proofs/new/TP_B-20.pdf

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
		MyPawnState State = MyPawnState::ACTIVE;
private:
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCustomStaticMeshComponent* CollisionComponent;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* MeshComponent;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UMyPawnMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UArrowComponent* ArrowComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MyPawn, meta = (AllowPrivateAccess = "true"))
		class UDecoratorComponent* DecoratorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MyPawn, meta = (AllowPrivateAccess = "true"))
		FVector StartLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MyPawn, meta = (AllowPrivateAccess = "true"))
		FRotator StartRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USoundBase* FallSfx;
protected:
	void CommonMoveForward(float Val);
	void MoveForward(float Val);

	void CommonMoveRight(float Val);
	void MoveRight(float Val);

	UFUNCTION(Server, Reliable, WithValidation)
	void CommonStopFire();
	void CommonStopFire_Implementation();
	bool CommonStopFire_Validate();
	void StopFire();

	UFUNCTION(Server, Reliable, WithValidation)
	void CommonStartFire();
	void CommonStartFire_Implementation();
	bool CommonStartFire_Validate();
	void StartFire();

	void DrawRay();

	void SetupBodyInstance();

	static TMap<int, AMyPawn*> Instances;

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(ReplicatedUsing=OnRep_SetYaw)
	float Yaw;

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerSetYaw(float value);
	void ServerSetYaw_Implementation(float value);
	bool ServerSetYaw_Validate(float value);

	void SetYaw();

	UFUNCTION()
	void OnRep_SetYaw();

	void StopMovement();

	void BeginPlay() override;
public:
	FORCEINLINE class UCustomStaticMeshComponent* GetCollisionComponent() const { return CollisionComponent; }

	FORCEINLINE class USkeletalMeshComponent* GetMeshComponent() const { return MeshComponent; }

	FORCEINLINE virtual UPawnMovementComponent* GetMovementComponent() const override { return MovementComponent; };

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE class UArrowComponent* GetArrowComponent() const { return ArrowComponent; }

	FORCEINLINE class UDecoratorComponent* GetDecoratorComponent() const { return DecoratorComponent; }
};