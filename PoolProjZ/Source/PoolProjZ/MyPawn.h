// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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

#include "MyPawn.generated.h"

UCLASS()
class POOLPROJZ_API AMyPawn : public APawn {
	GENERATED_BODY()

public:
	/** Default UObject constructor. */
	AMyPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region COMPONENTS

#pragma region Mesh
private:
	/** The main skeletal mesh associated with this Character (optional sub-object). */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* Mesh;
public:
	/** Returns Mesh subobject **/
	class USkeletalMeshComponent* GetMesh() const { return Mesh; }

	/** Name of the MeshComponent. Use this name if you want to prevent creation of the component (with ObjectInitializer.DoNotCreateDefaultSubobject). */
	static FName MeshComponentName;
#pragma endregion

#pragma region CharacterMovement
private:
	/** Movement component used for movement logic in various movement modes (walking, falling, etc), containing relevant settings and functions to control movement. */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCharacterMovementComponent* CharacterMovement;
public:
	/** Returns CharacterMovement subobject **/
	class UCharacterMovementComponent* GetCharacterMovement() const { return CharacterMovement; }

	/** Name of the CharacterMovement component. Use this name if you want to use a different class (with ObjectInitializer.SetDefaultSubobjectClass). */
	static FName CharacterMovementComponentName;
#pragma endregion

#pragma region CapsuleComponent
private:
	/** The CapsuleComponent being used for movement collision (by CharacterMovement). Always treated as being vertically aligned in simple collision check functions. */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* CapsuleComponent;
public:
	/** Returns CapsuleComponent subobject **/
	class UCapsuleComponent* GetCapsuleComponent() const { return CapsuleComponent; }

	/** Name of the CapsuleComponent. */
	static FName CapsuleComponentName;
#pragma endregion

#pragma region ArrowComponent

#if WITH_EDITORONLY_DATA
private:
	/** Component shown in the editor only to indicate character facing */
	UPROPERTY()
		UArrowComponent* ArrowComponent;
public:
	/** Returns ArrowComponent subobject **/
	class UArrowComponent* GetArrowComponent() const { return ArrowComponent; }
#endif
#pragma endregion

#pragma endregion

	// --------------------------------------------------------------------

#pragma region PROPS

#pragma region BasedMovement
protected:
	/** Info about our current movement base (object we are standing on). */
	UPROPERTY()
		struct FBasedMovementInfo BasedMovement;
public:
	/** Accessor for BasedMovement */
	FORCEINLINE const FBasedMovementInfo& GetBasedMovement() const { return BasedMovement; }
#pragma endregion

#pragma region ReplicatedMovementMode
protected:
	/** CharacterMovement MovementMode (and custom mode) replicated for simulated proxies. Use CharacterMovementComponent::UnpackNetworkMovementMode() to translate it. */
	UPROPERTY(Replicated)
		uint8 ReplicatedMovementMode;
public:
	/** Returns ReplicatedMovementMode */
	uint8 GetReplicatedMovementMode() const { return ReplicatedMovementMode; }
#pragma endregion

#pragma region ReplicatedBasedMovement
protected:
	/** Replicated version of relative movement. Read-only on simulated proxies! */
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedBasedMovement)
		struct FBasedMovementInfo ReplicatedBasedMovement;
public:
	/** Rep notify for ReplicatedBasedMovement */
	UFUNCTION()
		virtual void OnRep_ReplicatedBasedMovement();

	/** Accessor for ReplicatedBasedMovement */
	FORCEINLINE const FBasedMovementInfo& GetReplicatedBasedMovement() const { return ReplicatedBasedMovement; }
#pragma endregion

#pragma region BaseTranslationOffset
protected:
	/** Saved translation offset of mesh. */
	UPROPERTY()
		FVector BaseTranslationOffset;
public:
	/** Get the saved translation offset of mesh. This is how much extra offset is applied from the center of the capsule. */
	UFUNCTION(BlueprintCallable, Category = Character)
		FVector GetBaseTranslationOffset() const { return BaseTranslationOffset; }
#pragma endregion

#pragma region BaseRotationOffset
protected:
	/** Saved rotation offset of mesh. */
	UPROPERTY()
		FQuat BaseRotationOffset;
