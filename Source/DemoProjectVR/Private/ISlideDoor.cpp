// Fill out your copyright notice in the Description page of Project Settings.


#include "ISlideDoor.h"
#include "math.h"

// Sets default values
AISlideDoor::AISlideDoor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AISlideDoor::BeginPlay()
{
	Super::BeginPlay();

	if (isLeftOpen)
	{
		moveDistance *= -1.0f;
	}
}

// Called every frame
void AISlideDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	actionRunningTime += DeltaTime;

	if (isAction) 
	{
		FVector targetLocation = GetActorLocation();

		if (isOpen)
		{
			targetLocation.X -= moveDistance;
		}
		else
		{
			targetLocation.X += moveDistance;
		}

		float ratio = actionRunningTime / moveTime;

		bool isCompleted = ratio > 1.0f;
		if (isCompleted)
		{
			if (isOpen)
			{
				isOpen = false;
			}
			else
			{
				isOpen = true;
			}

			isAction = false;
			return;
		}

		FVector newLocation = FMath::Lerp(beforeLocation, targetLocation, ratio);

		SetActorLocation(newLocation);
	}
}

void AISlideDoor::Action()
{
	if (isAction) 
	{
		return;
	}

	beforeLocation = GetActorLocation();
	actionRunningTime = 0.0f;
	isAction = true;
}

