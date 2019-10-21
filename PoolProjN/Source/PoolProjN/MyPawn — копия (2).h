// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UObjectGlobals.h"
#include "Templates/SubclassOf.h"
#include "UObject/CoreNet.h"
#include "Engine/NetSerialization.h"
#include "Engine/EngineTypes.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Animation/AnimationAsset.h"
#include "GameFramework/RootMotionSource.h"
#include "Character.generated.h"

class AController;
class FDebugDisplayInfo;
class UAnimMontage;
class UArrowComponent;
class UCapsuleComponent;
class UCharacterMovementComponent;
class UPawnMovementComponent;
class UPrimitiveComponent;
class USkeletalMeshComponent;
struct FAnimMontageInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMovementModeChangedSignature, class ACharacter*, Character, EMovementMode, PrevMovementMode, uint8, PreviousCustomMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCharacterMovementUpdatedSignature, float, DeltaSeconds, FVector, OldLocation, FVector, OldVelocity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCharacterReachedApexSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLandedSignature, const FHitResult&, Hit);

UCLASS(config=Game, BlueprintType, meta=(ShortTooltip="A character is a type of Pawn that includes the ability to walk around."))
class ENGINE_API ACharacter : public APawn
{
	GENERATED_BODY()
public:
	/** Default UObject constructor. */
	ACharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	/** Scale to apply to root motion translation on this Character */
	UPROPERTY(Replicated)
	float AnimRootMotionTranslationScale;

public:
	/** Set whether this actor's movement replicates to network clients. */
	UFUNCTION(BlueprintCallable, Category=Replication)
	virtual void SetReplicateMovement(bool bInReplicateMovement) override;

protected:

	/** CharacterMovement ServerLastTransformUpdateTimeStamp value, replicated to simulated proxies. */
	UPROPERTY(Replicated)
	float ReplicatedServerLastTransformUpdateTimeStamp;

public:
	/** Accessor for ReplicatedServerLastTransformUpdateTimeStamp. */
	FORCEINLINE float GetReplicatedServerLastTransformUpdateTimeStamp() const { return ReplicatedServerLastTransformUpdateTimeStamp; }

	/** Save a new relative location in BasedMovement and a new rotation with is either relative or absolute. */
	void SaveRelativeBasedMovement(const FVector& NewRelativeLocation, const FRotator& NewRotation, bool bRelativeRotation);

