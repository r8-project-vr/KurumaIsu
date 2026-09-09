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
	int nextFloor = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevator|Audio")
	TObjectPtr<USoundBase> ElevatorButtonSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevator|Audio", meta = (ClampMin = "0.0", Units = "s"))
	float ButtonSoundStartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevator|Audio")
	TObjectPtr<USoundAttenuation> ButtonSoundAttenuation;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
