// Fill out your copyright notice in the Description page of Project Settings.

#include "Device/MoveInput.h"
#include "DebugHelper.h"
#include "Device/DeviceMoveReader.h"


// Sets default values
AMoveInput::AMoveInput()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMoveInput::BeginPlay()
{
	Super::BeginPlay();
	
	MoveReader->SetMoveInput(this);
	IsRight = MoveReader->bInputInversion;
}

// Called every frame
void AMoveInput::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GetDeviceValue();
}

float AMoveInput::GetDeviceValue()
{
	if (MoveReader == nullptr)
	{
		DEBUG_PRINT("Move Reader NOT Setting");
		return 0.0f;
	}

	return DeviceRPS;
}

void AMoveInput::SetValue(float newRps)
{
	DeviceRPS = newRps;

	DEBUG_PRINT("%s : RPS = %lf", *GetName(), DeviceRPS);
}

