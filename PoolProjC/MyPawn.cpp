// Fill out your copyright notice in the Description page of Project Settueings.

#pragma once

#include "MyPawn.h"
#include "GameFramework/DamageType.h"
#include "Components/SkinnedMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Engine/CollisionProfile.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "DisplayDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Animation/AnimInstance.h"

DECLARE_CYCLE_STAT(TEXT("Char OnNetUpdateSimulatedPosition"), STAT_CharacterOnNetUpdateSimulatedPosition, STATGROUP_Character);

// Sets default values
AMyPawn::AMyPawn(const FObjectInitializer& ObjectInitializer)
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AMyPawn::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME_CONDITION(AMyPawn, RepRootMotion, COND_SimulatedOnlyNoReplay);
	DOREPLIFETIME_CONDITION(AMyPawn, ReplicatedBasedMovement, COND_SimulatedOnly);
	//DOREPLIFETIME_CONDITION(AMyPawn, ReplicatedServerLastTransformUpdateTimeStamp, COND_SimulatedOnlyNoReplay);
	DOREPLIFETIME_CONDITION(AMyPawn, ReplicatedMovementMode, COND_SimulatedOnly);
	//DOREPLIFETIME_CONDITION(AMyPawn, AnimRootMotionTranslationScale, COND_SimulatedOnly);

	//// Change the condition of the replicated movement property to not replicate in replays since we handle this specifically via saving this out in external replay data
	//DOREPLIFETIME_CHANGE_CONDITION(AActor, ReplicatedMovement, COND_SimulatedOrPhysicsNoReplay);
}

#pragma region AACTOR INTERFACE

void AMyPawn::ClearCrossLevelReferences() {
	if (BasedMovement.MovementBase != nullptr && GetOutermost() != BasedMovement.MovementBase->GetOutermost()) {
		SetBase(nullptr);
	}

	Super::ClearCrossLevelReferences();
}

//
// Static variables for networking.
//
static uint8 SavedMovementMode;

void AMyPawn::PreNetReceive() {
	SavedMovementMode = ReplicatedMovementMode;
	Super::PreNetReceive();
}

void AMyPawn::PostNetReceive() {
	if (Role == ROLE_SimulatedProxy) {
		CharacterMovement->bNetworkUpdateReceived = true;
		CharacterMovement->bNetworkMovementModeChanged = (CharacterMovement->bNetworkMovementModeChanged || (SavedMovementMode != ReplicatedMovementMode));
	}

	Super::PostNetReceive();
}

void AMyPawn::OnRep_ReplicatedMovement() {
	// Skip standard position correction if we are playing root motion, OnRep_RootMotion will handle it.
	if (!IsPlayingNetworkedRootMotionMontage()) // animation root motion
	{
		if (!CharacterMovement || !CharacterMovement->CurrentRootMotion.HasActiveRootMotionSources()) // root motion sources
		{
			Super::OnRep_ReplicatedMovement();
		}
	}
}

void AMyPawn::PostNetReceiveLocationAndRotation() {
	if (Role == ROLE_SimulatedProxy) {
		// Don't change transform if using relative position (it should be nearly the same anyway, or base may be slightly out of sync)
		if (!ReplicatedBasedMovement.HasRelativeLocation()) {
			const FVector OldLocation = GetActorLocation();
			const FVector NewLocation = FRepMovement::RebaseOntoLocalOrigin(ReplicatedMovement.Location, this);
			const FQuat OldRotation = GetActorQuat();

			CharacterMovement->bNetworkSmoothingComplete = false;
			CharacterMovement->SmoothCorrection(OldLocation, OldRotation, NewLocation, ReplicatedMovement.Rotation.Quaternion());
			OnUpdateSimulatedPosition(OldLocation, OldRotation);
		}
	}
}

void AMyPawn::GetSimpleCollisionCylinder(float& CollisionRadius, float& CollisionHalfHeight) const {
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (IsTemplate()) {
		//UE_LOG(LogCharacter, Log, TEXT("WARNING AMyPawn::GetSimpleCollisionCylinder : Called on default object '%s'. Will likely return zero size. Consider using GetDefaultHalfHeight() instead."), *this->GetPathName());
	}
#endif

	if (RootComponent == CapsuleComponent && IsRootComponentCollisionRegistered()) {
		// Note: we purposefully ignore the component transform here aside from scale, always treating it as vertically aligned.
		// This improves performance and is also how we stated the CapsuleComponent would be used.
		CapsuleComponent->GetScaledCapsuleSize(CollisionRadius, CollisionHalfHeight);
	}
	else {
		Super::GetSimpleCollisionCylinder(CollisionRadius, CollisionHalfHeight);
	}
}

UActorComponent* AMyPawn::FindComponentByClass(const TSubclassOf<UActorComponent> ComponentClass) const {
	// If the character has a Mesh, treat it as the first 'hit' when finding components
	if (Mesh && ComponentClass && Mesh->IsA(ComponentClass)) {
		return Mesh;
	}

	return Super::FindComponentByClass(ComponentClass);
}

