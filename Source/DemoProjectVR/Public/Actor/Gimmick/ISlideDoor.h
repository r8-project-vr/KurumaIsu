// Fill out your copyright notice in the Description page of Project Settings.
// スライドドア（インターフェース利用）
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GimmickInterface.h"

#include "ISlideDoor.generated.h"

class USoundBase;
class USoundAttenuation;

UCLASS()
class DEMOPROJECTVR_API AISlideDoor : public AActor, public IGimmickInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AISlideDoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void Action() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	float moveTime = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	float moveDistance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	float scaleAmount = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	bool isLeftOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	bool isAxisChanged = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	AISlideDoor* doubleDoor;

	// Played once when opening or closing starts, at the moving door's location.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
	TObjectPtr<USoundBase> DoorMovementSound;

	// Skip the beginning of the sound; this does not delay the door action.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio", meta = (ClampMin = "0.0", Units = "s"))
	float DoorSoundStartTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
	TObjectPtr<USoundAttenuation> DoorSoundAttenuation;

private:
	void StartMovement(bool bPlaySound);

	FVector beforeLocation = FVector::Zero();

	bool isAction = false;
	bool isOpen = false;
	float actionRunningTime = 0.0f;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
