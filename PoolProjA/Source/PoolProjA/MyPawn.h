// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MyPawnMovementComponent.h"

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "UObject/ConstructorHelpers.h"
#include "Materials/Material.h"

#include "Components/PrimitiveComponent.h"

#include "MyPawn.generated.h"

UCLASS()
class POOLPROJA_API AMyPawn : public APawn {
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMyPawn();

	UPROPERTY(EditAnywhere, Category = MyPawn)
		float Mass = 1; // In Kg

	UPROPERTY(EditAnywhere, Category = MyPawn)
		float ForceAmount = 760000; // IN Newtons
	// https://billiards.colostate.edu/technical_proofs/new/TP_B-20.pdf

	UPROPERTY(VisibleAnywhere, Category = MyPawn)
		bool IsInFireMode = false; // In Kg

private:
	/** The main mesh associated with this Pawn (optional sub-object). */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* MeshComponent;

	/** Movement component used for movement logic in various movement modes (walking, falling, etc), containing relevant settings and functions to control movement. */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UMyPawnMovementComponent* MovementComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	void Fire();

	void InitPlayerCameraManager();

protected:
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	FORCEINLINE class UStaticMeshComponent* GetMeshComponent() const { return MeshComponent; }

	FORCEINLINE virtual UPawnMovementComponent* GetMovementComponent() const override { return MovementComponent; };
};