#include "Actor/Ryuji/AnimatedImageActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AAnimatedImageActor::AAnimatedImageActor()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ImagePlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ImagePlane"));
	ImagePlane->SetupAttachment(SceneRoot);
	ImagePlane->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	ImagePlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ImagePlane->SetGenerateOverlapEvents(false);
	ImagePlane->SetCastShadow(false);
	ImagePlane->SetReceivesDecals(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(
		TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		ImagePlane->SetStaticMesh(PlaneMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultDisplayMaterial(
		TEXT("/Game/Ryuji/Material/M_AnimatedImage_CPP.M_AnimatedImage_CPP"));
	if (DefaultDisplayMaterial.Succeeded())
	{
		DisplayMaterial = DefaultDisplayMaterial.Object;
		ImagePlane->SetMaterial(0, DisplayMaterial);
	}
}

void AAnimatedImageActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	DynamicMaterial = nullptr;
	EnsureDynamicMaterial();
	const int32 PreviewFrame = FindFirstValidFrame(StartIndex);
	if (PreviewFrame != INDEX_NONE)
	{
		ApplyFrame(PreviewFrame);
	}
}

void AAnimatedImageActor::BeginPlay()
{
	Super::BeginPlay();

	EnsureDynamicMaterial();
	if (bAutoPlay)
	{
		PlayAnimation();
	}
	else
	{
		const int32 InitialFrame = FindFirstValidFrame(StartIndex);
		if (InitialFrame != INDEX_NONE)
		{
			ApplyFrame(InitialFrame);
		}
	}
}

void AAnimatedImageActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAnimation();
	Super::EndPlay(EndPlayReason);
}

void AAnimatedImageActor::PlayAnimation()
{
	StopAnimation();
	EnsureDynamicMaterial();

	const int32 InitialFrame = FindFirstValidFrame(
		CurrentFrameIndex == INDEX_NONE ? StartIndex : CurrentFrameIndex);
	if (InitialFrame == INDEX_NONE || !ApplyFrame(InitialFrame))
	{
		return;
	}

	if (Images.Num() > 1)
	{
		GetWorldTimerManager().SetTimer(
			FrameTimerHandle,
			this,
			&AAnimatedImageActor::AdvanceFrame,
			FMath::Max(FrameInterval, 0.01f),
			true);
	}
}

void AAnimatedImageActor::StopAnimation()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FrameTimerHandle);
	}
}

void AAnimatedImageActor::RestartAnimation()
{
	CurrentFrameIndex = INDEX_NONE;
	PlayAnimation();
}

void AAnimatedImageActor::SetFrame(const int32 FrameIndex)
{
	const int32 ValidFrame = FindFirstValidFrame(FrameIndex);
	if (ValidFrame != INDEX_NONE)
	{
		EnsureDynamicMaterial();
		ApplyFrame(ValidFrame);
	}
}

bool AAnimatedImageActor::IsAnimationPlaying() const
{
	UWorld* World = GetWorld();
	return World && World->GetTimerManager().IsTimerActive(FrameTimerHandle);
}

void AAnimatedImageActor::AdvanceFrame()
{
	if (Images.IsEmpty())
	{
		StopAnimation();
		return;
	}

	const int32 FirstCandidate = CurrentFrameIndex == INDEX_NONE ? StartIndex : CurrentFrameIndex + 1;
	for (int32 Offset = 0; Offset < Images.Num(); ++Offset)
	{
		int32 Candidate = FirstCandidate + Offset;
		if (Candidate >= Images.Num())
		{
			if (!bLoop)
			{
				StopAnimation();
				return;
			}
			Candidate %= Images.Num();
		}

		if (Images[Candidate])
		{
			ApplyFrame(Candidate);
			return;
		}
	}

	StopAnimation();
}

void AAnimatedImageActor::EnsureDynamicMaterial()
{
	if (DynamicMaterial || !ImagePlane)
	{
		return;
	}

	UMaterialInterface* BaseMaterial = DisplayMaterial;
	if (!BaseMaterial)
	{
		BaseMaterial = ImagePlane->GetMaterial(0);
	}

	if (BaseMaterial)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		ImagePlane->SetMaterial(0, DynamicMaterial);
	}
}

bool AAnimatedImageActor::ApplyFrame(const int32 FrameIndex)
{
	if (!Images.IsValidIndex(FrameIndex) || !Images[FrameIndex] || !DynamicMaterial)
	{
		return false;
	}

	DynamicMaterial->SetTextureParameterValue(TextureParameterName, Images[FrameIndex]);
	CurrentFrameIndex = FrameIndex;
	return true;
}

int32 AAnimatedImageActor::FindFirstValidFrame(const int32 PreferredIndex) const
{
	if (Images.IsEmpty())
	{
		return INDEX_NONE;
	}

	const int32 ClampedIndex = FMath::Clamp(PreferredIndex, 0, Images.Num() - 1);
	for (int32 Index = ClampedIndex; Index < Images.Num(); ++Index)
	{
		if (Images[Index])
		{
			return Index;
		}
	}

	for (int32 Index = 0; Index < ClampedIndex; ++Index)
	{
		if (Images[Index])
		{
			return Index;
		}
	}

	return INDEX_NONE;
}
