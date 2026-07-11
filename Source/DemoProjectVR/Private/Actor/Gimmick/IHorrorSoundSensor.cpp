// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmick/IHorrorSoundSensor.h"

// Sets default values
AIHorrorSoundSensor::AIHorrorSoundSensor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AIHorrorSoundSensor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AIHorrorSoundSensor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// 何もしない
void AIHorrorSoundSensor::Action()
{
	// デバッグ用
	{
		if (onActive)
		{
			//SoundDisable();
		}
		else
		{
			SoundEnable();
		}
	}
}

// サウンド有効化
void AIHorrorSoundSensor::SoundEnable()
{
	onActive = true;

	DEBUG_PRINT("%s : SoundGimmick -> %s", *GetName(), (onActive ? TEXT("true") : TEXT("false")));

	for (AIHorrorSound* gimmick : gimmickes)
	{
		gimmick->GimmickPlay();

		DEBUG_PRINT("%s : GimmickPlay -> %s", *GetName(), *(gimmick->GetName()));
	}
}

// サウンド無効化
void AIHorrorSoundSensor::SoundDisable()
{
	onActive = false;

	DEBUG_PRINT("%s : SoundGimmick -> %s", *GetName(), (onActive ? TEXT("true") : TEXT("false")));

	for (AIHorrorSound* gimmick : gimmickes)
	{
		gimmick->Action();
	}
}

