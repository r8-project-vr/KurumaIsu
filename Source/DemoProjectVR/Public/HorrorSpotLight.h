// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SpotLightComponent.h"

#include "HorrorSpotLight.generated.h"

UCLASS()
class DEMOPROJECTVR_API AHorrorSpotLight : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHorrorSpotLight();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	USpotLightComponent* spotLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	UCurveFloat* moveCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting")
	float intensityMax = 2400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting")
	float loopTime = 3.0f;

private:
	float elapsedTime = 0.0f;
};
