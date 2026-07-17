// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmick/IItem.h"

// Sets default values
AIItem::AIItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AIItem::BeginPlay()
{
	Super::BeginPlay();
	
	bool isNoneItem = itemType == EItemType::None;
	if (isNoneItem)
	{
		DEBUG_PRINT("%s : ItemType が設定されていません。", *GetName());
	}
}

// Called every frame
void AIItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// アイテムの取得
void AIItem::Action()
{
	APawn* player = GetWorld()->GetFirstPlayerController()->GetPawn();
	UInventoryComponent* inventory = player->FindComponentByClass<UInventoryComponent>();
	if (inventory)
	{
		inventory->inventory.Add(itemType);

		DEBUG_PRINT("%s : Get Item -> %s", *GetName(), ENUM_STR(itemType));

		Destroy();
	}
}

