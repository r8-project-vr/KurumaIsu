#include "Actor/Monster/Monster.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

#include "Actor/Monster/MonsterAIController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

#pragma region BASE

AMonster::AMonster()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<USoundBase> TransitionAsset(
		TEXT("/Game/Sound/SFX/MonsterFirst.MonsterFirst"));
	static ConstructorHelpers::FObjectFinder<USoundBase> MovementAsset(
		TEXT("/Game/Sound/SFX/MonsterSound.MonsterSound"));
	MovementTransitionSound = TransitionAsset.Object;
	MovementLoopSound = MovementAsset.Object;

	MonsterSoundAttenuation = CreateDefaultSubobject<USoundAttenuation>(TEXT("MonsterSoundAttenuation"));
	FSoundAttenuationSettings& Attenuation = MonsterSoundAttenuation->Attenuation;
	Attenuation.bAttenuate = true;
	Attenuation.bSpatialize = true;
	Attenuation.AttenuationShape = EAttenuationShape::Sphere;
	Attenuation.AttenuationShapeExtents = FVector(100.0f, 0.0f, 0.0f);
	Attenuation.FalloffDistance = 1400.0f;
	Attenuation.DistanceAlgorithm = EAttenuationDistanceModel::Linear;
	Attenuation.bApplyNormalizationToStereoSounds = true;

	MovementAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("MovementAudio"));
	TransitionAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("TransitionAudio"));
	for (UAudioComponent* Audio : {MovementAudio.Get(), TransitionAudio.Get()})
	{
		Audio->SetupAttachment(GetRootComponent());
		Audio->bAutoActivate = false;
		Audio->bAllowSpatialization = true;
		Audio->AttenuationSettings = MonsterSoundAttenuation;
	}
}

void AMonster::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	bAudioMoving = false;
	MovementAudio->Stop();
	TransitionAudio->Stop();
	Super::EndPlay(EndPlayReason);
}

void AMonster::UpdateMovementAudio(float Speed)
{
	// Hysteresis prevents repeated start/stop sounds around the movement threshold.
	const bool bMovingNow = Speed > (bAudioMoving ? 5.0f : 10.0f);
	if (bMovingNow == bAudioMoving)
	{
		return;
	}
	bAudioMoving = bMovingNow;

	TransitionAudio->Stop();
	if (MovementTransitionSound)
	{
		TransitionAudio->SetSound(MovementTransitionSound);
		TransitionAudio->SetAttenuationSettings(MonsterSoundAttenuation);
		TransitionAudio->Play(FMath::Max(0.0f, TransitionSoundStartTime));
	}

	if (bAudioMoving && MovementLoopSound)
	{
		MovementAudio->SetSound(MovementLoopSound);
		MovementAudio->SetAttenuationSettings(MonsterSoundAttenuation);
		MovementAudio->Play(FMath::Max(0.0f, MovementSoundStartTime));
	}
	else
	{
		MovementAudio->Stop();
	}
}

void AMonster::BeginPlay()
{
	Super::BeginPlay();

	// BPMonster already owns the visible mesh as a Blueprint component. Resolve
	// that component at runtime so the procedural motion affects the mesh the
	// player can actually see instead of an empty native component.
	TArray<UStaticMeshComponent*> StaticMeshComponents;
	GetComponents<UStaticMeshComponent>(StaticMeshComponents);

	MonsterMesh = nullptr;
	for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
	{
		if (IsValid(StaticMeshComponent) && StaticMeshComponent->GetStaticMesh())
		{
			MonsterMesh = StaticMeshComponent;
			break;
		}
	}

	if (MonsterMesh)
	{
		// The visual mesh must not block its owner's character movement or carve
		// a moving hole in the navigation mesh. The capsule handles collision.
		MonsterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MonsterMesh->SetCanEverAffectNavigation(false);

		BaseMeshRelativeLocation = MonsterMesh->GetRelativeLocation();
		BaseMeshRelativeRotation = MonsterMesh->GetRelativeRotation();
	}

	TwitchCountdown = FMath::RandRange(
		TwitchIntervalMin,
		TwitchIntervalMax
	);
}

void AMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateMovementAudio(GetVelocity().Size2D());

	if (!MonsterMesh)
	{
		return;
	}

	ProceduralMotionTime += DeltaTime;

	const float Speed = GetVelocity().Size2D();
	const bool bIsMovingNow = Speed > 10.0f;

	const float TargetBlend = bIsMovingNow ? 1.0f : 0.0f;

	MovementBlend = FMath::FInterpTo(
		MovementBlend,
		TargetBlend,
		DeltaTime,
		3.0f
	);

	const float MotionSpeed = FMath::Lerp(
		IdleMotionSpeed,
		MoveMotionSpeed,
		MovementBlend
	);

	const float BobAmount = FMath::Lerp(
		IdleBobAmount,
		MoveBobAmount,
		MovementBlend
	);

	const float SwayAmount = FMath::Lerp(
		IdleSwayAmount,
		MoveSwayAmount,
		MovementBlend
	);

	const float Time = ProceduralMotionTime * MotionSpeed;

	const float Bob =
		FMath::Sin(Time * 1.7f) * BobAmount +
		FMath::Sin(Time * 0.53f + 2.1f) * BobAmount * 0.25f;

	const float SideSway =
		FMath::Sin(Time * 1.1f) * SwayAmount * 0.5f +
		FMath::Sin(Time * 0.61f + 1.3f) * SwayAmount * 0.15f;

	const float Roll =
		FMath::Sin(Time * 1.3f) * SwayAmount +
		FMath::Sin(Time * 0.47f + 0.6f) * SwayAmount * 0.3f;

	const float Pitch =
		FMath::Sin(Time * 0.8f + 1.7f) * SwayAmount * 0.35f +
		FMath::Sin(Time * 0.39f) * SwayAmount * 0.15f;

	const float Yaw =
		FMath::Sin(Time * 0.55f + 0.8f) * SwayAmount * 0.25f;

	FVector NewLocation = BaseMeshRelativeLocation;
	FRotator NewRotation = BaseMeshRelativeRotation;

	NewLocation.Y += SideSway;
	NewLocation.Z += Bob;

	NewRotation.Roll += Roll;
	NewRotation.Pitch += Pitch;
	NewRotation.Yaw += Yaw;

	if (bIsMovingNow)
	{
		if (!bTwitching)
		{
			TwitchCountdown -= DeltaTime;

			if (TwitchCountdown <= 0.0f)
			{
				bTwitching = true;
				TwitchElapsed = 0.0f;

				CurrentTwitchRoll = FMath::RandRange(
					-TwitchRotationAmount,
					TwitchRotationAmount
				);

				CurrentTwitchPitch = FMath::RandRange(
					-TwitchRotationAmount * 0.5f,
					TwitchRotationAmount * 0.5f
				);

				CurrentTwitchYaw = FMath::RandRange(
					-TwitchRotationAmount * 0.4f,
					TwitchRotationAmount * 0.4f
				);

				CurrentTwitchY = FMath::RandRange(
					-TwitchLocationAmount,
					TwitchLocationAmount
				);

				CurrentTwitchZ = FMath::RandRange(
					-TwitchLocationAmount * 0.5f,
					TwitchLocationAmount * 0.5f
				);

				TwitchCountdown = FMath::RandRange(
					TwitchIntervalMin,
					TwitchIntervalMax
				);
			}
		}
	}
	else
	{
		bTwitching = false;
		TwitchElapsed = 0.0f;

		if (TwitchCountdown < 0.5f)
		{
			TwitchCountdown = FMath::RandRange(
				TwitchIntervalMin,
				TwitchIntervalMax
			);
		}
	}

	if (bTwitching)
	{
		TwitchElapsed += DeltaTime;

		const float TwitchAlpha = FMath::Clamp(
			TwitchElapsed / TwitchDuration,
			0.0f,
			1.0f
		);

		const float TwitchPulse =
			FMath::Sin(TwitchAlpha * PI);

		NewLocation.Y += CurrentTwitchY * TwitchPulse;
		NewLocation.Z += CurrentTwitchZ * TwitchPulse;

		NewRotation.Roll += CurrentTwitchRoll * TwitchPulse;
		NewRotation.Pitch += CurrentTwitchPitch * TwitchPulse;
		NewRotation.Yaw += CurrentTwitchYaw * TwitchPulse;

		if (TwitchAlpha >= 1.0f)
		{
			bTwitching = false;
			TwitchElapsed = 0.0f;
		}
	}

	MonsterMesh->SetRelativeLocationAndRotation(
		NewLocation,
		NewRotation
	);
}

#pragma endregion




void AMonster::ActivateCondition(APawn* FollowTarget)
{
	if (!FollowTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: ActivateCondition called without a follow target"), *GetName());
		return;
	}

	bConditionActive = true;
	bTemporarilyPaused = false;

	StartFollowing(FollowTarget);
}


void AMonster::DeactivateCondition()
{
	bConditionActive = false;
	bTemporarilyPaused = false;

	GetWorldTimerManager().ClearTimer(FollowPauseTimerHandle);

	StopFollowing();
}


void AMonster::StartFollowing(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	AMonsterAIController* AIController = Cast<AMonsterAIController>(GetController());
	if (!AIController)
	{
		SpawnDefaultController();
		AIController = Cast<AMonsterAIController>(GetController());
	}

	if (AIController)
	{
		AIController->StartFollowing(Target);
		bIsFollowing = true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: MonsterAIController is not available"), *GetName());
	}
}


void AMonster::StopFollowing()
{
	if (AMonsterAIController* AIController =
		Cast<AMonsterAIController>(GetController()))
	{
		AIController->StopFollowing();
	}

	bIsFollowing = false;
}


void AMonster::PauseFollowingTemporarily()
{
	if (!bConditionActive)
	{
		return;
	}

	if (bTemporarilyPaused)
	{
		return;
	}

	AMonsterAIController* AIController =
		Cast<AMonsterAIController>(GetController());

	if (!AIController)
	{
		return;
	}

	bTemporarilyPaused = true;
	bIsFollowing = false;

	AIController->PauseFollowing();

	GetWorldTimerManager().SetTimer(
		FollowPauseTimerHandle,
		this,
		&AMonster::ResumeFollowing,
		FollowPauseDuration,
		false
	);
}


void AMonster::ResumeFollowing()
{
	if (!bConditionActive)
	{
		return;
	}

	AMonsterAIController* AIController =
		Cast<AMonsterAIController>(GetController());

	if (!AIController)
	{
		return;
	}

	bTemporarilyPaused = false;

	AIController->ResumeFollowing();

	bIsFollowing = true;
}


bool AMonster::IsFollowing() const
{
	return bIsFollowing;
}