	/** Default crouched eye height */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera)
	float CrouchedEyeHeight;

	/** Set by character movement to specify that this Character is currently crouched. */
	UPROPERTY(BlueprintReadOnly, replicatedUsing=OnRep_IsCrouched, Category=Character)
	uint32 bIsCrouched:1;

	/** Handle Crouching replicated from server */
	UFUNCTION()
	virtual void OnRep_IsCrouched();

	/** When true, player wants to jump */
	UPROPERTY(BlueprintReadOnly, Category=Character)
	uint32 bPressedJump:1;

	/** When true, applying updates to network client (replaying saved moves for a locally controlled character) */
	UPROPERTY(Transient)
	uint32 bClientUpdating:1;

	/** True if Pawn was initially falling when started to replay network moves. */
	UPROPERTY(Transient)
	uint32 bClientWasFalling:1; 

	/** If server disagrees with root motion track position, client has to resimulate root motion from last AckedMove. */
	UPROPERTY(Transient)
	uint32 bClientResimulateRootMotion:1;

	/** If server disagrees with root motion state, client has to resimulate root motion from last AckedMove. */
	UPROPERTY(Transient)
	uint32 bClientResimulateRootMotionSources:1;

	/** Disable root motion on the server. When receiving a DualServerMove, where the first move is not root motion and the second is. */
	UPROPERTY(Transient)
	uint32 bServerMoveIgnoreRootMotion:1;

	/** Tracks whether or not the character was already jumping last frame. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category=Character)
	uint32 bWasJumping : 1;

	/** 
	 * Jump key Held Time.
	 * This is the time that the player has held the jump key, in seconds.
	 */
	UPROPERTY(Transient, BlueprintReadOnly, VisibleInstanceOnly, Category=Character)
	float JumpKeyHoldTime;

	/** 
	 * The max time the jump key can be held.
	 * Note that if StopJumping() is not called before the max jump hold time is reached,
	 * then the character will carry on receiving vertical velocity. Therefore it is usually 
	 * best to call StopJumping() when jump input has ceased (such as a button up event).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category=Character, Meta=(ClampMin=0.0, UIMin=0.0))
	float JumpMaxHoldTime;

    /**
     * The max number of jumps the character can perform.
     * Note that if JumpMaxHoldTime is non zero and StopJumping is not called, the player
     * may be able to perform and unlimited number of jumps. Therefore it is usually
     * best to call StopJumping() when jump input has ceased (such as a button up event).
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category=Character)
    int32 JumpMaxCount;

    /**
     * Tracks the current number of jumps performed.
     * This is incremented in CheckJumpInput, used in CanJump_Implementation, and reset in OnMovementModeChanged.
     * When providing overrides for these methods, it's recommended to either manually
     * increment / reset this value, or call the Super:: method.
     */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category=Character)
    int32 JumpCurrentCount;

	/** Apply momentum caused by damage. */
	virtual void ApplyDamageMomentum(float DamageTaken, FDamageEvent const& DamageEvent, APawn* PawnInstigator, AActor* DamageCauser);

	/** 
	 * Make the character jump on the next update.	 
	 * If you want your character to jump according to the time that the jump key is held,
	 * then you can set JumpKeyHoldTime to some non-zero value. Make sure in this case to
	 * call StopJumping() when you want the jump's z-velocity to stop being applied (such 
	 * as on a button up event), otherwise the character will carry on receiving the 
	 * velocity until JumpKeyHoldTime is reached.
	 */
	UFUNCTION(BlueprintCallable, Category=Character)
	virtual void Jump();

	/** 
	 * Stop the character from jumping on the next update. 
	 * Call this from an input event (such as a button 'up' event) to cease applying
	 * jump Z-velocity. If this is not called, then jump z-velocity will be applied
	 * until JumpMaxHoldTime is reached.
	 */
	UFUNCTION(BlueprintCallable, Category=Character)
	virtual void StopJumping();

	/**
	 * Check if the character can jump in the current state.
	 *
	 * The default implementation may be overridden or extended by implementing the custom CanJump event in Blueprints. 
	 * 
	 * @Return Whether the character can jump in the current state. 
	 */
	UFUNCTION(BlueprintCallable, Category=Character)
	bool CanJump() const;

protected:
	/**
	 * Customizable event to check if the character can jump in the current state.
	 * Default implementation returns true if the character is on the ground and not crouching,
	 * has a valid CharacterMovementComponent and CanEverJump() returns true.
	 * Default implementation also allows for 'hold to jump higher' functionality: 
	 * As well as returning true when on the ground, it also returns true when GetMaxJumpTime is more
	 * than zero and IsJumping returns true.
	 * 
	 *
	 * @Return Whether the character can jump in the current state. 
	 */
	UFUNCTION(BlueprintNativeEvent, Category=Character, meta=(DisplayName="CanJump"))
	bool CanJumpInternal() const;
	virtual bool CanJumpInternal_Implementation() const;

