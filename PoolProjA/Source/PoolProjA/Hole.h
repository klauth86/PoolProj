// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UObject/ConstructorHelpers.h"
#include "Materials/Material.h"

#include "Hole.generated.h"

UCLASS()
class POOLPROJA_API AHole : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AHole();

private:
	/** The main skeletal mesh associated with this Character (optional sub-object). */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Mesh;

	UFUNCTION()
		void CallOnActorBeginOverlap(AActor* actor, AActor* otherActor);
};