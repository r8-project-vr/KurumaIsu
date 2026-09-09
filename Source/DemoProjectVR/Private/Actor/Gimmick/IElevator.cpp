// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmick/IElevator.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AIElevator::AIElevator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	root = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Root"));
	RootComponent = root;

	door1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door1"));
	door1->SetupAttachment(RootComponent);

	door2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door2"));
	door2->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<USoundBase> DoorSoundAsset(
		TEXT("/Game/Sound/SFX/ElevatorDoor.ElevatorDoor"));
	static ConstructorHelpers::FObjectFinder<USoundBase> EngineSoundAsset(
		TEXT("/Game/Sound/SFX/ElevatorEngine.ElevatorEngine"));
	ElevatorDoorSound = DoorSoundAsset.Object;
	ElevatorEngineSound = EngineSoundAsset.Object;

	ElevatorSoundAttenuation = CreateDefaultSubobject<USoundAttenuation>(TEXT("ElevatorSoundAttenuation"));
	FSoundAttenuationSettings& Attenuation = ElevatorSoundAttenuation->Attenuation;
	Attenuation.bAttenuate = true;
	Attenuation.bSpatialize = true;
	Attenuation.AttenuationShape = EAttenuationShape::Sphere;
	Attenuation.AttenuationShapeExtents = FVector(100.0f, 0.0f, 0.0f);
	Attenuation.FalloffDistance = 1400.0f;
	Attenuation.DistanceAlgorithm = EAttenuationDistanceModel::Linear;
	Attenuation.bApplyNormalizationToStereoSounds = true;

	EngineAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudio"));
	EngineAudio->SetupAttachment(RootComponent);
	EngineAudio->bAutoActivate = false;
	EngineAudio->bAllowSpatialization = true;
	EngineAudio->AttenuationSettings = ElevatorSoundAttenuation;
}

// Called when the game starts or when spawned
void AIElevator::BeginPlay()
{
	Super::BeginPlay();
	
	nextFloor = floor;
	
	//ドアの操作
	beforeDoor1 = door1->GetComponentLocation();
	beforeDoor2 = door2->GetComponentLocation();
	EngineAudio->OnAudioFinished.AddDynamic(this, &AIElevator::UpdateEngineSound);
}

void AIElevator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	EngineAudio->OnAudioFinished.RemoveDynamic(this, &AIElevator::UpdateEngineSound);
	EngineAudio->Stop();
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AIElevator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	actionRunningTime += DeltaTime;

	if (isAction)
	{
		FVector targetLocation = beforeLocation;

		bool isMoveUp = nextFloor > floor;
		if (isMoveUp)
		{
			targetLocation.Z += moveDistance;
		}
		else
		{
			targetLocation.Z -= moveDistance;
		}

		float elapsedRaito = actionRunningTime / moveTime;
		float ratio = moveCurve->GetFloatValue(elapsedRaito);

		bool isCompleted = elapsedRaito > 1.0f;
		if (isCompleted)
		{
			if (isMoveUp) 
			{
				floor++;
			}
			else
			{
				floor--;
			}

			isAction = false;

			bool isMoveComplete = floor == nextFloor;

			DEBUG_PRINT("%s : 現在 %d 階 / 目的地 %d 階", *GetName(), floor, nextFloor);

			if (!isMoveComplete)
			{
				Action();
			}
			else 
			{
				UpdateEngineSound();
				StartDoorAction();
			}
			return;
		}

		FVector newLocation = FMath::Lerp(beforeLocation, targetLocation, ratio);

		SetActorLocation(newLocation);
	}
	if (isDoorAction) 
	{
		float elapsedRaito = actionRunningTime / moveTime;
		float ratio = moveCurve->GetFloatValue(elapsedRaito);

		bool isComp = ratio >= 1.0f;
		if (!isComp)
		{
			FVector newDoor1 = FVector::Zero();
			FVector newDoor2 = FVector::Zero();

			if (!isOpen)
			{
				newDoor1 = FMath::Lerp(beforeDoor1, doorTargetLocation, ratio);
				newDoor2 = FMath::Lerp(beforeDoor2, doorTargetLocation, ratio);
			}
			else
			{
				newDoor1 = FMath::Lerp(doorTargetLocation, beforeDoor1, ratio);
				newDoor2 = FMath::Lerp(doorTargetLocation, beforeDoor2, ratio);
			}

			door1->SetWorldLocation(newDoor1);
			door2->SetWorldLocation(newDoor2);
		}
		else
		{
			if(isOpen)
			{
				isOpen = false;

				if (isDoorOpenOnly)
				{
					isDoorOpenOnly = false;
					isAction = true;
					actionRunningTime = 0.0f;
				}
				else
				{
					isAction = true;
					actionRunningTime = 0.0f;
				}
			}
			else 
			{
				isOpen = true;
			}
			isDoorAction = false;
			UpdateEngineSound();
		}
	}
}

void AIElevator::Action()
{
	if (isAction || isDoorAction)
	{
		DEBUG_PRINT("%s : Action中断 / 実行中により", *GetName());
		return;
	}

	// 移動先が決まってない状態の呼び出しは無効
	bool canMove = floor != nextFloor;
	if (!canMove) 
	{
		if (!isOpen)
		{
			isDoorOpenOnly = true;
			StartDoorAction();
		}

		DEBUG_PRINT("%s : Action中断 / 移動先の未指定により", *GetName());
		return;
	}

	beforeLocation = GetActorLocation();
	actionRunningTime = 0.0f;

	if (isOpen)
	{
		StartDoorAction();
	}
	else
	{
		isAction = true;
		UpdateEngineSound();
	}
}

bool AIElevator::MoveSet(int next)
{
	bool canMove = next <= floorMax;
	canMove &= next >= floorMin;

	if (canMove)
	{
		nextFloor = next;
		// A new call may change the direction during a trip.
		UpdateEngineSound();
	}

	DEBUG_PRINT("%s : 今 %d 階、移動先は %d 階", *GetName(), floor, nextFloor);

	return canMove;
}

void AIElevator::StartDoorAction()
{
	if (isDoorAction || isAction)
	{
		return;
	}

	if (!isOpen && ElevatorDoorSound)
	{
		const FVector DoorLocation = (door1->GetComponentLocation() + door2->GetComponentLocation()) * 0.5f;
		UGameplayStatics::SpawnSoundAttached(ElevatorDoorSound, RootComponent,
			NAME_None, DoorLocation, EAttachLocation::KeepWorldPosition,
			true, 1.0f, 1.0f, FMath::Max(0.0f, DoorSoundStartTime), ElevatorSoundAttenuation);
	}

	FVector door = door1->GetComponentLocation();
	doorTargetLocation = beforeDoor1;
	doorTargetLocation.Z = door.Z;
	doorTargetLocation.X += doorMovePos;
	beforeDoor1.Z = door.Z;
	beforeDoor2.Z = door.Z;
	isDoorAction = true;
	actionRunningTime = 0.0f;
}

void AIElevator::UpdateEngineSound()
{
	const bool bDescending = isAction && nextFloor < floor;
	if (!bDescending || !ElevatorEngineSound)
	{
		EngineAudio->Stop();
		return;
	}

	if (!EngineAudio->IsPlaying())
	{
		EngineAudio->SetSound(ElevatorEngineSound);
		EngineAudio->SetAttenuationSettings(ElevatorSoundAttenuation);
		EngineAudio->Play(FMath::Max(0.0f, EngineSoundStartTime));
	}
}
