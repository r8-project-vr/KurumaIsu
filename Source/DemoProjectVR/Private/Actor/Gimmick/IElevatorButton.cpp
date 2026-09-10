// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmick/IElevatorButton.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AIElevatorButton::AIElevatorButton()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<USoundBase> ButtonSoundAsset(
		TEXT("/Game/Sound/SFX/ElevatorButton.ElevatorButton"));
	ElevatorButtonSound = ButtonSoundAsset.Object;
	ButtonSoundAttenuation = CreateDefaultSubobject<USoundAttenuation>(TEXT("ButtonSoundAttenuation"));
	FSoundAttenuationSettings& Attenuation = ButtonSoundAttenuation->Attenuation;
	Attenuation.bAttenuate = true;
	Attenuation.bSpatialize = true;
	Attenuation.AttenuationShape = EAttenuationShape::Sphere;
	Attenuation.AttenuationShapeExtents = FVector(100.0f, 0.0f, 0.0f);
	Attenuation.FalloffDistance = 1400.0f;
	Attenuation.DistanceAlgorithm = EAttenuationDistanceModel::Linear;
	Attenuation.bApplyNormalizationToStereoSounds = true;
}

// Called when the game starts or when spawned
void AIElevatorButton::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AIElevatorButton::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AIElevatorButton::Action()
{
	if (!IsValid(elevatorActor))
	{
		return;
	}

	// BPのメソッドを検索
	UFunction* interface = elevatorActor->FindFunction(TEXT("MoveElevator"));

	if (interface)
	{
		if (ElevatorButtonSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ElevatorButtonSound, GetActorLocation(),
				1.0f, 1.0f, FMath::Max(0.0f, ButtonSoundStartTime), ButtonSoundAttenuation);
		}

		struct 
		{
			int next;
		} Param;

		Param.next = nextFloor;

		elevatorActor->ProcessEvent(interface, &Param);
	}
}

