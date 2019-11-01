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
#include "Animation/AnimBlueprint.h"
#include "Engine/SkeletalMesh.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/ArrowComponent.h"
#include "Camera/CameraComponent.h"

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		APoolProjAGameModeBase* GameMode;

	int ControllerId;
protected:
	void MoveForward(float Val);

	void MoveRight(float Val);

	UFUNCTION(Server, Reliable, WithValidation)
	void Fire();
	void Fire_Implementation();
	bool Fire_Validate();

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

public:
	FORCEINLINE class UCustomStaticMeshComponent* GetCollisionComponent() const { return CollisionComponent; }

	FORCEINLINE class USkeletalMeshComponent* GetMeshComponent() const { return MeshComponent; }

	FORCEINLINE virtual UPawnMovementComponent* GetMovementComponent() const override { return MovementComponent; };

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE class UArrowComponent* GetArrowComponent() const { return ArrowComponent; }

	FORCEINLINE int GetControllerId() const { return ControllerId; }
};