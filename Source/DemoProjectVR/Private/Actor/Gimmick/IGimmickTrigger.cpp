// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmick/IGimmickTrigger.h"

// Sets default values
AIGimmickTrigger::AIGimmickTrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AIGimmickTrigger::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AIGimmickTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// 管理しているギミックの実行呼び出し
void AIGimmickTrigger::Action()
{
	for (const TScriptInterface<IGimmickInterface>& gimmick  : gimmickes)
	{
		if (gimmick)
		{
			gimmick->Action();
		}
	}
}

