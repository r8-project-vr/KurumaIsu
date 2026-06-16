// Fill out your copyright notice in the Description page of Project Settings.


#include "IElevatorButton.h"

// Sets default values
AIElevatorButton::AIElevatorButton()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AIElevatorButton::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AIElevatorButton::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AIElevatorButton::Action()
{
	UFunction* interface = elevatorActor->FindFunction(TEXT("MoveElevator"));

	if (interface)
	{
		struct 
		{
			bool isUp;
		} Param;

		Param.isUp = isMoveUp;

		elevatorActor->ProcessEvent(interface, &Param);
	}
}

