// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemLight.h"

// Sets default values
AItemLight::AItemLight()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
	RootComponent = PointLight;
}

// Called when the game starts or when spawned
void AItemLight::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AItemLight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	elapsedTime += DeltaTime;
	
	bool isLooped = elapsedTime > loopTime;
	if (isLooped)
	{
		elapsedTime -= loopTime;
	}

	float elapsedRaito = elapsedTime / loopTime;

	float ratio = moveCurve->GetFloatValue(elapsedRaito);

	float newIntensity = intensityMax * ratio;
	PointLight->SetIntensity(newIntensity);
}

