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

}

void AILockerDoor::Action()
{

}

