// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmick/ISlideDoor.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"
#include "math.h"

// Sets default values
AISlideDoor::AISlideDoor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<USoundBase> DoorSoundAsset(
		TEXT("/Game/Sound/SFX/DoorOpen.DoorOpen"));
	DoorMovementSound = DoorSoundAsset.Object;

	DoorSoundAttenuation = CreateDefaultSubobject<USoundAttenuation>(TEXT("DoorSoundAttenuation"));
	FSoundAttenuationSettings& Attenuation = DoorSoundAttenuation->Attenuation;
	Attenuation.bAttenuate = true;
	Attenuation.bSpatialize = true;
	Attenuation.SpatializationAlgorithm = SPATIALIZATION_Default;
	Attenuation.AttenuationShape = EAttenuationShape::Sphere;
	Attenuation.AttenuationShapeExtents = FVector(100.0f, 0.0f, 0.0f);
	Attenuation.FalloffDistance = 1400.0f;
	Attenuation.DistanceAlgorithm = EAttenuationDistanceModel::Linear;
}

// Called when the game starts or when spawned
void AISlideDoor::BeginPlay()
{
	Super::BeginPlay();

	if (isLeftOpen)
	{
		moveDistance *= -1.0f;
	}

	// 移動量をスケール比に調整
	moveDistance *= scaleAmount;
}

// Called every frame
void AISlideDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	actionRunningTime += DeltaTime;

	if (isAction) 
	{
		FVector targetLocation = beforeLocation;

		if (isAxisChanged) 
		{
			if (isOpen)
			{
				targetLocation.Y -= moveDistance;
			}
			else
			{
				targetLocation.Y += moveDistance;
			}
		}
		else
		{
			if (isOpen)
			{
				targetLocation.X -= moveDistance;
			}
			else
			{
				targetLocation.X += moveDistance;
			}
		}

		float ratio = actionRunningTime / moveTime;

		bool isCompleted = ratio > 1.0f;
		if (isCompleted)
		{
			if (isOpen)
			{
				isOpen = false;
			}
			else
			{
				isOpen = true;
			}

			isAction = false;
			return;
		}

		FVector newLocation = FMath::Lerp(beforeLocation, targetLocation, ratio);

		SetActorLocation(newLocation);
	}
}

void AISlideDoor::Action()
{
	if(!isDirectAction)
	{
		StartMovement(true);
	}
}

void AISlideDoor::StartMovement(bool bPlaySound)
{
	if (isAction) 
	{
		return;
	}

	beforeLocation = GetActorLocation();
	actionRunningTime = 0.0f;
	isAction = true;

	if (bPlaySound && DoorMovementSound && GetRootComponent())
	{
		UGameplayStatics::SpawnSoundAttached(DoorMovementSound, GetRootComponent(),
			NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset,
			true, 1.0f, 1.0f, FMath::Max(0.0f, DoorSoundStartTime), DoorSoundAttenuation);
	}

	if (IsValid(doubleDoor))
	{
		// A linked pair is one operation; do not layer two identical sounds.
		doubleDoor->StartMovement(false);
	}
}