void AMyPawn::TornOff() {
	Super::TornOff();

	if (CharacterMovement) {
		CharacterMovement->ResetPredictionData_Client();
		CharacterMovement->ResetPredictionData_Server();
	}

	// We're no longer controlled remotely, resume regular ticking of animations.
	if (Mesh) {
		Mesh->bOnlyAllowAutonomousTickPose = false;
	}
}
#pragma endregion

FVector AMyPawn::GetNavAgentLocation() const {
	FVector AgentLocation = FNavigationSystem::InvalidLocation;

	if (GetCharacterMovement() != nullptr) {
		AgentLocation = GetCharacterMovement()->GetActorFeetLocation();
	}

	if (FNavigationSystem::IsValidLocation(AgentLocation) == false && CapsuleComponent != nullptr) {
		AgentLocation = GetActorLocation() - FVector(0, 0, CapsuleComponent->GetScaledCapsuleHalfHeight());
	}

	return AgentLocation;
}

#pragma region APAWN INTERFACE

void AMyPawn::PostInitializeComponents() {
	Super::PostInitializeComponents();

	if (!IsPendingKill()) {
		if (Mesh) {
			CacheInitialMeshOffset(Mesh->RelativeLocation, Mesh->RelativeRotation);

			// force animation tick after movement component updates
			if (Mesh->PrimaryComponentTick.bCanEverTick && CharacterMovement) {
				Mesh->PrimaryComponentTick.AddPrerequisite(CharacterMovement, CharacterMovement->PrimaryComponentTick);
			}
		}

		if (CharacterMovement && CapsuleComponent) {
			CharacterMovement->UpdateNavAgent(*CapsuleComponent);
		}

		if (Controller == nullptr && GetNetMode() != NM_Client) {
			if (CharacterMovement && CharacterMovement->bRunPhysicsWithNoController) {
				CharacterMovement->SetDefaultMovementMode();
			}
		}
	}
}

UPawnMovementComponent* AMyPawn::GetMovementComponent() const {
	return CharacterMovement;
}

float AMyPawn::GetDefaultHalfHeight() const {
	UCapsuleComponent* DefaultCapsule = GetClass()->GetDefaultObject<AMyPawn>()->CapsuleComponent;
	if (DefaultCapsule) {
		return DefaultCapsule->GetScaledCapsuleHalfHeight();
	}
	else {
		return Super::GetDefaultHalfHeight();
	}
}

void AMyPawn::TurnOff() {
	if (CharacterMovement != nullptr) {
		CharacterMovement->StopMovementImmediately();
		CharacterMovement->DisableMovement();
	}

	if (GetNetMode() != NM_DedicatedServer && Mesh != nullptr) {
		Mesh->bPauseAnims = true;
		if (Mesh->IsSimulatingPhysics()) {
			Mesh->bBlendPhysics = true;
			Mesh->KinematicBonesUpdateType = EKinematicBonesUpdateToPhysics::SkipAllBones;
		}
	}

	Super::TurnOff();
}

void AMyPawn::Restart() {
	Super::Restart();

	if (CharacterMovement) {
		CharacterMovement->SetDefaultMovementMode();
	}
}

void AMyPawn::PawnClientRestart() {
	if (CharacterMovement != nullptr) {
		CharacterMovement->StopMovementImmediately();
		CharacterMovement->ResetPredictionData_Client();
	}

	Super::PawnClientRestart();
}

void AMyPawn::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);

	// If we are controlled remotely, set animation timing to be driven by client's network updates. So timing and events remain in sync.
	if (Mesh && bReplicateMovement && (GetRemoteRole() == ROLE_AutonomousProxy && GetNetConnection() != nullptr)) {
		Mesh->bOnlyAllowAutonomousTickPose = true;
	}
}

void AMyPawn::UnPossessed() {
	Super::UnPossessed();

	if (CharacterMovement) {
		CharacterMovement->ResetPredictionData_Client();
		CharacterMovement->ResetPredictionData_Server();
	}

	// We're no longer controlled remotely, resume regular ticking of animations.
	if (Mesh) {
		Mesh->bOnlyAllowAutonomousTickPose = false;
	}
}

void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	check(PlayerInputComponent);
}

void AMyPawn::UpdateNavigationRelevance() {
	if (CapsuleComponent) {
		CapsuleComponent->SetCanEverAffectNavigation(bCanAffectNavigationGeneration);
	}
}
#pragma endregion

