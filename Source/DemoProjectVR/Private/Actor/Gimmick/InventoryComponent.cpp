// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmick/InventoryComponent.h"


// Called when the game starts or when spawned
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

// 所持アイテムの表示
void UInventoryComponent::ShowItems()
{
	DEBUG_PRINT("%s : 総アイテム数 -> %d\n", *GetName(), inventory.Num());

	int count = 0;

	for (EItemType& item : inventory)
	{
		DEBUG_PRINT("%d -> %s", count, ENUM_STR(item));
	}
}

