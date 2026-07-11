// Fill out your copyright notice in the Description page of Project Settings.
// ホラーサウンドギミック用：心電図ギミック・ナースコール（インターフェース利用）

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundAttenuation.h"
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
	
	bool IsDistanceCheck();

	bool SetCamera();

	void GenerateAudioComponent();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void Action() override;

	UFUNCTION()
	void GimmickInitialize();

	UFUNCTION()
	void GimmickPlay();

	UFUNCTION()
	bool IsPlayingCheck();
private:
	// 音源
	UPROPERTY(EditAnywhere, Category = "Setting")
	USoundBase* sound;

	UAudioComponent* audioComponent;

	// サウンド減衰
	UPROPERTY(EditAnywhere, Category = "Setting")
	USoundAttenuation* soundAttenuation;

	// プレイヤー検知範囲
	UPROPERTY(EditAnywhere, Category = "Setting")
	float detectionRadius = 1000.0f;

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