public:

	/** Marks character as not trying to jump */
	void ResetJumpState();

	/**
	 * True if jump is actively providing a force, such as when the jump key is held and the time it has been held is less than JumpMaxHoldTime.
	 * @see CharacterMovement->IsFalling
	 */
	UFUNCTION(BlueprintCallable, Category=Character)
	virtual bool IsJumpProvidingForce() const;

	/** Play Animation Montage on the character mesh **/
	UFUNCTION(BlueprintCallable, Category=Animation)
	virtual float PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None);

	/** Stop Animation Montage. If nullptr, it will stop what's currently active. The Blend Out Time is taken from the montage asset that is being stopped. **/
	UFUNCTION(BlueprintCallable, Category=Animation)
	virtual void StopAnimMontage(class UAnimMontage* AnimMontage = nullptr);

	/** Return current playing Montage **/
	UFUNCTION(BlueprintCallable, Category=Animation)
	class UAnimMontage* GetCurrentMontage();

	/**
	 * Set a pending launch velocity on the Character. This velocity will be processed on the next CharacterMovementComponent tick,
	 * and will set it to the "falling" state. Triggers the OnLaunched event.
	 * @PARAM LaunchVelocity is the velocity to impart to the Character
	 * @PARAM bXYOverride if true replace the XY part of the Character's velocity instead of adding to it.
	 * @PARAM bZOverride if true replace the Z component of the Character's velocity instead of adding to it.
	 */
	UFUNCTION(BlueprintCallable, Category=Character)
	virtual void LaunchCharacter(FVector LaunchVelocity, bool bXYOverride, bool bZOverride);

	/** Let blueprint know that we were launched */
	UFUNCTION(BlueprintImplementableEvent)
	void OnLaunched(FVector LaunchVelocity, bool bXYOverride, bool bZOverride);

	/** Event fired when the character has just started jumping */
	UFUNCTION(BlueprintNativeEvent, Category=Character)
	void OnJumped();
	virtual void OnJumped_Implementation();

	/** Called when the character's movement enters falling */
	virtual void Falling() {}

	/** Called when character's jump reaches Apex. Needs CharacterMovement->bNotifyApex = true */
	virtual void NotifyJumpApex();

	/** Broadcast when Character's jump reaches its apex. Needs CharacterMovement->bNotifyApex = true */
	UPROPERTY(BlueprintAssignable, Category=Character)
	FCharacterReachedApexSignature OnReachedJumpApex;

	/**
	 * Called upon landing when falling, to perform actions based on the Hit result. Triggers the OnLanded event.
	 * Note that movement mode is still "Falling" during this event. Current Velocity value is the velocity at the time of landing.
	 * Consider OnMovementModeChanged() as well, as that can be used once the movement mode changes to the new mode (most likely Walking).
	 *
	 * @param Hit Result describing the landing that resulted in a valid landing spot.
	 * @see OnMovementModeChanged()
	 */
	virtual void Landed(const FHitResult& Hit);

	/**
	 * Called upon landing when falling, to perform actions based on the Hit result.
	 * Note that movement mode is still "Falling" during this event. Current Velocity value is the velocity at the time of landing.
	 * Consider OnMovementModeChanged() as well, as that can be used once the movement mode changes to the new mode (most likely Walking).
	 *
	 * @param Hit Result describing the landing that resulted in a valid landing spot.
	 * @see OnMovementModeChanged()
	 */
	FLandedSignature LandedDelegate;

	/**
	 * Called upon landing when falling, to perform actions based on the Hit result.
	 * Note that movement mode is still "Falling" during this event. Current Velocity value is the velocity at the time of landing.
	 * Consider OnMovementModeChanged() as well, as that can be used once the movement mode changes to the new mode (most likely Walking).
	 *
	 * @param Hit Result describing the landing that resulted in a valid landing spot.
	 * @see OnMovementModeChanged()
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void OnLanded(const FHitResult& Hit);

	/**
	 * Event fired when the Character is walking off a surface and is about to fall because CharacterMovement->CurrentFloor became unwalkable.
	 * If CharacterMovement->MovementMode does not change during this event then the character will automatically start falling afterwards.
	 * @note Z velocity is zero during walking movement, and will be here as well. Another velocity can be computed here if desired and will be used when starting to fall.
	 *
	 * @param  PreviousFloorImpactNormal Normal of the previous walkable floor.
	 * @param  PreviousFloorContactNormal Normal of the contact with the previous walkable floor.
	 * @param  PreviousLocation	Previous character location before movement off the ledge.
	 * @param  TimeTick	Time delta of movement update resulting in moving off the ledge.
	 */
	UFUNCTION(BlueprintNativeEvent, Category=Character)
	void OnWalkingOffLedge(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta);
	virtual void OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta);

	/**
	 * Called when pawn's movement is blocked
	 * @param Impact describes the blocking hit.
	 */
	virtual void MoveBlockedBy(const FHitResult& Impact) {};

	/**
	 * Request the character to start crouching. The request is processed on the next update of the CharacterMovementComponent.
	 * @see OnStartCrouch
	 * @see IsCrouched
	 * @see CharacterMovement->WantsToCrouch
	 */
	UFUNCTION(BlueprintCallable, Category=Character, meta=(HidePin="bClientSimulation"))
	virtual void Crouch(bool bClientSimulation = false);

	/**
	 * Request the character to stop crouching. The request is processed on the next update of the CharacterMovementComponent.
	 * @see OnEndCrouch
	 * @see IsCrouched
	 * @see CharacterMovement->WantsToCrouch
	 */
	UFUNCTION(BlueprintCallable, Category=Character, meta=(HidePin="bClientSimulation"))
	virtual void UnCrouch(bool bClientSimulation = false);

	/** @return true if this character is currently able to crouch (and is not currently crouched) */
	virtual bool CanCrouch();

	/** 
	 * Called when Character stops crouching. Called on non-owned Characters through bIsCrouched replication.
	 * @param	HalfHeightAdjust		difference between default collision half-height, and actual crouched capsule half-height.
	 * @param	ScaledHalfHeightAdjust	difference after component scale is taken in to account.
	 */
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

	/** 
	 * Event when Character stops crouching.
	 * @param	HalfHeightAdjust		difference between default collision half-height, and actual crouched capsule half-height.
	 * @param	ScaledHalfHeightAdjust	difference after component scale is taken in to account.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnEndCrouch", ScriptName="OnEndCrouch"))
	void K2_OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

	/**
	 * Called when Character crouches. Called on non-owned Characters through bIsCrouched replication.
	 * @param	HalfHeightAdjust		difference between default collision half-height, and actual crouched capsule half-height.
	 * @param	ScaledHalfHeightAdjust	difference after component scale is taken in to account.
	 */
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

	/**
	 * Event when Character crouches.
	 * @param	HalfHeightAdjust		difference between default collision half-height, and actual crouched capsule half-height.
	 * @param	ScaledHalfHeightAdjust	difference after component scale is taken in to account.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnStartCrouch", ScriptName="OnStartCrouch"))
	void K2_OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

	/**
	 * Called from CharacterMovementComponent to notify the character that the movement mode has changed.
	 * @param	PrevMovementMode	Movement mode before the change
	 * @param	PrevCustomMode		Custom mode before the change (applicable if PrevMovementMode is Custom)
	 */
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0);

	/** Multicast delegate for MovementMode changing. */
	UPROPERTY(BlueprintAssignable, Category=Character)
	FMovementModeChangedSignature MovementModeChangedDelegate;

	/**
	 * Called from CharacterMovementComponent to notify the character that the movement mode has changed.
	 * @param	PrevMovementMode	Movement mode before the change
	 * @param	NewMovementMode		New movement mode
	 * @param	PrevCustomMode		Custom mode before the change (applicable if PrevMovementMode is Custom)
	 * @param	NewCustomMode		New custom mode (applicable if NewMovementMode is Custom)
	 */
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnMovementModeChanged", ScriptName="OnMovementModeChanged"))
	void K2_OnMovementModeChanged(EMovementMode PrevMovementMode, EMovementMode NewMovementMode, uint8 PrevCustomMode, uint8 NewCustomMode);

	/**
	 * Event for implementing custom character movement mode. Called by CharacterMovement if MovementMode is set to Custom.
	 * @note C++ code should override UCharacterMovementComponent::PhysCustom() instead.
	 * @see UCharacterMovementComponent::PhysCustom()
	 */
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="UpdateCustomMovement", ScriptName="UpdateCustomMovement"))
	void K2_UpdateCustomMovement(float DeltaTime);

	/**
	 * Event triggered at the end of a CharacterMovementComponent movement update.
	 * This is the preferred event to use rather than the Tick event when performing custom updates to CharacterMovement properties based on the current state.
	 * This is mainly due to the nature of network updates, where client corrections in position from the server can cause multiple iterations of a movement update,
	 * which allows this event to update as well, while a Tick event would not.
	 *
	 * @param	DeltaSeconds		Delta time in seconds for this update
	 * @param	InitialLocation		Location at the start of the update. May be different than the current location if movement occurred.
	 * @param	InitialVelocity		Velocity at the start of the update. May be different than the current velocity.
	 */
	UPROPERTY(BlueprintAssignable, Category=Character)
	FCharacterMovementUpdatedSignature OnCharacterMovementUpdated;

	/** Returns true if the Landed() event should be called. Used by CharacterMovement to prevent notifications while playing back network moves. */
	virtual bool ShouldNotifyLanded(const struct FHitResult& Hit);

	/** Trigger jump if jump button has been pressed. */
	virtual void CheckJumpInput(float DeltaTime);

	/** Reset jump input state after having checked input. */
	virtual void ClearJumpInput();

	/**
	 * Get the maximum jump time for the character.
	 * Note that if StopJumping() is not called before the max jump hold time is reached,
	 * then the character will carry on receiving vertical velocity. Therefore it is usually 
	 * best to call StopJumping() when jump input has ceased (such as a button up event).
	 * 
	 * @return Maximum jump time for the character
	 */
	virtual float GetJumpMaxHoldTime() const;

	UFUNCTION(Reliable, Client)
	void ClientCheatWalk();
	virtual void ClientCheatWalk_Implementation();

	UFUNCTION(Reliable, Client)
	void ClientCheatFly();
	virtual void ClientCheatFly_Implementation();

	UFUNCTION(Reliable, Client)
	void ClientCheatGhost();
	virtual void ClientCheatGhost_Implementation();

	UFUNCTION(Reliable, Client)
	void RootMotionDebugClientPrintOnScreen(const FString& InString);
	virtual void RootMotionDebugClientPrintOnScreen_Implementation(const FString& InString);

	// Root Motion

	/** 
	 * For LocallyControlled Autonomous clients. 
	 * During a PerformMovement() after root motion is prepared, we save it off into this and
	 * then record it into our SavedMoves.
	 * During SavedMove playback we use it as our "Previous Move" SavedRootMotion which includes
	 * last received root motion from the Server
	 */
	UPROPERTY(Transient)
	FRootMotionSourceGroup SavedRootMotion;

	/** For LocallyControlled Autonomous clients. Saved root motion data to be used by SavedMoves. */
	UPROPERTY(Transient)
	FRootMotionMovementParams ClientRootMotionParams;

	/** Array of previously received root motion moves from the server. */
	UPROPERTY(Transient)
	TArray<FSimulatedRootMotionReplicatedMove> RootMotionRepMoves;
	
	/** Find usable root motion replicated move from our buffer.
	 * Goes through the buffer back in time, to find the first move that clears 'CanUseRootMotionRepMove' below.
	 * Returns index of that move or INDEX_NONE otherwise.
	 */
	int32 FindRootMotionRepMove(const FAnimMontageInstance& ClientMontageInstance) const;

	/** True if buffered move is usable to teleport client back to. */
	bool CanUseRootMotionRepMove(const FSimulatedRootMotionReplicatedMove& RootMotionRepMove, const FAnimMontageInstance& ClientMontageInstance) const;

	/** Restore actor to an old buffered move. */
	bool RestoreReplicatedMove(const FSimulatedRootMotionReplicatedMove& RootMotionRepMove);

	/** Replicated Root Motion montage */
	UPROPERTY(ReplicatedUsing=OnRep_RootMotion)
	struct FRepRootMotionMontage RepRootMotion;
	
	/** Handles replicated root motion properties on simulated proxies and position correction. */
	UFUNCTION()
	void OnRep_RootMotion();

	/** Position fix up for Simulated Proxies playing Root Motion */
	void SimulatedRootMotionPositionFixup(float DeltaSeconds);

	/** Get FAnimMontageInstance playing RootMotion */
	FAnimMontageInstance * GetRootMotionAnimMontageInstance() const;

	/** True if we are playing Root Motion right now */
	UFUNCTION(BlueprintCallable, Category=Animation)
	bool IsPlayingRootMotion() const;

	/** Sets scale to apply to root motion translation on this Character */
	void SetAnimRootMotionTranslationScale(float InAnimRootMotionTranslationScale = 1.f);

	/** Returns current value of AnimRootMotionScale */
	UFUNCTION(BlueprintCallable, Category=Animation)
	float GetAnimRootMotionTranslationScale() const;

	/**
	 * Called on the actor right before replication occurs.
	 * Only called on Server, and for autonomous proxies if recording a Client Replay.
	 */
	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;

	/**
	 * Called on the actor right before replication occurs.
	 * Called for everyone when recording a Client Replay, including Simulated Proxies.
	 */
	virtual void PreReplicationForReplay(IRepChangedPropertyTracker & ChangedPropertyTracker) override;
};
