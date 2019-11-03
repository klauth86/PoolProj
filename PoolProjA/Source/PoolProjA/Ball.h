	// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/Material.h"

#include "Ball.generated.h"

UCLASS(Blueprintable)
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

	void BeginPlay() override;
public:
	UFUNCTION(BlueprintCallable)
	static int GetInstanceCount() { return instanceCount; };
	UFUNCTION(BlueprintCallable)
	static void SetInstanceCount(int value) { instanceCount = value; };
};