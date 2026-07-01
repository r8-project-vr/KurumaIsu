// Fill out your copyright notice in the Description page of Project Settings.


#include "ILockerDoor.h"

// Sets default values
AILockerDoor::AILockerDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AILockerDoor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AILockerDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	actionRunningTime += DeltaTime;

	if (isAction) 
	{
		bool isCompleted = actionRunningTime >= moveTime;
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

		float angle = moveAngle / moveTime;
		if (isOpen)
		{
			angle *= -1.0f;
		}

		AddActorLocalRotation(FRotator(0.0f, angle * DeltaTime, 0.0f));
	}
}

void AILockerDoor::Action()
{
	if (isAction) 
	{
		return;
	}

	actionRunningTime = 0.0f;
	isAction = true;
}

