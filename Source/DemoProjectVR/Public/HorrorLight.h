// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/RectLightComponent.h"

#include "HorrorLight.generated.h"

UCLASS()
class DEMOPROJECTVR_API AHorrorLight : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AHorrorLight();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	URectLightComponent* RectLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	UCurveFloat* moveCurve;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	float intensityMax = 2400.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	float restorationTime = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	float intervalMin = 1.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	float intervalMax = 7.0f;
private:
	float outageTime = 0.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void LightOutage();
};
