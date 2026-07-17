// Fill out your copyright notice in the Description page of Project Settings.
// ItemType列挙型宣言用

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None UMETA(DisplayName = "None"),
    ExitKey UMETA(DisplayName="ExitKey"),
    Fuse UMETA(DisplayName="Fuse"),
    PhotoFrame UMETA(DisplayName="PhotoFrame")
};

/**
 * 
 */
class DEMOPROJECTVR_API ItemType
{
public:
	ItemType();
	~ItemType();
};
