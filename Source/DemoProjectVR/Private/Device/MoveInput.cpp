// Fill out your copyright notice in the Description page of Project Settings.

#include "Device/MoveInput.h"
#include "DebugHelper.h"

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
	
}

// Called every frame
void AMoveInput::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ReadDeviceValue();
}

void AMoveInput::ReadDeviceValue()
{
	// Move Readerを検索する
	if (!IsValid(MoveReader))
	{
		TActorIterator<ADeviceMoveReader>It(GetWorld());

		if (It)
		{
			MoveReader = *It;

			DEBUG_PRINT("%s : Device Search", *GetName());
		}
	}
}

