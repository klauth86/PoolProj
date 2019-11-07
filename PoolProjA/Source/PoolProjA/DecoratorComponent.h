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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Decorator)
		float AngularVelocity = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Decorator)
		FVector Magnitude;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Decorator)
		FVector TargetOffset;

	virtual void OnRegister() override;
	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void ApplyWorldOffset(const FVector& InOffset, bool bWorldShift) override;

protected:
	virtual void UpdateDesiredArmLocation(float DeltaTime);
};
