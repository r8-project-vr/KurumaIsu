// Fill out your copyright notice in the Description page of Project Settings.
// エレベーター（インターフェース利用）
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GimmickInterface.h"

#include "IElevator.generated.h"

UCLASS()
class DEMOPROJECTVR_API AIElevator : public AActor , public IGimmickInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AIElevator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
		
	UFUNCTION(BlueprintCallable)
	void Action() override;

	UFUNCTION(BlueprintCallable)
	bool MoveUp();

	UFUNCTION(BlueprintCallable)
	bool MoveDown();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	UCurveFloat* moveCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	float moveTime = 3.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	float moveDistance = 200.0f;
	
	// 現在の階数
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting")
	int floor = 1;
private:
	FVector beforeLocation = FVector::Zero();

	bool isAction = false;
	float actionRunningTime = 0.0f;


	// 移動先の階数
	int nextFloor = floor;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	int floorMin = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	int floorMax = 3;
};
