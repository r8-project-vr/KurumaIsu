// Fill out your copyright notice in the Description page of Project Settings.
// プレイヤーインベントリコンポーネント

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DebugHelper.h"
#include "Actor/Gimmick/ItemType.h"

#include "InventoryComponent.generated.h"

UCLASS(meta=(BlueprintSpawnableComponent))
class DEMOPROJECTVR_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void ShowItems();
	
	UFUNCTION(BlueprintCallable)
	int CountItems();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	UPROPERTY(BlueprintReadOnly)
	TSet<EItemType> inventory;
};
