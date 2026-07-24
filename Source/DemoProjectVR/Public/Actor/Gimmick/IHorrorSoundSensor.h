// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GimmickInterface.h"
#include "DebugHelper.h"
#include "Engine/TriggerBox.h"
#include "GameFramework/Character.h"
#include "IHorrorSound.h"

#include "IHorrorSoundSensor.generated.h"

UCLASS()
class DEMOPROJECTVR_API AIHorrorSoundSensor : public AActor, public IGimmickInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AIHorrorSoundSensor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	void Action() override;

	UFUNCTION(BlueprintCallable)
	void SoundPlayLoop();

	UFUNCTION(BlueprintCallable)
	void SoundEnable();

	UFUNCTION(BlueprintCallable)
	void SoundDisable();

	UFUNCTION()
	void OnTriggerEnter(AActor* overlappedActor, AActor* otherActor);
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// ギミックが動作中か
	bool onActive = false;

public:
	// このアクターが管理するギミックリスト
	UPROPERTY(EditAnywhere, Category = "Setting")
	TArray<AIHorrorSound*> gimmickes;
};
