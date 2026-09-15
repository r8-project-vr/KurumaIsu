// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"

#include "MoveInput.generated.h"

class ADeviceMoveReader;

UCLASS()
class DEMOPROJECTVR_API AMoveInput : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMoveInput();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure)
	float GetDeviceValue();

	UFUNCTION()
	void SetValue(float newRps);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting")
	ADeviceMoveReader* MoveReader = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	float DeviceRPS = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting")
	bool IsRight = false;

private:

	const float DecayFactor = 10.0f;

	float DesiredRPS;
	float ActualRPS;

	float SustainElapsed;
	const float SustainTimeout = 1.0f;
	const float TimeoutDecay = 8.0f;
};
