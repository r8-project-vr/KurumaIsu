// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Device/DeviceMoveReader.h"
#include "EngineUtils.h"

#include "MoveInput.generated.h"

UCLASS()
class DEMOPROJECTVR_API AMoveInput : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMoveInput();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ReadDeviceValue();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY()
	ADeviceMoveReader* MoveReader = nullptr;
};
