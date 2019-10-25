#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameFramework/PawnMovementComponent.h"
#include "MyPawnMovementComponent.generated.h"

/**
 * FloatingPawnMovement is a movement component that provides simple movement for any Pawn class.
 * Limits on speed and acceleration are provided, while gravity is not implemented.
 *
 * Normally the root component of the owning actor is moved, however another component may be selected (see SetUpdatedComponent()).
 * During swept (non-teleporting) movement only collision of UpdatedComponent is considered, attached components will teleport to the end location ignoring collision.
 */
UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class UMyPawnMovementComponent : public UPawnMovementComponent {
	GENERATED_BODY()

public:

	UMyPawnMovementComponent(const FObjectInitializer& ObjectInitializer);

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual float GetMaxSpeed() const override { return MaxSpeed; }

protected:

	virtual bool ResolvePenetrationImpl(const FVector& Adjustment, const FHitResult& Hit, const FQuat& NewRotation) override;

	FVector MovementInput;
	FVector ConsumeMovementInput();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMoveForward(FVector val);
	void ServerMoveForward_Implementation(FVector val);
	bool ServerMoveForward_Validate(FVector val);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FloatingPawnMovement)
		float MaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FloatingPawnMovement)
		float Acceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FloatingPawnMovement)
		float Deceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FloatingPawnMovement, meta = (ClampMin = "0", UIMin = "0"))
		float TurningBoost;

	void MoveForward(FVector val);

protected:

	virtual void ApplyControlInputToVelocity(float DeltaTime, FVector input);

	virtual bool LimitWorldBounds();

	UPROPERTY(Transient)
		uint32 bPositionCorrected : 1;
};
