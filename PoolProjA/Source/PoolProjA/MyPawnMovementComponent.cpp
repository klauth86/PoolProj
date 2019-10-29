#pragma once

#include "MyPawnMovementComponent.h"

UMyPawnMovementComponent::UMyPawnMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {
	MaxSpeed = 1200.f;
	Acceleration = 4000.f;
	Deceleration = 8000.f;
	TurningBoost = 8.0f;
	bPositionCorrected = false;

	ResetMoveState();
}

void UMyPawnMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) {
	if (ShouldSkipUpdate(DeltaTime)) {
		return;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PawnOwner || !UpdatedComponent) {
		return;
	}

	if (ROLE_Authority == PawnOwner->Role) {
		
		auto movementInput = ConsumeMovementInput();

		ApplyControlInputToVelocity(DeltaTime, movementInput);
		
		if (IsExceedingMaxSpeed(MaxSpeed) == true) {
			Velocity = Velocity.GetUnsafeNormal() * MaxSpeed;
		}

		LimitWorldBounds();
		bPositionCorrected = false;

		FVector Delta = Velocity * DeltaTime;

		auto tolerance = 1e-6f;
		auto deltaIsNearlyZero = Delta.IsNearlyZero(tolerance);

		if (!deltaIsNearlyZero) {
			
			const FVector OldLocation = UpdatedComponent->GetComponentLocation();

			FHitResult Hit(1.f);
			SafeMoveUpdatedComponent(Delta, FQuat(UpdatedComponent->GetComponentQuat()), true, Hit);

			if (Hit.IsValidBlockingHit()) {
				HandleImpact(Hit, DeltaTime, Delta);
				// Try to slide the remaining distance along the surface.
				SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
			}

			// Update velocity
			// We don't want position changes to vastly reverse our direction (which can happen due to penetration fixups etc)
			if (!bPositionCorrected) {
				const FVector NewLocation = UpdatedComponent->GetComponentLocation();
				Velocity = ((NewLocation - OldLocation) / DeltaTime);
			}
		}

		// Finalize
		UpdateComponentVelocity();
	}
};

bool UMyPawnMovementComponent::LimitWorldBounds() {
	AWorldSettings* WorldSettings = PawnOwner ? PawnOwner->GetWorldSettings() : NULL;
	if (!WorldSettings || !WorldSettings->bEnableWorldBoundsChecks || !UpdatedComponent) {
		return false;
	}

	const FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
	if (CurrentLocation.Z < WorldSettings->KillZ) {
		Velocity.Z = FMath::Min(GetMaxSpeed(), WorldSettings->KillZ - CurrentLocation.Z + 2.0f);
		return true;
	}

	return false;
}

void UMyPawnMovementComponent::ApplyControlInputToVelocity(float DeltaTime, FVector input) {
	const FVector ControlAcceleration = input.GetClampedToMaxSize(1.f);

	const float AnalogInputModifier = (ControlAcceleration.SizeSquared() > 0.f ? ControlAcceleration.Size() : 0.f);
	const float MaxPawnSpeed = GetMaxSpeed() * AnalogInputModifier;
	const bool bExceedingMaxSpeed = IsExceedingMaxSpeed(MaxPawnSpeed);

	if (AnalogInputModifier > 0.f && !bExceedingMaxSpeed) {
		// Apply change in velocity direction
		if (Velocity.SizeSquared() > 0.f) {
			// Change direction faster than only using acceleration, but never increase velocity magnitude.
			const float TimeScale = FMath::Clamp(DeltaTime * TurningBoost, 0.f, 1.f);
			Velocity = Velocity + (ControlAcceleration * Velocity.Size() - Velocity) * TimeScale;
		}
	}
	else {
		// Dampen velocity magnitude based on deceleration.
		if (Velocity.SizeSquared() > 0.f) {
			const FVector OldVelocity = Velocity;
			const float VelSize = FMath::Max(Velocity.Size() - FMath::Abs(Deceleration) * DeltaTime, 0.f);
			Velocity = Velocity.GetSafeNormal() * VelSize;

			// Don't allow braking to lower us below max speed if we started above it.
			if (bExceedingMaxSpeed && Velocity.SizeSquared() < FMath::Square(MaxPawnSpeed)) {
				Velocity = OldVelocity.GetSafeNormal() * MaxPawnSpeed;
			}
		}
	}

	// Apply acceleration and clamp velocity magnitude.
	const float NewMaxSpeed = (IsExceedingMaxSpeed(MaxPawnSpeed)) ? Velocity.Size() : MaxPawnSpeed;
	Velocity += ControlAcceleration * FMath::Abs(Acceleration) * DeltaTime;
	Velocity = Velocity.GetClampedToMaxSize(NewMaxSpeed);

	ConsumeInputVector();
}

bool UMyPawnMovementComponent::ResolvePenetrationImpl(const FVector& Adjustment, const FHitResult& Hit, const FQuat& NewRotationQuat) {
	bPositionCorrected |= Super::ResolvePenetrationImpl(Adjustment, Hit, NewRotationQuat);
	return bPositionCorrected;
}


// MOVE FORWARD
void UMyPawnMovementComponent::MoveForward(FVector val) {
	ServerMoveForward(val);
}

void UMyPawnMovementComponent::ServerMoveForward_Implementation(FVector val) {
	MovementInput += val;
}

bool UMyPawnMovementComponent::ServerMoveForward_Validate(FVector val) {
	return true;
}

FVector UMyPawnMovementComponent::ConsumeMovementInput() {
	auto result = MovementInput;
	MovementInput = FVector::ZeroVector;
	return result;
}
