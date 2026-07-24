// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmick/IGimmickTrigger.h"

// Sets default values
AIGimmickTrigger::AIGimmickTrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = TriggerBox;

    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

// Called when the game starts or when spawned
void AIGimmickTrigger::BeginPlay()
{
	Super::BeginPlay();

	// イベント登録
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AIGimmickTrigger::OnBeginOverlap);
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

// トリガーイベント
void AIGimmickTrigger::OnBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	Action();
}

