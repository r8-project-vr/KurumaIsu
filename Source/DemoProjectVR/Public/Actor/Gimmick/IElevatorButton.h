// Fill out your copyright notice in the Description page of Project Settings.
// エレベーターのボタン（インターフェース利用）
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GimmickInterface.h"
#include "IElevator.h"

#include "IElevatorButton.generated.h"

UCLASS()
class DEMOPROJECTVR_API AIElevatorButton : public AActor, public IGimmickInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AIElevatorButton();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	void Action() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	AIElevator* elevatorActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	bool isMoveUp = true;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
