// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GimmickInterface.h"
#include "DebugHelper.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

#include "ISoundTrigger.generated.h"

UCLASS()
class DEMOPROJECTVR_API AISoundTrigger : public AActor, public IGimmickInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AISoundTrigger();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void Action() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> TriggerComponent;

	// トリガーによるサウンド再生かどうか（トリガーではない場合はほかのBPなどからの呼び出し）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	bool isTriggerControll = true;

	UPROPERTY(EditAnywhere, Category = "Setting")
	USoundBase* sound;

	UAudioComponent* audioComponent;

private:
	bool isPlayedSound = false;

};