/**	Change the Pawn's base. */
void AMyPawn::SetBase( UPrimitiveComponent* NewBaseComponent, const FName InBoneName, bool bNotifyPawn )
{
	// If NewBaseComponent is nullptr, ignore bone name.
	const FName BoneName = (NewBaseComponent ? InBoneName : NAME_None);

	// See what changed.
	const bool bBaseChanged = (NewBaseComponent != BasedMovement.MovementBase);
	const bool bBoneChanged = (BoneName != BasedMovement.BoneName);

	if (bBaseChanged || bBoneChanged)
	{
		// Verify no recursion.
		APawn* Loop = (NewBaseComponent ? Cast<APawn>(NewBaseComponent->GetOwner()) : nullptr);
		while (Loop)
		{
			if (Loop == this)
			{
				// UE_LOG(LogCharacter, Warning, TEXT(" SetBase failed! Recursion detected. Pawn %s already based on %s."), *GetName(), *NewBaseComponent->GetName()); //-V595
				return;
			}
			if (UPrimitiveComponent* LoopBase =	Loop->GetMovementBase())
			{
				Loop = Cast<APawn>(LoopBase->GetOwner());
			}
			else
			{
				break;
			}
		}

		// Set base.
		UPrimitiveComponent* OldBase = BasedMovement.MovementBase;
		BasedMovement.MovementBase = NewBaseComponent;
		BasedMovement.BoneName = BoneName;

		if (CharacterMovement)
		{
			const bool bBaseIsSimulating = NewBaseComponent && NewBaseComponent->IsSimulatingPhysics();
			if (bBaseChanged)
			{
				MovementBaseUtility::RemoveTickDependency(CharacterMovement->PrimaryComponentTick, OldBase);
				// We use a special post physics function if simulating, otherwise add normal tick prereqs.
				if (!bBaseIsSimulating)
				{
					MovementBaseUtility::AddTickDependency(CharacterMovement->PrimaryComponentTick, NewBaseComponent);
				}
			}

			if (NewBaseComponent)
			{
				// Update OldBaseLocation/Rotation as those were referring to a different base
				// ... but not when handling replication for proxies (since they are going to copy this data from the replicated values anyway)
				if (!bInBaseReplication)
				{
					// Force base location and relative position to be computed since we have a new base or bone so the old relative offset is meaningless.
					CharacterMovement->SaveBaseLocation();
				}

				// Enable PostPhysics tick if we are standing on a physics object, as we need to to use post-physics transforms
				CharacterMovement->PostPhysicsTickFunction.SetTickFunctionEnable(bBaseIsSimulating);
			}
			else
			{
				BasedMovement.BoneName = NAME_None; // None, regardless of whether user tried to set a bone name, since we have no base component.
				BasedMovement.bRelativeRotation = false;
				CharacterMovement->CurrentFloor.Clear();
				CharacterMovement->PostPhysicsTickFunction.SetTickFunctionEnable(false);
			}

			if (Role == ROLE_Authority || Role == ROLE_AutonomousProxy)
			{
				BasedMovement.bServerHasBaseComponent = (BasedMovement.MovementBase != nullptr); // Also set on proxies for nicer debugging.
				// UE_LOG(LogCharacter, Verbose, TEXT("Setting base on %s for '%s' to '%s'"), Role == ROLE_Authority ? TEXT("Server") : TEXT("AutoProxy"), *GetName(), *GetFullNameSafe(NewBaseComponent));
			}
			else
			{
				// UE_LOG(LogCharacter, Verbose, TEXT("Setting base on Client for '%s' to '%s'"), *GetName(), *GetFullNameSafe(NewBaseComponent));
			}

		}

		// Notify this actor of his new floor.
		if ( bNotifyPawn )
		{
			BaseChange();
		}
	}
}

#pragma region MOVEMENT RPC

