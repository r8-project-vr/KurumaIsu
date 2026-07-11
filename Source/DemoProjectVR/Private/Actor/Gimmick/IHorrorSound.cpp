// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Gimmick/IHorrorSound.h"

// Sets default values
AIHorrorSound::AIHorrorSound()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
		
	GenerateAudioComponent();
}

// Called when the game starts or when spawned
void AIHorrorSound::BeginPlay()
{
	Super::BeginPlay();

	bool isNull = !audioComponent || !sound;
	if (isNull) 
	{
		DEBUG_PRINT("%s : AudioComponent または Sound が設定されていません。", *GetName());
	}
	else
	{
		// サウンドデータをコンポーネントに渡す
		audioComponent->SetSound(sound);
		
		// サウンド減衰を設定
		if (soundAttenuation)
		{
			audioComponent->AttenuationSettings = soundAttenuation;			
		}
		else
		{
			DEBUG_PRINT("%s : SoundAttenuation が設定されていません。", *GetName());
		}

		// デリゲート登録
		bool isECG = gimmickType == EHorrorSoundGimmickType::ElectroCardiogram;
		if (isECG)
		{
			audioComponent->OnAudioFinished.AddDynamic(
				this,
				&AIHorrorSound::GimmickPlay
			);
		}
	}

	if (!SetCamera())
	{
		DEBUG_PRINT("%s : Player または CameraComponent が見つかりません。", *GetName());
	}

	isPlayLoop = true;
}

// Called every frame
void AIHorrorSound::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// サウンド再生チェック	
	bool isNull = !audioComponent || !sound || !camera;
	if (isNull)
	{
		return;
	}

	FVector camPos = camera->GetComponentLocation();
	FVector thisPos = GetActorLocation();
	float distance = FVector::Distance(camPos, thisPos);
	
	// サウンドの再生速度(ピッチ)を調整
	bool isECG = gimmickType == EHorrorSoundGimmickType::ElectroCardiogram;
	if (isECG)
	{
		float percent = distance / detectionRadius;
		float newTempo = tempo + ( 1.0f - percent );
		audioComponent->SetPitchMultiplier(newTempo);
	}

	bool isActive = IsDistanceCheck();
	isActive &= !IsPlayingCheck();
	isActive &= !hasControll;
	if (isActive)
	{
		GimmickPlay();
	}
}

// 停止
void AIHorrorSound::Action()
{
	isPlayLoop = false;
	audioComponent->Stop();
}

// アクティブチェック
bool AIHorrorSound::IsPlayingCheck()
{
	bool rtv = audioComponent->IsPlaying();
	return rtv;
}

// 動作距離チェック
bool AIHorrorSound::IsDistanceCheck()
{
	FVector camPos = camera->GetComponentLocation();
	FVector thisPos = GetActorLocation();
	float distance = FVector::Distance(camPos, thisPos);
	
	bool rtv = distance <= detectionRadius;

	return rtv;
}

// ギミック初期化
void AIHorrorSound::GimmickInitialize()
{
	isPlayLoop = true;
}

// ギミック再生
void AIHorrorSound::GimmickPlay()
{
	if(IsDistanceCheck() && !IsPlayingCheck() && isPlayLoop)
	{
		audioComponent->Play();
	}
}

// カメラコンポーネントの取得
bool AIHorrorSound::SetCamera()
{
	APawn* playerPawn =
    UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (!playerPawn)
	{
		return false;
	}

	camera = playerPawn->FindComponentByClass<UCameraComponent>();

	if (!camera)
	{
		return false;
	}

	return true;
}

// オーディオコンポーネントの生成
void AIHorrorSound::GenerateAudioComponent()
{
	audioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
	audioComponent->SetupAttachment(RootComponent);
}