public:
	/** Get the saved rotation offset of mesh. This is how much extra rotation is applied from the capsule rotation. */
	virtual FQuat GetBaseRotationOffset() const { return BaseRotationOffset; }

	/** Get the saved rotation offset of mesh. This is how much extra rotation is applied from the capsule rotation. */
	UFUNCTION(BlueprintCallable, Category = Character, meta = (DisplayName = "GetBaseRotationOffset", ScriptName = "GetBaseRotationOffset"))
		FRotator GetBaseRotationOffsetRotator() const { return GetBaseRotationOffset().Rotator(); }
#pragma endregion

#pragma region ReplicatedServerLastTransformUpdateTimeStamp
protected:
	/** CharacterMovement ServerLastTransformUpdateTimeStamp value, replicated to simulated proxies. */
	UPROPERTY(Replicated)
		float ReplicatedServerLastTransformUpdateTimeStamp;
public:
	/** Accessor for ReplicatedServerLastTransformUpdateTimeStamp. */
	FORCEINLINE float GetReplicatedServerLastTransformUpdateTimeStamp() const { return ReplicatedServerLastTransformUpdateTimeStamp; }
#pragma endregion


public:
	/** Disable simulated gravity (set when character encroaches geometry on client, to keep him from falling through floors) */
	UPROPERTY()
		uint32 bSimGravityDisabled : 1;

	UPROPERTY(Transient)
		uint32 bClientCheckEncroachmentOnNetUpdate : 1;

	/** When true, applying updates to network client (replaying saved moves for a locally controlled character) */
	UPROPERTY(Transient)
		uint32 bClientUpdating : 1;

	/** True if Pawn was initially falling when started to replay network moves. */
	UPROPERTY(Transient)
		uint32 bClientWasFalling : 1;

	/** If server disagrees with root motion track position, client has to resimulate root motion from last AckedMove. */
	UPROPERTY(Transient)
		uint32 bClientResimulateRootMotion : 1;

	/** If server disagrees with root motion state, client has to resimulate root motion from last AckedMove. */
	UPROPERTY(Transient)
		uint32 bClientResimulateRootMotionSources : 1;

	/** Disable root motion on the server. When receiving a DualServerMove, where the first move is not root motion and the second is. */
	UPROPERTY(Transient)
		uint32 bServerMoveIgnoreRootMotion : 1;

protected:
	/** Flag that we are receiving replication of the based movement. */
	UPROPERTY()
		bool bInBaseReplication;

	/** Scale to apply to root motion translation on this Character */
	UPROPERTY(Replicated)
		float AnimRootMotionTranslationScale;
#pragma endregion

public:

#pragma region AACTOR INTERFACE
	//~ Begin AActor Interface.
	virtual void ClearCrossLevelReferences() override;
	virtual void PreNetReceive() override;
	virtual void PostNetReceive() override;
	virtual void OnRep_ReplicatedMovement() override;
	virtual void PostNetReceiveLocationAndRotation() override;
	virtual void GetSimpleCollisionCylinder(float& CollisionRadius, float& CollisionHalfHeight) const override;
	virtual UActorComponent* FindComponentByClass(const TSubclassOf<UActorComponent> ComponentClass) const override;
	virtual void TornOff() override;
	//~ End AActor Interface
#pragma endregion

// --------------------------------------------------------------------

	template<class T>
	T* FindComponentByClass() const {
		return AActor::FindComponentByClass<T>();
	}

	//~ Begin INavAgentInterface Interface
	virtual FVector GetNavAgentLocation() const override;
	//~ End INavAgentInterface Interface

// --------------------------------------------------------------------

#pragma region APAWN INTERFACE
//~ Begin APawn Interface.
	virtual void PostInitializeComponents() override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;
	virtual UPrimitiveComponent* GetMovementBase() const override final { return BasedMovement.MovementBase; }
	virtual float GetDefaultHalfHeight() const override;
	virtual void TurnOff() override;
	virtual void Restart() override;
	virtual void PawnClientRestart() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void UpdateNavigationRelevance() override;
	//~ End APawn Interface
#pragma endregion

// --------------------------------------------------------------------

	/** Sets the component the Character is walking on, used by CharacterMovement walking movement to be able to follow dynamic objects. */
	virtual void SetBase(UPrimitiveComponent* NewBase, const FName BoneName = NAME_None, bool bNotifyActor = true);

#pragma region MOVEMENT RPC
	//////////////////////////////////////////////////////////////////////////
// Server RPCs that pass through to CharacterMovement (avoids RPC overhead for components).
// The base RPC function (eg 'ServerMove') is auto-generated for clients to trigger the call to the server function,
// eventually going to the _Implementation function (which we just pass to the CharacterMovementComponent).
//////////////////////////////////////////////////////////////////////////

