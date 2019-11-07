// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "DecoratorComponent.generated.h"

/**
 * 
 */
UCLASS()
class POOLPROJA_API UDecoratorComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UDecoratorComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Decorator)
		float AngularVelocity = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Decorator)
		float Magnitude = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Decorator)
		FVector OscDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Decorator)
		FVector Origin;

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
};
