// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorSpotLight.h"

// Sets default values
AHorrorSpotLight::AHorrorSpotLight()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	spotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
	RootComponent = spotLight;
}

// Called when the game starts or when spawned
void AHorrorSpotLight::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHorrorSpotLight::Tick(float DeltaTime)
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
	spotLight->SetIntensity(newIntensity);
}