// ServerMove
void AMyPawn::ServerMove_Implementation(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode) {
	GetCharacterMovement()->ServerMove_Implementation(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

bool AMyPawn::ServerMove_Validate(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode) {
	return GetCharacterMovement()->ServerMove_Validate(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

// ServerMoveNoBase
void AMyPawn::ServerMoveNoBase_Implementation(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode) {
	GetCharacterMovement()->ServerMove_Implementation(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, /*ClientMovementBase=*/ nullptr, /*ClientBaseBoneName=*/ NAME_None, ClientMovementMode);
}

bool AMyPawn::ServerMoveNoBase_Validate(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode) {
	return GetCharacterMovement()->ServerMove_Validate(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, /*ClientMovementBase=*/ nullptr, /*ClientBaseBoneName=*/ NAME_None, ClientMovementMode);
}

// ServerMoveDual
void AMyPawn::ServerMoveDual_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode) {
	GetCharacterMovement()->ServerMoveDual_Implementation(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

bool AMyPawn::ServerMoveDual_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode) {
	return GetCharacterMovement()->ServerMoveDual_Validate(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

// ServerMoveDualNoBase
void AMyPawn::ServerMoveDualNoBase_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode) {
	GetCharacterMovement()->ServerMoveDual_Implementation(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, /*ClientMovementBase=*/ nullptr, /*ClientBaseBoneName=*/ NAME_None, ClientMovementMode);
}

bool AMyPawn::ServerMoveDualNoBase_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode) {
	return GetCharacterMovement()->ServerMoveDual_Validate(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, /*ClientMovementBase=*/ nullptr, /*ClientBaseBoneName=*/ NAME_None, ClientMovementMode);
}

// ServerMoveDualHybridRootMotion
void AMyPawn::ServerMoveDualHybridRootMotion_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode) {
	GetCharacterMovement()->ServerMoveDualHybridRootMotion_Implementation(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

bool AMyPawn::ServerMoveDualHybridRootMotion_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode) {
	return GetCharacterMovement()->ServerMoveDualHybridRootMotion_Validate(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
}

// ServerMoveOld
void AMyPawn::ServerMoveOld_Implementation(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags) {
	GetCharacterMovement()->ServerMoveOld_Implementation(OldTimeStamp, OldAccel, OldMoveFlags);
}

bool AMyPawn::ServerMoveOld_Validate(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags) {
	return GetCharacterMovement()->ServerMoveOld_Validate(OldTimeStamp, OldAccel, OldMoveFlags);
}

// ClientAckGoodMove
void AMyPawn::ClientAckGoodMove_Implementation(float TimeStamp) {
	GetCharacterMovement()->ClientAckGoodMove_Implementation(TimeStamp);
}

// ClientAdjustPosition
void AMyPawn::ClientAdjustPosition_Implementation(float TimeStamp, FVector NewLoc, FVector NewVel, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode) {
	GetCharacterMovement()->ClientAdjustPosition_Implementation(TimeStamp, NewLoc, NewVel, NewBase, NewBaseBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode);
}

// ClientVeryShortAdjustPosition
void AMyPawn::ClientVeryShortAdjustPosition_Implementation(float TimeStamp, FVector NewLoc, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode) {
	GetCharacterMovement()->ClientVeryShortAdjustPosition_Implementation(TimeStamp, NewLoc, NewBase, NewBaseBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode);
}

// ClientAdjustRootMotionPosition
void AMyPawn::ClientAdjustRootMotionPosition_Implementation(float TimeStamp, float ServerMontageTrackPosition, FVector ServerLoc, FVector_NetQuantizeNormal ServerRotation, float ServerVelZ, UPrimitiveComponent* ServerBase, FName ServerBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode) {
	GetCharacterMovement()->ClientAdjustRootMotionPosition_Implementation(TimeStamp, ServerMontageTrackPosition, ServerLoc, ServerRotation, ServerVelZ, ServerBase, ServerBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode);
}

// ClientAdjustRootMotionSourcePosition
void AMyPawn::ClientAdjustRootMotionSourcePosition_Implementation(float TimeStamp, FRootMotionSourceGroup ServerRootMotion, bool bHasAnimRootMotion, float ServerMontageTrackPosition, FVector ServerLoc, FVector_NetQuantizeNormal ServerRotation, float ServerVelZ, UPrimitiveComponent* ServerBase, FName ServerBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode) {
	GetCharacterMovement()->ClientAdjustRootMotionSourcePosition_Implementation(TimeStamp, ServerRootMotion, bHasAnimRootMotion, ServerMontageTrackPosition, ServerLoc, ServerRotation, ServerVelZ, ServerBase, ServerBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode);
}

#pragma endregion


void AMyPawn::BaseChange() {
	if (CharacterMovement && CharacterMovement->MovementMode != MOVE_None) {
		AActor* ActualMovementBase = GetMovementBaseActor(this);
		if ((ActualMovementBase != nullptr) && !ActualMovementBase->CanBeBaseForCharacter(this)) {
			CharacterMovement->JumpOff(ActualMovementBase);
		}
	}
}

bool AMyPawn::IsPlayingNetworkedRootMotionMontage() const {
	if (Mesh) {
		return Mesh->IsPlayingNetworkedRootMotionMontage();
	}
	return false;
}

void AMyPawn::OnUpdateSimulatedPosition(const FVector& OldLocation, const FQuat& OldRotation) {
	SCOPE_CYCLE_COUNTER(STAT_CharacterOnNetUpdateSimulatedPosition);

	bSimGravityDisabled = false;
	const bool bLocationChanged = (OldLocation != GetActorLocation());
	if (bClientCheckEncroachmentOnNetUpdate) {
		// Only need to check for encroachment when teleported without any velocity.
		// Normal movement pops the character out of geometry anyway, no use doing it before and after (with different rules).
		// Always consider Location as changed if we were spawned this tick as in that case our replicated Location was set as part of spawning, before PreNetReceive()
		if (CharacterMovement->Velocity.IsZero() && (bLocationChanged || CreationTime == GetWorld()->TimeSeconds)) {
			if (GetWorld()->EncroachingBlockingGeometry(this, GetActorLocation(), GetActorRotation())) {
				bSimGravityDisabled = true;
			}
		}
	}
	CharacterMovement->bJustTeleported |= bLocationChanged;
}

void AMyPawn::CacheInitialMeshOffset(FVector MeshRelativeLocation, FRotator MeshRelativeRotation) {
	BaseTranslationOffset = MeshRelativeLocation;
	BaseRotationOffset = MeshRelativeRotation.Quaternion();
}

void AMyPawn::OnRep_ReplicatedBasedMovement() {
	if (Role != ROLE_SimulatedProxy) {
		return;
	}

	// Skip base updates while playing root motion, it is handled inside of OnRep_RootMotion
	if (IsPlayingNetworkedRootMotionMontage()) {
		return;
	}

	TGuardValue<bool> bInBaseReplicationGuard(bInBaseReplication, true);

	const bool bBaseChanged = (BasedMovement.MovementBase != ReplicatedBasedMovement.MovementBase || BasedMovement.BoneName != ReplicatedBasedMovement.BoneName);
	if (bBaseChanged) {
		// Even though we will copy the replicated based movement info, we need to use SetBase() to set up tick dependencies and trigger notifications.
		SetBase(ReplicatedBasedMovement.MovementBase, ReplicatedBasedMovement.BoneName);
	}

	// Make sure to use the values of relative location/rotation etc from the server.
	BasedMovement = ReplicatedBasedMovement;

	if (ReplicatedBasedMovement.HasRelativeLocation()) {
		// Update transform relative to movement base
		const FVector OldLocation = GetActorLocation();
		const FQuat OldRotation = GetActorQuat();
		MovementBaseUtility::GetMovementBaseTransform(ReplicatedBasedMovement.MovementBase, ReplicatedBasedMovement.BoneName, CharacterMovement->OldBaseLocation, CharacterMovement->OldBaseQuat);
		const FVector NewLocation = CharacterMovement->OldBaseLocation + ReplicatedBasedMovement.Location;
		FRotator NewRotation;

		if (ReplicatedBasedMovement.HasRelativeRotation()) {
			// Relative location, relative rotation
			NewRotation = (FRotationMatrix(ReplicatedBasedMovement.Rotation) * FQuatRotationMatrix(CharacterMovement->OldBaseQuat)).Rotator();

			// TODO: need a better way to not assume we only use Yaw.
			NewRotation.Pitch = 0.f;
			NewRotation.Roll = 0.f;
		}
		else {
			// Relative location, absolute rotation
			NewRotation = ReplicatedBasedMovement.Rotation;
		}

		// When position or base changes, movement mode will need to be updated. This assumes rotation changes don't affect that.
		CharacterMovement->bJustTeleported |= (bBaseChanged || NewLocation != OldLocation);
		CharacterMovement->bNetworkSmoothingComplete = false;
		CharacterMovement->SmoothCorrection(OldLocation, OldRotation, NewLocation, NewRotation.Quaternion());
		OnUpdateSimulatedPosition(OldLocation, OldRotation);
	}
}

void AMyPawn::SetReplicateMovement(bool bInReplicateMovement) {
	Super::SetReplicateMovement(bInReplicateMovement);

	if (CharacterMovement != nullptr && Role == ROLE_Authority) {
		// Set prediction data time stamp to current time to stop extrapolating
		// from time bReplicateMovement was turned off to when it was turned on again
		FNetworkPredictionData_Server* NetworkPrediction = CharacterMovement->HasPredictionData_Server() ? CharacterMovement->GetPredictionData_Server() : nullptr;

		if (NetworkPrediction != nullptr) {
			NetworkPrediction->ServerTimeStamp = GetWorld()->GetTimeSeconds();
		}
	}
}

void AMyPawn::SaveRelativeBasedMovement(const FVector& NewRelativeLocation, const FRotator& NewRotation, bool bRelativeRotation) {
	checkSlow(BasedMovement.HasRelativeLocation());
	BasedMovement.Location = NewRelativeLocation;
	BasedMovement.Rotation = NewRotation;
	BasedMovement.bRelativeRotation = bRelativeRotation;
}

float AMyPawn::PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName) {
	UAnimInstance * AnimInstance = (Mesh) ? Mesh->GetAnimInstance() : nullptr;
	if (AnimMontage && AnimInstance) {
		float const Duration = AnimInstance->Montage_Play(AnimMontage, InPlayRate);

		if (Duration > 0.f) {
			// Start at a given Section.
			if (StartSectionName != NAME_None) {
				AnimInstance->Montage_JumpToSection(StartSectionName, AnimMontage);
			}

			return Duration;
		}
	}

	return 0.f;
}

void AMyPawn::StopAnimMontage(class UAnimMontage* AnimMontage) {
	UAnimInstance * AnimInstance = (Mesh) ? Mesh->GetAnimInstance() : nullptr;
	UAnimMontage * MontageToStop = (AnimMontage) ? AnimMontage : GetCurrentMontage();
	bool bShouldStopMontage = AnimInstance && MontageToStop && !AnimInstance->Montage_GetIsStopped(MontageToStop);

	if (bShouldStopMontage) {
		AnimInstance->Montage_Stop(MontageToStop->BlendOut.GetBlendTime(), MontageToStop);
	}
}

class UAnimMontage * AMyPawn::GetCurrentMontage() {
	UAnimInstance * AnimInstance = (Mesh) ? Mesh->GetAnimInstance() : nullptr;
	if (AnimInstance) {
		return AnimInstance->GetCurrentActiveMontage();
	}

	return nullptr;
}


int32 AMyPawn::FindRootMotionRepMove(const FAnimMontageInstance& ClientMontageInstance) const {
	int32 FoundIndex = INDEX_NONE;

	// Start with most recent move and go back in time to find a usable move.
	for (int32 MoveIndex = RootMotionRepMoves.Num() - 1; MoveIndex >= 0; MoveIndex--) {
		if (CanUseRootMotionRepMove(RootMotionRepMoves[MoveIndex], ClientMontageInstance)) {
			FoundIndex = MoveIndex;
			break;
		}
	}

	UE_LOG(LogRootMotion, Log, TEXT("\tACharacter::FindRootMotionRepMove FoundIndex: %d, NumSavedMoves: %d"), FoundIndex, RootMotionRepMoves.Num());
	return FoundIndex;
}

bool AMyPawn::CanUseRootMotionRepMove(const FSimulatedRootMotionReplicatedMove& RootMotionRepMove, const FAnimMontageInstance& ClientMontageInstance) const {
	// Ignore outdated moves.
	if (GetWorld()->TimeSince(RootMotionRepMove.Time) <= 0.5f) {
		// Make sure montage being played matched between client and server.
		if (RootMotionRepMove.RootMotion.AnimMontage && (RootMotionRepMove.RootMotion.AnimMontage == ClientMontageInstance.Montage)) {
			UAnimMontage * AnimMontage = ClientMontageInstance.Montage;
			const float ServerPosition = RootMotionRepMove.RootMotion.Position;
			const float ClientPosition = ClientMontageInstance.GetPosition();
			const float DeltaPosition = (ClientPosition - ServerPosition);
			const int32 CurrentSectionIndex = AnimMontage->GetSectionIndexFromPosition(ClientPosition);
			if (CurrentSectionIndex != INDEX_NONE) {
				const int32 NextSectionIndex = ClientMontageInstance.GetNextSectionID(CurrentSectionIndex);

				// We can only extract root motion if we are within the same section.
				// It's not trivial to jump through sections in a deterministic manner, but that is luckily not frequent. 
				const bool bSameSections = (AnimMontage->GetSectionIndexFromPosition(ServerPosition) == CurrentSectionIndex);
				// if we are looping and just wrapped over, skip. That's also not easy to handle and not frequent.
				const bool bHasLooped = (NextSectionIndex == CurrentSectionIndex) && (FMath::Abs(DeltaPosition) > (AnimMontage->GetSectionLength(CurrentSectionIndex) / 2.f));
				// Can only simulate forward in time, so we need to make sure server move is not ahead of the client.
				const bool bServerAheadOfClient = ((DeltaPosition * ClientMontageInstance.GetPlayRate()) < 0.f);

				UE_LOG(LogRootMotion, Log, TEXT("\t\tACharacter::CanUseRootMotionRepMove ServerPosition: %.3f, ClientPosition: %.3f, DeltaPosition: %.3f, bSameSections: %d, bHasLooped: %d, bServerAheadOfClient: %d"),
					ServerPosition, ClientPosition, DeltaPosition, bSameSections, bHasLooped, bServerAheadOfClient);

				return bSameSections && !bHasLooped && !bServerAheadOfClient;
			}
		}
	}
	return false;
}

bool AMyPawn::RestoreReplicatedMove(const FSimulatedRootMotionReplicatedMove& RootMotionRepMove) {
	UPrimitiveComponent* ServerBase = RootMotionRepMove.RootMotion.MovementBase;
	const FName ServerBaseBoneName = RootMotionRepMove.RootMotion.MovementBaseBoneName;

	// Relative Position
	if (RootMotionRepMove.RootMotion.bRelativePosition) {
		bool bSuccess = false;
		if (MovementBaseUtility::UseRelativeLocation(ServerBase)) {
			FVector BaseLocation;
			FQuat BaseRotation;
			MovementBaseUtility::GetMovementBaseTransform(ServerBase, ServerBaseBoneName, BaseLocation, BaseRotation);

			const FVector ServerLocation = BaseLocation + RootMotionRepMove.RootMotion.Location;
			FRotator ServerRotation;
			if (RootMotionRepMove.RootMotion.bRelativeRotation) {
				// Relative rotation
				ServerRotation = (FRotationMatrix(RootMotionRepMove.RootMotion.Rotation) * FQuatRotationTranslationMatrix(BaseRotation, FVector::ZeroVector)).Rotator();
			}
			else {
				// Absolute rotation
				ServerRotation = RootMotionRepMove.RootMotion.Rotation;
			}

			SetActorLocationAndRotation(ServerLocation, ServerRotation);
			bSuccess = true;
		}
		// If we received local space position, but can't resolve parent, then move can't be used. :(
		if (!bSuccess) {
			return false;
		}
	}
	// Absolute position
	else {
		FVector LocalLocation = FRepMovement::RebaseOntoLocalOrigin(RootMotionRepMove.RootMotion.Location, this);
		SetActorLocationAndRotation(LocalLocation, RootMotionRepMove.RootMotion.Rotation);
	}

	CharacterMovement->bJustTeleported = true;
	SetBase(ServerBase, ServerBaseBoneName);

	return true;
}

void AMyPawn::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) {
	Super::PreReplication(ChangedPropertyTracker);

	if (CharacterMovement->CurrentRootMotion.HasActiveRootMotionSources() || IsPlayingNetworkedRootMotionMontage()) {
		const FAnimMontageInstance* RootMotionMontageInstance = GetRootMotionAnimMontageInstance();

		RepRootMotion.bIsActive = true;
		// Is position stored in local space?
		RepRootMotion.bRelativePosition = BasedMovement.HasRelativeLocation();
		RepRootMotion.bRelativeRotation = BasedMovement.HasRelativeRotation();
		RepRootMotion.Location = RepRootMotion.bRelativePosition ? BasedMovement.Location : FRepMovement::RebaseOntoZeroOrigin(GetActorLocation(), GetWorld()->OriginLocation);
		RepRootMotion.Rotation = RepRootMotion.bRelativeRotation ? BasedMovement.Rotation : GetActorRotation();
		RepRootMotion.MovementBase = BasedMovement.MovementBase;
		RepRootMotion.MovementBaseBoneName = BasedMovement.BoneName;
		if (RootMotionMontageInstance) {
			RepRootMotion.AnimMontage = RootMotionMontageInstance->Montage;
			RepRootMotion.Position = RootMotionMontageInstance->GetPosition();
		}
		else {
			RepRootMotion.AnimMontage = nullptr;
		}

		RepRootMotion.AuthoritativeRootMotion = CharacterMovement->CurrentRootMotion;
		RepRootMotion.Acceleration = CharacterMovement->GetCurrentAcceleration();
		RepRootMotion.LinearVelocity = CharacterMovement->Velocity;

		DOREPLIFETIME_ACTIVE_OVERRIDE(ACharacter, RepRootMotion, true);
	}
	else {
		RepRootMotion.Clear();

		DOREPLIFETIME_ACTIVE_OVERRIDE(ACharacter, RepRootMotion, false);
	}

	ReplicatedServerLastTransformUpdateTimeStamp = CharacterMovement->GetServerLastTransformUpdateTimeStamp();
	ReplicatedMovementMode = CharacterMovement->PackNetworkMovementMode();
	ReplicatedBasedMovement = BasedMovement;

	// Optimization: only update and replicate these values if they are actually going to be used.
	if (BasedMovement.HasRelativeLocation()) {
		// When velocity becomes zero, force replication so the position is updated to match the server (it may have moved due to simulation on the client).
		ReplicatedBasedMovement.bServerHasVelocity = !CharacterMovement->Velocity.IsZero();

		// Make sure absolute rotations are updated in case rotation occurred after the base info was saved.
		if (!BasedMovement.HasRelativeRotation()) {
			ReplicatedBasedMovement.Rotation = GetActorRotation();
		}
	}
}

void AMyPawn::OnRep_RootMotion() {
	if (Role == ROLE_SimulatedProxy) {
		// UE_LOG(LogRootMotion, Log, TEXT("ACharacter::OnRep_RootMotion"));

		// Save received move in queue, we'll try to use it during Tick().
		if (RepRootMotion.bIsActive) {
			if (CharacterMovement) {
				// Add new move
				RootMotionRepMoves.AddZeroed(1);
				FSimulatedRootMotionReplicatedMove& NewMove = RootMotionRepMoves.Last();
				NewMove.RootMotion = RepRootMotion;
				NewMove.Time = GetWorld()->GetTimeSeconds();

				// Convert RootMotionSource Server IDs -> Local IDs in AuthoritativeRootMotion and cull invalid
				// so that when we use this root motion it has the correct IDs
				CharacterMovement->ConvertRootMotionServerIDsToLocalIDs(CharacterMovement->CurrentRootMotion, NewMove.RootMotion.AuthoritativeRootMotion, NewMove.Time);
				NewMove.RootMotion.AuthoritativeRootMotion.CullInvalidSources();
			}
		}
		else {
			// Clear saved moves.
			RootMotionRepMoves.Empty();
		}
	}
}

void AMyPawn::SimulatedRootMotionPositionFixup(float DeltaSeconds) {
	const FAnimMontageInstance* ClientMontageInstance = GetRootMotionAnimMontageInstance();
	if (ClientMontageInstance && CharacterMovement && Mesh) {
		// Find most recent buffered move that we can use.
		const int32 MoveIndex = FindRootMotionRepMove(*ClientMontageInstance);
		if (MoveIndex != INDEX_NONE) {
			const FVector OldLocation = GetActorLocation();
			const FQuat OldRotation = GetActorQuat();
			// Move Actor back to position of that buffered move. (server replicated position).
			const FSimulatedRootMotionReplicatedMove& RootMotionRepMove = RootMotionRepMoves[MoveIndex];
			if (RestoreReplicatedMove(RootMotionRepMove)) {
				const float ServerPosition = RootMotionRepMove.RootMotion.Position;
				const float ClientPosition = ClientMontageInstance->GetPosition();
				const float DeltaPosition = (ClientPosition - ServerPosition);
				if (FMath::Abs(DeltaPosition) > KINDA_SMALL_NUMBER) {
					// Find Root Motion delta move to get back to where we were on the client.
					const FTransform LocalRootMotionTransform = ClientMontageInstance->Montage->ExtractRootMotionFromTrackRange(ServerPosition, ClientPosition);

					// Simulate Root Motion for delta move.
					if (CharacterMovement) {
						const float MontagePlayRate = ClientMontageInstance->GetPlayRate();
						// Guess time it takes for this delta track position, so we can get falling physics accurate.
						if (!FMath::IsNearlyZero(MontagePlayRate)) {
							const float DeltaTime = DeltaPosition / MontagePlayRate;

							// Even with negative playrate deltatime should be positive.
							check(DeltaTime > 0.f);
							CharacterMovement->SimulateRootMotion(DeltaTime, LocalRootMotionTransform);

							// After movement correction, smooth out error in position if any.
							CharacterMovement->bNetworkSmoothingComplete = false;
							CharacterMovement->SmoothCorrection(OldLocation, OldRotation, GetActorLocation(), GetActorQuat());
						}
					}
				}
			}

			// Delete this move and any prior one, we don't need them anymore.
			// UE_LOG(LogRootMotion, Log, TEXT("\tClearing old moves (%d)"), MoveIndex + 1);
			RootMotionRepMoves.RemoveAt(0, MoveIndex + 1);
		}
	}
}

/** Get FAnimMontageInstance playing RootMotion */
FAnimMontageInstance * AMyPawn::GetRootMotionAnimMontageInstance() const {
	return (Mesh && Mesh->GetAnimInstance()) ? Mesh->GetAnimInstance()->GetRootMotionMontageInstance() : nullptr;
}

bool AMyPawn::IsPlayingRootMotion() const {
	if (Mesh) {
		return Mesh->IsPlayingRootMotion();
	}
	return false;
}

void AMyPawn::SetAnimRootMotionTranslationScale(float InAnimRootMotionTranslationScale) {
	AnimRootMotionTranslationScale = InAnimRootMotionTranslationScale;
}

float AMyPawn::GetAnimRootMotionTranslationScale() const {
	return AnimRootMotionTranslationScale;
}

void AMyPawn::PreReplicationForReplay(IRepChangedPropertyTracker & ChangedPropertyTracker) {
	Super::PreReplicationForReplay(ChangedPropertyTracker);

	// If this is a replay, we save out certain values we need to runtime to do smooth interpolation
	// We'll be able to look ahead in the replay to have these ahead of time for smoother playback
	FCharacterReplaySample ReplaySample;

	// If this is a client-recorded replay, use the mesh location and rotation, since these will always
	// be smoothed - unlike the actor position and rotation.
	const USkeletalMeshComponent* const MeshComponent = GetMesh();
	if (MeshComponent && GetWorld()->IsRecordingClientReplay()) {
		// Remove the base transform from the mesh's transform, since on playback the base transform
		// will be stored in the mesh's RelativeLocation and RelativeRotation.
		const FTransform BaseTransform(GetBaseRotationOffset(), GetBaseTranslationOffset());
		const FTransform MeshRootTransform = BaseTransform.Inverse() * MeshComponent->GetComponentTransform();

		ReplaySample.Location = MeshRootTransform.GetLocation();
		ReplaySample.Rotation = MeshRootTransform.GetRotation().Rotator();
	}
	else {
		ReplaySample.Location = GetActorLocation();
		ReplaySample.Rotation = GetActorRotation();
	}

	ReplaySample.Velocity = GetVelocity();
	ReplaySample.Acceleration = CharacterMovement->GetCurrentAcceleration();
	ReplaySample.RemoteViewPitch = RemoteViewPitch;

	FBitWriter Writer(0, true);
	Writer << ReplaySample;

	ChangedPropertyTracker.SetExternalData(Writer.GetData(), Writer.GetNumBits());
}