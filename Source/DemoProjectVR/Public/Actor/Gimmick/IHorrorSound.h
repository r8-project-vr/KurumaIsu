// Fill out your copyright notice in the Description page of Project Settings.
// ホラーサウンドギミック用：心電図ギミック・ナースコール（インターフェース利用）

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "DebugHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "GimmickInterface.h"

#include "IHorrorSound.generated.h"

UENUM(BlueprintType)
enum class EHorrorSoundGimmickType : uint8
{
    ElectroCardiogram UMETA(DisplayName="ElectroCardiogram"),
    NurseCall UMETA(DisplayName="NurseCall")
};

UCLASS()
class DEMOPROJECTVR_API AIHorrorSound : public AActor , public IGimmickInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AIHorrorSound();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	void Action() override;

	bool IsPlayingCheck();

	void GimmickPlay();

	bool SetCamera();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, Category = "Setting")
	USoundBase* sound;

	UPROPERTY(EditAnywhere, Category = "Setting")
	UAudioComponent* audioComponent;

	// プレイヤー検知範囲
	UPROPERTY(EditAnywhere, Category = "Setting")
	float detectionRadius = 10.0f;

	// ギミックタイプ：心電図かナースコールか
	UPROPERTY(EditAnywhere, Category = "Setting")
	EHorrorSoundGimmickType gimmickType = EHorrorSoundGimmickType::ElectroCardiogram;

	// サウンド再生速度
	float tempo = 1.0f;

	// サウンドのループ再生フラグ
	bool isPlayLoop = true;

	// カメラ（ギミック作動検知用）
	UCameraComponent* camera;

public:
	// 別のアクターに制御されてギミックを発動するか
	UPROPERTY(EditAnywhere, Category = "Setting")
	bool hasControll = false;
};
