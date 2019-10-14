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
class POOLPROJN_API AMyPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMyPawn();

	UPROPERTY(EditAnywhere, Category = MyPawn)
		float Mass = 1;
	UPROPERTY(EditAnywhere, Category = MyPawn)
		float ForceAmount = 10000;

private:

	/** The main skeletal mesh associated with this Character (optional sub-object). */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Mesh;

	/** Movement component used for movement logic in various movement modes (walking, falling, etc), containing relevant settings and functions to control movement. */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UMyPawnMovementComponent* PawnMovement;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void InitPlayerCameraManager();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	void Fire();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Returns Mesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetMesh() const { return Mesh; }

	/** Returns CharacterMovement subobject **/
	FORCEINLINE class UMyPawnMovementComponent* GetPawnMovement() const { return PawnMovement; }
};
