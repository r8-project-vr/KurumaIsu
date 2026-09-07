// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemGenerater.h"

// Sets default values
AItemGenerater::AItemGenerater()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AItemGenerater::BeginPlay()
{
	Super::BeginPlay();
	
	ItemGenerate();
}

// Called every frame
void AItemGenerater::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItemGenerater::ItemGenerate()
{
	int spawnPointIndex = FMath::RandRange(0, spawnPoint.Num() - 1);

	spawnPoint[spawnPointIndex]->ItemGenerate(spawnItem, spawnLight);
}