/** Replicated function sent by client to server - contains client movement and view info. */
	UFUNCTION(unreliable, server, WithValidation)
		void ServerMove(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);
	void ServerMove_Implementation(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);
	bool ServerMove_Validate(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);

	/**
	 * Replicated function sent by client to server. Saves bandwidth over ServerMove() by implying that ClientMovementBase and ClientBaseBoneName are null.
	 * Passes through to CharacterMovement->ServerMove_Implementation() with null base params.
	 */
	UFUNCTION(unreliable, server, WithValidation)
		void ServerMoveNoBase(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode);
	void ServerMoveNoBase_Implementation(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode);
	bool ServerMoveNoBase_Validate(float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 CompressedMoveFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode);

	/** Replicated function sent by client to server - contains client movement and view info for two moves. */
	UFUNCTION(unreliable, server, WithValidation)
		void ServerMoveDual(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);
	void ServerMoveDual_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);
	bool ServerMoveDual_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);

	/** Replicated function sent by client to server - contains client movement and view info for two moves. */
	UFUNCTION(unreliable, server, WithValidation)
		void ServerMoveDualNoBase(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode);
	void ServerMoveDualNoBase_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode);
	bool ServerMoveDualNoBase_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, uint8 ClientMovementMode);

	/** Replicated function sent by client to server - contains client movement and view info for two moves. First move is non root motion, second is root motion. */
	UFUNCTION(unreliable, server, WithValidation)
		void ServerMoveDualHybridRootMotion(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);
	void ServerMoveDualHybridRootMotion_Implementation(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);
	bool ServerMoveDualHybridRootMotion_Validate(float TimeStamp0, FVector_NetQuantize10 InAccel0, uint8 PendingFlags, uint32 View0, float TimeStamp, FVector_NetQuantize10 InAccel, FVector_NetQuantize100 ClientLoc, uint8 NewFlags, uint8 ClientRoll, uint32 View, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode);

	/* Resending an (important) old move. Process it if not already processed. */
	UFUNCTION(unreliable, server, WithValidation)
		void ServerMoveOld(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags);
	void ServerMoveOld_Implementation(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags);
	bool ServerMoveOld_Validate(float OldTimeStamp, FVector_NetQuantize10 OldAccel, uint8 OldMoveFlags);

	//////////////////////////////////////////////////////////////////////////
	// Client RPCS that pass through to CharacterMovement (avoids RPC overhead for components).
	//////////////////////////////////////////////////////////////////////////

	/** If no client adjustment is needed after processing received ServerMove(), ack the good move so client can remove it from SavedMoves */
	UFUNCTION(unreliable, client)
		void ClientAckGoodMove(float TimeStamp);
	void ClientAckGoodMove_Implementation(float TimeStamp);

	/** Replicate position correction to client, associated with a timestamped servermove.  Client will replay subsequent moves after applying adjustment.  */
	UFUNCTION(unreliable, client)
		void ClientAdjustPosition(float TimeStamp, FVector NewLoc, FVector NewVel, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode);
	void ClientAdjustPosition_Implementation(float TimeStamp, FVector NewLoc, FVector NewVel, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode);

	/* Bandwidth saving version, when velocity is zeroed */
	UFUNCTION(unreliable, client)
		void ClientVeryShortAdjustPosition(float TimeStamp, FVector NewLoc, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode);
	void ClientVeryShortAdjustPosition_Implementation(float TimeStamp, FVector NewLoc, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode);

	/** Replicate position correction to client when using root motion for movement. (animation root motion specific) */
	UFUNCTION(unreliable, client)
		void ClientAdjustRootMotionPosition(float TimeStamp, float ServerMontageTrackPosition, FVector ServerLoc, FVector_NetQuantizeNormal ServerRotation, float ServerVelZ, UPrimitiveComponent* ServerBase, FName ServerBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode);
	void ClientAdjustRootMotionPosition_Implementation(float TimeStamp, float ServerMontageTrackPosition, FVector ServerLoc, FVector_NetQuantizeNormal ServerRotation, float ServerVelZ, UPrimitiveComponent* ServerBase, FName ServerBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode);

	/** Replicate root motion source correction to client when using root motion for movement. */
	UFUNCTION(unreliable, client)
		void ClientAdjustRootMotionSourcePosition(float TimeStamp, FRootMotionSourceGroup ServerRootMotion, bool bHasAnimRootMotion, float ServerMontageTrackPosition, FVector ServerLoc, FVector_NetQuantizeNormal ServerRotation, float ServerVelZ, UPrimitiveComponent* ServerBase, FName ServerBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode);
	void ClientAdjustRootMotionSourcePosition_Implementation(float TimeStamp, FRootMotionSourceGroup ServerRootMotion, bool bHasAnimRootMotion, float ServerMontageTrackPosition, FVector ServerLoc, FVector_NetQuantizeNormal ServerRotation, float ServerVelZ, UPrimitiveComponent* ServerBase, FName ServerBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode);
