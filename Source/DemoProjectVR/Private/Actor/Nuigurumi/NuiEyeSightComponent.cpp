// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Nuigurumi/NuiEyeSightComponent.h"

#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "Engine/EngineTypes.h"
#include "Actor/Gimmick/GimmickInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Actor/Gimmick/IElevator.h"

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

	// Follow/IMU updates the owner transform in its tick. Query the same pose that is rendered.
	if (AActor* Owner = GetOwner())
	{
		AddTickPrerequisiteActor(Owner);
	}
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

	const FVector OwnerLocation = Owner->GetActorLocation();
	const FVector Forward = Owner->GetActorRotation().RotateVector(FRotator(SightPitchOffset, 0.0f, 0.0f).Vector());
	const float Range = FMath::Max(0.0f, SightDistance);
	const float HalfAngleRadians = FMath::DegreesToRadians(FMath::Clamp(SightAngle, 0.0f, 180.0f) * 0.5f);
	const float AngleLimit = FMath::Cos(HalfAngleRadians);

	//if (bShowDetectionDebug)
	//{
	//	DrawDebugCone(GetWorld(), OwnerLocation, Forward, Range,
	//		HalfAngleRadians, HalfAngleRadians, 32, FColor::Green, false, 0.0f, 0, 1.0f);
	//}

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Owner);

	TArray<UPrimitiveComponent*> OutComponents;

	UKismetSystemLibrary::SphereOverlapComponents(
		this,
		OwnerLocation,
		Range,
		ObjectTypes,
		nullptr,
		ActorsToIgnore,
		OutComponents
	);

	AActor* ClosestActor = nullptr;
	double ClosestDistance = TNumericLimits<double>::Max();
	FVector DetectedPoint = OwnerLocation;

	for (UPrimitiveComponent* TargetComponent : OutComponents)
	{
		AActor* TargetActor = IsValid(TargetComponent) ? TargetComponent->GetOwner() : nullptr;
		if (!IsValid(TargetActor))
		{
			continue;
		}

		if (TargetActor->IsA<AIElevator>())
		{
			continue;
		}

		const bool bMatchesTag = bDetectTargetTag
			&& TargetTag != NAME_None
			&& TargetActor->ActorHasTag(TargetTag);
		const bool bMatchesGimmick = bDetectGimmickInterface
			&& TargetActor->GetClass()->ImplementsInterface(UGimmickInterface::StaticClass());

		if (!bMatchesTag && !bMatchesGimmick)
		{
			continue;
		}

		const auto ConsiderPoint = [&](const FVector& Point)
		{
			const FVector ToPoint = Point - OwnerLocation;
			const double Distance = ToPoint.Size();
			// A surface containing the origin is contact, and has no meaningful direction.
			const bool bInside = Distance <= Range && (Distance <= KINDA_SMALL_NUMBER
				|| FVector::DotProduct(Forward, ToPoint / Distance) >= AngleLimit);

			if (bInside && Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestActor = TargetActor;
				DetectedPoint = Point;
			}
		};

		// Query the overlapping component's surface, not the actor pivot (which may be
		// outside the cone/range even when a door or other large object intersects it).
		// Sample along the sight axis too: the surface nearest the origin can be off to
		// one side while another part of the same object is directly in front of us.
		constexpr int32 AxisSamples = 8;
		bool bHasCollisionPoint = false;
		for (int32 Sample = 0; Sample <= AxisSamples + 1; ++Sample)
		{
			const double AxisDistance = Sample <= AxisSamples
				? Range * (static_cast<double>(Sample) / AxisSamples)
				: FMath::Clamp(FVector::DotProduct(TargetComponent->Bounds.Origin - OwnerLocation, Forward), 0.0, static_cast<double>(Range));
			FVector SurfacePoint;
			if (TargetComponent->GetClosestPointOnCollision(OwnerLocation + Forward * AxisDistance, SurfacePoint) >= 0.0f)
			{
				bHasCollisionPoint = true;
				ConsiderPoint(SurfacePoint);
			}
		}

		if (!bHasCollisionPoint)
		{
			// Complex-only collision cannot provide a closest point. Trace that component
			// instead of treating empty space in its bounding box as a contact.
			FCollisionQueryParams Params(SCENE_QUERY_STAT(NuiSightSurface), true);
			FHitResult Hit;
			if (TargetComponent->LineTraceComponent(Hit, OwnerLocation, OwnerLocation + Forward * Range, Params))
			{
				ConsiderPoint(Hit.ImpactPoint);
			}
			if (TargetComponent->LineTraceComponent(Hit, OwnerLocation, TargetComponent->Bounds.Origin, Params))
			{
				ConsiderPoint(Hit.ImpactPoint);
			}
		}
	}

	SetDetectedActor(ClosestActor);

	//if (bShowDetectionDebug && IsValid(DetectedActor))
	//{
	//	DrawDebugLine(GetWorld(), OwnerLocation, DetectedPoint, FColor::Cyan, false, 0.0f, 0, 2.0f);
	//}
	//if (bShowDetectionDebug && GEngine)
	//{
	//	GEngine->AddOnScreenDebugMessage(reinterpret_cast<UPTRINT>(this), 0.0f,
	//		IsValid(DetectedActor) ? FColor::Green : FColor::Red,
	//		FString::Printf(TEXT("Nui detected: %s | overlapping components: %d"),
	//			*GetNameSafe(DetectedActor), OutComponents.Num()));
	//}
}

void UNuiEyeSightComponent::SetDetectedActor(AActor* NewActor)
{
	AActor* OldActor = DetectedActor;
	bool bOldHasDetectedActor = HasDetectedActor();
	bool bOldHasDetectedGimmick = HasDetectedGimmick();

	DetectedActor = NewActor;

	bool bNewHasDetectedActor = HasDetectedActor();
	bool bNewHasDetectedGimmick = HasDetectedGimmick();

	if (bOldHasDetectedActor != bNewHasDetectedActor)
	{
		OnDetectionChanged.Broadcast(bNewHasDetectedActor);
	}

	if (OldActor != DetectedActor)
	{
		OnDetectedActorChanged.Broadcast(DetectedActor);
	}

	if (bOldHasDetectedGimmick != bNewHasDetectedGimmick)
	{
		OnGimmickFocusChanged.Broadcast(bNewHasDetectedGimmick);
	}
}

bool UNuiEyeSightComponent::HasDetectedActor() const
{
	return IsValid(DetectedActor);
}

AActor* UNuiEyeSightComponent::GetDetectedActor() const
{
	return DetectedActor;
}

bool UNuiEyeSightComponent::HasDetectedGimmick() const
{
	return IsValid(DetectedActor)
		&& DetectedActor->GetClass()->ImplementsInterface(UGimmickInterface::StaticClass());
}

bool UNuiEyeSightComponent::TryActionDetectedGimmick(AActor* InstigatorActor)
{
	if (HasDetectedGimmick() == false)
	{
		return false;
	}

	IGimmickInterface* Gimmick = Cast<IGimmickInterface>(DetectedActor);
	if (Gimmick == nullptr)
	{
		return false;
	}

	Gimmick->Action();
	return true;
}
