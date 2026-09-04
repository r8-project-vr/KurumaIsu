// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmick/IElevator.h"

// Sets default values
AIElevator::AIElevator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	root = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Root"));
	RootComponent = root;

	door1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door1"));
	door1->SetupAttachment(RootComponent);

	door2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door2"));
	door2->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AIElevator::BeginPlay()
{
	Super::BeginPlay();
	
	nextFloor = floor;
	
	//ドアの操作
	beforeDoor1 = door1->GetComponentLocation();
	beforeDoor2 = door2->GetComponentLocation();
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

			DEBUG_PRINT("%s : 現在 %d 階 / 目的地 %d 階", *GetName(), floor, nextFloor);

			if (!isMoveComplete)
			{
				Action();
			}
			else 
			{
				StartDoorAction();
			}
			return;
		}

		FVector newLocation = FMath::Lerp(beforeLocation, targetLocation, ratio);

		SetActorLocation(newLocation);
	}
	if (isDoorAction) 
	{
		float elapsedRaito = actionRunningTime / moveTime;
		float ratio = moveCurve->GetFloatValue(elapsedRaito);

		bool isComp = ratio >= 1.0f;
		if (!isComp)
		{
			FVector newDoor1 = FVector::Zero();
			FVector newDoor2 = FVector::Zero();

			if (!isOpen)
			{
				newDoor1 = FMath::Lerp(beforeDoor1, doorTargetLocation, ratio);
				newDoor2 = FMath::Lerp(beforeDoor2, doorTargetLocation, ratio);
			}
			else
			{
				newDoor1 = FMath::Lerp(doorTargetLocation, beforeDoor1, ratio);
				newDoor2 = FMath::Lerp(doorTargetLocation, beforeDoor2, ratio);
			}

			door1->SetWorldLocation(newDoor1);
			door2->SetWorldLocation(newDoor2);
		}
		else
		{
			if(isOpen)
			{
				isOpen = false;

				if (isDoorOpenOnly)
				{
					isAction = true;
					actionRunningTime = 0.0f;
				}
				else
				{
					isDoorOpenOnly = false;
				}
			}
			else 
			{
				isOpen = true;
			}
			isDoorAction = false;
		}
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
		if (!isOpen)
		{
			isDoorOpenOnly = true;
			StartDoorAction();
		}

		DEBUG_PRINT("%s : Action中断 / 移動先の未指定により", *GetName());
		return;
	}

	beforeLocation = GetActorLocation();
	actionRunningTime = 0.0f;

	if (isOpen)
	{
		StartDoorAction();
	}
	else
	{
		isAction = true;
	}
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

void AIElevator::StartDoorAction()
{
	FVector door = door1->GetComponentLocation();
	doorTargetLocation = beforeDoor1;
	doorTargetLocation.Z = door.Z;
	doorTargetLocation.X += doorMovePos;
	beforeDoor1.Z = door.Z;
	beforeDoor2.Z = door.Z;
	isDoorAction = true;
	actionRunningTime = 0.0f;
}