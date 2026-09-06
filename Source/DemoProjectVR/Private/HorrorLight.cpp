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

	bool isOutage = endOutageTime <= outageTime;

	DEBUG_PRINT("%s : 消灯チェック / %lf <= %lf", *GetName(), endOutageTime, outageTime);
	if (isOutage)
	{
		LightOutage();
	}
	else
	{
		outageTime += DeltaTime;

		float elapsedRaito = outageTime / restorationTime;
		float ratio = moveCurve->GetFloatValue(elapsedRaito);

		float newIntensity = intensityMax * ratio;
		RectLight->Intensity = newIntensity;

		DEBUG_PRINT("%s : 電気復旧中 / 現在 %lf %% / 光量 %lf", *GetName(), ratio, newIntensity);
	}
}

void AHorrorLight::LightOutage()
{
	outageTime = 0.0f;
	RectLight->Intensity = 0.0f;
	endOutageTime = FMath::FRandRange(intervalMin, intervalMax);
}

