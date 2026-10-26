// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmick/ISoundTrigger.h"

// Sets default values
AISoundTrigger::AISoundTrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	// トリガー作成
	TriggerComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerComponent"));
	RootComponent = TriggerComponent;
	TriggerComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerComponent->SetGenerateOverlapEvents(true);

	// オーディオコンポーネント作成
	audioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
	audioComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AISoundTrigger::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AISoundTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// BPから呼び出し
void AISoundTrigger::Action()
{
	if (isTriggerControll)
	{
		audioComponent->Play();
	}
}