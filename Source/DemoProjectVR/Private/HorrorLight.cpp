 // Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorLight.h"

// Sets default values
AHorrorLight::AHorrorLight()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RectLight == CreateDefaultSubobject<URectLightComponent>(TEXT("RectLight"));
	RootComponent = RectLight;
}

// Called when the game starts or when spawned
void AHorrorLight::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHorrorLight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	bool isOutage = outageTime <= 0.0f;
	if (isOutage)
	{

	}
	else
	{
		outageTime -= DeltaTime;
	}
}

void AHorrorLight::LightOutage()
{
	RectLight->Intensity = 0.0f;
	outageTime = FMath::FRandRange(intervalMin, intervalMax);
}

