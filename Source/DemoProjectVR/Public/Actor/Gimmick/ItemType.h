// Fill out your copyright notice in the Description page of Project Settings.
// ItemType列挙型宣言用

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None UMETA(DisplayName = "None"),
    ExitKey1 UMETA(DisplayName="ExitKey1"),
    ExitKey2 UMETA(DisplayName="ExitKey2"),
    ExitKey3 UMETA(DisplayName="ExitKey3"),
    ExitKey4 UMETA(DisplayName="ExitKey4"),
    Fuse UMETA(DisplayName="Fuse")
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