#pragma endregion

	/**
	* True if we are playing Root Motion right now, through a Montage with RootMotionMode == ERootMotionMode::RootMotionFromMontagesOnly.
	* This means code path for networked root motion is enabled.
	*/
	UFUNCTION(BlueprintCallable, Category = Animation)
		bool IsPlayingNetworkedRootMotionMontage() const;

	/**
	* Called on client after position update is received to respond to the new location and rotation.
	* Actual change in location is expected to occur in CharacterMovement->SmoothCorrection(), after which this occurs.
	* Default behavior is to check for penetration in a blocking object if bClientCheckEncroachmentOnNetUpdate is enabled, and set bSimGravityDisabled=true if so.
	*/
	virtual void OnUpdateSimulatedPosition(const FVector& OldLocation, const FQuat& OldRotation);

	/**
	 * Cache mesh offset from capsule. This is used as the target for network smoothing interpolation, when the mesh is offset with lagged smoothing.
	 * This is automatically called during initialization; call this at runtime if you intend to change the default mesh offset from the capsule.
	 * @see GetBaseTranslationOffset(), GetBaseRotationOffset()
	 */
	UFUNCTION(BlueprintCallable, Category = Character)
		virtual void CacheInitialMeshOffset(FVector MeshRelativeLocation, FRotator MeshRelativeRotation);

	/** Set whether this actor's movement replicates to network clients. */
	UFUNCTION(BlueprintCallable, Category = Replication)
		virtual void SetReplicateMovement(bool bInReplicateMovement) override;

	/** Save a new relative location in BasedMovement and a new rotation with is either relative or absolute. */
	void SaveRelativeBasedMovement(const FVector& NewRelativeLocation, const FRotator& NewRotation, bool bRelativeRotation);

	/** Play Animation Montage on the character mesh **/
	UFUNCTION(BlueprintCallable, Category = Animation)
		virtual float PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None);

	/** Stop Animation Montage. If nullptr, it will stop what's currently active. The Blend Out Time is taken from the montage asset that is being stopped. **/
	UFUNCTION(BlueprintCallable, Category = Animation)
		virtual void StopAnimMontage(class UAnimMontage* AnimMontage = nullptr);

	/** Return current playing Montage **/
	UFUNCTION(BlueprintCallable, Category = Animation)
		class UAnimMontage* GetCurrentMontage();

	/**
	* Called when pawn's movement is blocked
	* @param Impact describes the blocking hit.
	*/
	virtual void MoveBlockedBy(const FHitResult& Impact) {};

	/** Multicast delegate for MovementMode changing. */
	UPROPERTY(BlueprintAssignable, Category = Character)
		FMovementModeChangedSignature MovementModeChangedDelegate;

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
	UPROPERTY(ReplicatedUsing = OnRep_RootMotion)
		struct FRepRootMotionMontage RepRootMotion;

	/** Handles replicated root motion properties on simulated proxies and position correction. */
	UFUNCTION()
		void OnRep_RootMotion();

	/** Position fix up for Simulated Proxies playing Root Motion */
	void SimulatedRootMotionPositionFixup(float DeltaSeconds);

	/** Get FAnimMontageInstance playing RootMotion */
	FAnimMontageInstance * GetRootMotionAnimMontageInstance() const;

	/** True if we are playing Root Motion right now */
	UFUNCTION(BlueprintCallable, Category = Animation)
		bool IsPlayingRootMotion() const;

	/** Sets scale to apply to root motion translation on this Character */
	void SetAnimRootMotionTranslationScale(float InAnimRootMotionTranslationScale = 1.f);

	/** Returns current value of AnimRootMotionScale */
	UFUNCTION(BlueprintCallable, Category = Animation)
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

protected:
	/** Event called after actor's base changes (if SetBase was requested to notify us with bNotifyPawn). */
	virtual void BaseChange();
};