// Fill out your copyright notice in the Description page of Project Settings.
// エレベーター（インターフェース利用）
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GimmickInterface.h"
#include "DebugHelper.h"

#include "IElevator.generated.h"

class UAudioComponent;
class USoundBase;
class USoundAttenuation;

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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
		
	UFUNCTION(BlueprintCallable)
	void Action() override;

	UFUNCTION(BlueprintCallable)
	bool MoveSet(int next);

	UFUNCTION(BlueprintCallable)
	void StartDoorAction();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	UCurveFloat* moveCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	float moveTime = 3.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	float moveDistance = 200.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Setting")
	TObjectPtr<UStaticMeshComponent> door1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	TObjectPtr<UStaticMeshComponent> door2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	float doorMovePos = -94.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevator|Audio")
	TObjectPtr<USoundBase> ElevatorDoorSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevator|Audio", meta = (ClampMin = "0.0", Units = "s"))
	float DoorSoundStartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevator|Audio")
	TObjectPtr<USoundBase> ElevatorEngineSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevator|Audio", meta = (ClampMin = "0.0", Units = "s"))
	float EngineSoundStartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elevator|Audio")
	TObjectPtr<USoundAttenuation> ElevatorSoundAttenuation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Elevator|Audio")
	TObjectPtr<UAudioComponent> EngineAudio;

private:
	// Also restarts a finished clip when a descent lasts longer than the recording.
	UFUNCTION()
	void UpdateEngineSound();

	FVector beforeLocation = FVector::Zero();
	FVector beforeDoor1 = FVector::Zero();
	FVector beforeDoor2 = FVector::Zero();
	FVector doorTargetLocation = FVector::Zero();

	bool isAction = false;
	bool isDoorAction = false;
	bool isOpen = false;
	bool isDoorOpenOnly = false;
	float actionRunningTime = 0.0f;

	// 移動先の階数
	int nextFloor = 0;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	int floorMin = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	int floorMax = 3;

	// 現在の階数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	int floor = 1;
};
