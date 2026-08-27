// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmick/IElevator.h"

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
	
	nextFloor = floor;
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
			if (isMoveUp) 
			{
				floor++;
			}
			else
			{
				floor--;
			}

			isAction = false;

			bool isMoveComplete = floor == nextFloor;
			if (!isMoveComplete)
			{
				Action();
			}

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
		DEBUG_PRINT("%s : Action中断 / 実行中により", *GetName());
		return;
	}

	// 移動先が決まってない状態の呼び出しは無効
	bool canMove = floor != nextFloor;
	if (!canMove) 
	{
		DEBUG_PRINT("%s : Action中断 / 移動先の未指定により", *GetName());
		return;
	}

	beforeLocation = GetActorLocation();
	actionRunningTime = 0.0f;
	isAction = true;
}

bool AIElevator::MoveSet(int next)
{
	bool canMove = next <= floorMax;
	canMove &= next >= floorMin;

	if (canMove)
	{
		nextFloor = next;
	}

	DEBUG_PRINT("%s : 今 %d 階、移動先は %d 階", *GetName(), floor, nextFloor);

	return canMove;
}

