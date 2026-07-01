// Fill out your copyright notice in the Description page of Project Settings.


#include "IElectrical.h"

// Sets default values
AIElectrical::AIElectrical()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AIElectrical::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AIElectrical::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AIElectrical::Action()
{

}

