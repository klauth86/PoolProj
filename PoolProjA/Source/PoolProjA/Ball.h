	// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/Material.h"

#include "Ball.generated.h"

UCLASS()
class POOLPROJA_API ABall : public AActor {
	GENERATED_BODY()

public:
	ABall();
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = MyPawn)
		float Mass = 1;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Mesh;

protected:
	static int instanceCount;
public:
	static int InstanceCount() { return instanceCount; };
	static void ResetInstanceCount() { instanceCount = 0; };
};