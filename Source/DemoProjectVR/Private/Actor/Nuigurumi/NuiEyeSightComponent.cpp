// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Nuigurumi/NuiEyeSightComponent.h"
#include "Engine/EngineTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UNuiEyeSightComponent::UNuiEyeSightComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UNuiEyeSightComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UNuiEyeSightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	DetectObject();
	// ...
}

void UNuiEyeSightComponent::DetectObject()
{
	AActor* Owner = GetOwner();

	if (Owner == nullptr)
	{
		SetDetectedActor(nullptr);
		return;
	}

	// 親Actorの情報
	FVector OwnerLocation = Owner->GetActorLocation();
	FVector Forward = Owner->GetActorForwardVector();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Owner);

	TArray<AActor*> OutActors;

	bool bOverlap = UKismetSystemLibrary::SphereOverlapActors(
		this,
		OwnerLocation,
		SightDistance,
		ObjectTypes,
		nullptr,
		ActorsToIgnore,
		OutActors
	);

	if (bOverlap == false)
	{
		SetDetectedActor(nullptr);
		return;
	}

	float HalfAngle = SightAngle * 0.5f;
	float AngleLimit = FMath::Cos(FMath::DegreesToRadians(HalfAngle));

	AActor* ClosestActor = nullptr;
	float ClosestDistance = SightDistance;

	for (AActor* TargetActor : OutActors)
	{
		if (TargetActor == nullptr)
		{
			continue;
		}

		if (TargetActor->ActorHasTag(TargetTag) == false)
		{
			continue;
		}

		FVector ToTarget = TargetActor->GetActorLocation() - OwnerLocation;
		float Distance = ToTarget.Size();

		if (Distance <= 0.0f)
		{
			continue;
		}

		ToTarget.Normalize();

		float Dot = FVector::DotProduct(Forward, ToTarget);

		if (Dot >= AngleLimit)
		{
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestActor = TargetActor;
			}
		}
	}

	SetDetectedActor(ClosestActor);

	// デバック用線
	DrawDebugCone(
		GetWorld(), OwnerLocation, Forward, SightDistance,
		FMath::DegreesToRadians(SightAngle * 0.5f),
		FMath::DegreesToRadians(SightAngle * 0.5f),
		24, FColor::Green, false, 0.0f, 0, 1.0f);

	// デバック用（当たったものを確認します）
	if (DetectedActor != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(TEXT("Detected: %s"), *DetectedActor->GetName()));
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, TEXT("Detected: None"));
	}
}

void UNuiEyeSightComponent::SetDetectedActor(AActor* NewActor)
{
	bool bOldHasDetectedActor = HasDetectedActor();

	DetectedActor = NewActor;

	bool bNewHasDetectedActor = HasDetectedActor();

	if (bOldHasDetectedActor != bNewHasDetectedActor)
	{
		OnDetectionChanged.Broadcast(bNewHasDetectedActor);
	}
}

bool UNuiEyeSightComponent::HasDetectedActor() const
{
	return IsValid(DetectedActor);
}

