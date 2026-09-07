// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DebugHelper.h"
#include "Actor/Gimmick/GimmickInterface.h"
#include "ItemSpawnPoint.h"

#include "ItemGenerater.generated.h"

UCLASS()
class DEMOPROJECTVR_API AItemGenerater : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemGenerater();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void ItemGenerate();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	TSubclassOf<AIItem> spawnItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	TArray<AItemSpawnPoint*> spawnPoint;
};
