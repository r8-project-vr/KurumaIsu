// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemSpawnPoint.h"

// Sets default values
AItemSpawnPoint::AItemSpawnPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	spawnPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnPoint"));
	spawnPoint->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AItemSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AItemSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItemSpawnPoint::ItemGenerate(TSubclassOf<AIItem> item)
{
	GetWorld()->SpawnActor<AIItem>(item, spawnPoint->GetComponentLocation(), FRotator::ZeroRotator);

	FVector temp = spawnPoint->GetComponentLocation();
}