// Fill out your copyright notice in the Description page of Project Settings.


#include "IElevator.h"

// Sets default values
AIElevator::AIElevator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AIElevator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AIElevator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	actionRunningTime += DeltaTime;

	if (isAction)
	{
		FVector targetLocation = beforeLocation;

		bool isMoveUp = nextFloor > floor;
		if (isMoveUp)
		{
			targetLocation.Z += moveDistance;
		}
		else
		{
			targetLocation.Z -= moveDistance;
		}

		float elapsedRaito = actionRunningTime / moveTime;
		float ratio = moveCurve->GetFloatValue(elapsedRaito);

		
		bool isCompleted = elapsedRaito > 1.0f;
		if (isCompleted)
		{
			floor = nextFloor;
			isAction = false;
			return;
		}

		FVector newLocation = FMath::Lerp(beforeLocation, targetLocation, ratio);

		SetActorLocation(newLocation);
	}
}

void AIElevator::Action()
{
	if (isAction)
	{
		return;
	}

	beforeLocation = GetActorLocation();
	actionRunningTime = 0.0f;
	isAction = true;
}

bool AIElevator::MoveUp()
{
	bool canMove = (floor + 1) <= floorMax;
	canMove &= floor == nextFloor;

	if (canMove)
	{
		nextFloor++;
	}

	return canMove;
}

bool AIElevator::MoveDown()
{
	bool canMove = (floor - 1) >= floorMin;
	canMove &= floor == nextFloor;

	if (canMove)
	{
		nextFloor--;
	}

	return canMove;
}

