// Fill out your copyright notice in the Description page of Project Settings.
// 電源（インターフェース利用）
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GimmickInterface.h"

#include "IElectrical.generated.h"

UCLASS()
class DEMOPROJECTVR_API AIElectrical : public AActor , public IGimmickInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AIElectrical();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void Action() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
