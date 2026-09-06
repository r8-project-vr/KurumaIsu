 // Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorLight.h"

// Sets default values
AHorrorLight::AHorrorLight()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RectLight = CreateDefaultSubobject<URectLightComponent>(TEXT("RectLight"));
	RootComponent = RectLight;
}

// Called when the game starts or when spawned
void AHorrorLight::BeginPlay()
{
	Super::BeginPlay();
	
	LightOutage();
}

// Called every frame
void AHorrorLight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	bool isOutage = endOutageTime < elapsedTime;

	if (isOutage)
	{
		LightOutage();
	}
	else
	{
		elapsedTime += DeltaTime;

		float elapsedRaito = elapsedTime / restorationTime;

		float ratio = moveCurve[index]->GetFloatValue(elapsedRaito);

		float newIntensity = intensityMax * ratio;
		RectLight->SetIntensity(newIntensity);
	}
}

void AHorrorLight::LightOutage()
{
	elapsedTime = 0.0f;
	RectLight->SetIntensity(0.0f);
	index = FMath::RandRange(0, moveCurve.Num() - 1);
	endOutageTime = FMath::FRandRange(intervalMin, intervalMax);
}

