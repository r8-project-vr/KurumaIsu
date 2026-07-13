// Fill out your copyright notice in the Description page of Project Settings.
// ロッカーのドア（インターフェース利用）
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GimmickInterface.h"

#include "ILockerDoor.generated.h"

UCLASS()
class DEMOPROJECTVR_API AILockerDoor : public AActor , public IGimmickInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AILockerDoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	void Action() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	float moveTime = 1.25f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	float moveAngle = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	bool isLeftOpen = false;

private:
	bool isAction = false;
	bool isOpen = false;
	float actionRunningTime = 0.0f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
