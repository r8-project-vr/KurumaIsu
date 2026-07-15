// Fill out your copyright notice in the Description page of Project Settings.
// アイテムクラス（インターフェース利用）

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GimmickInterface.h"
#include "DebugHelper.h"

#include "IItem.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None UMETA(DisplayName = "None"),
    ExitKey UMETA(DisplayName="ExitKey"),
    Fuse UMETA(DisplayName="Fuse"),
    PhotoFrame UMETA(DisplayName="PhotoFrame")
};

UCLASS()
class DEMOPROJECTVR_API AIItem : public AActor, public IGimmickInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AIItem();
	
	UFUNCTION(BlueprintCallable)
	void Action() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// このアイテムのタイプ
	UPROPERTY(EditAnywhere, Category = "Setting")
	EItemType itemType = EItemType::None;
};
