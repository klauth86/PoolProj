// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UObject/ConstructorHelpers.h"
#include "Materials/Material.h"

#include "Ball.generated.h"

UCLASS()
class POOLPROJZ_API ABall : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABall();

	UPROPERTY(EditAnywhere, Category = MyPawn)
		float Mass = 1;

private:

	/** The main skeletal mesh associated with this Character (optional sub-object). */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Mesh;
};