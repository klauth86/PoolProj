// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "CustomStaticMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class POOLPROJA_API UCustomStaticMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	void CreatePhysicsState();
	void DestroyPhysicsState();
};
