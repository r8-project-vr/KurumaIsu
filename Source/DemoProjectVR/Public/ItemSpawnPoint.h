// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DebugHelper.h"
#include "Actor/Gimmick/IItem.h"
#include "ItemLight.h"

#include "ItemSpawnPoint.generated.h"

UCLASS()
class DEMOPROJECTVR_API AItemSpawnPoint : public AActor
{
	GENERATED_BODY()
	 
public:	
	// Sets default values for this actor's properties
	AItemSpawnPoint();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void ItemGenerate(TSubclassOf<AIItem> item, TSubclassOf<AItemLight> light);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	TObjectPtr<UStaticMeshComponent> spawnPoint;
};
