// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UObject/ConstructorHelpers.h"
#include "Materials/Material.h"

#include "PoolProjAGameModeBase.h"
#include "Ball.h"

#include "Hole.generated.h"

UCLASS()
class POOLPROJA_API AHole : public AActor {
	GENERATED_BODY()

public:

	AHole();

	UPROPERTY(Category = Hole, EditAnywhere)
		int MyOwnerControllerId = -1;

	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Mesh;

	UFUNCTION()
		void CallOnActorBeginOverlap(AActor* actor, AActor* otherActor);
};