// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Nuigurumi/NuiEyeSightComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/EngineTypes.h"
#include "GimmickInterface.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/UnrealType.h"

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
	HandleDebugActionInput();
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
		if (IsDetectionTarget(TargetActor) == false)
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
	return IsGimmickActor(DetectedActor);
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

void UNuiEyeSightComponent::HandleDebugActionInput()
{
	if (bEnableKeyboardDebugAction == false)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController == nullptr || PlayerController->WasInputKeyJustPressed(EKeys::E) == false)
	{
		return;
	}

	bool bDidAction = TryActionElevatorDebug(DetectedActor);
	if (bDidAction == false)
	{
		bDidAction = TryActionDetectedGimmick(GetOwner());
	}
	if (GEngine != nullptr)
	{
		const FString Message = bDidAction
			? TEXT("Debug Action: E pressed")
			: TEXT("Debug Action: No detected gimmick");
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, bDidAction ? FColor::Cyan : FColor::Yellow, Message);
	}
}

bool UNuiEyeSightComponent::TryActionElevatorDebug(AActor* GimmickActor) const
{
	if (IsValid(GimmickActor) == false)
	{
		return false;
	}

	AActor* ElevatorActor = nullptr;
	FObjectProperty* ElevatorProperty = FindFProperty<FObjectProperty>(GimmickActor->GetClass(), TEXT("elevatorActor"));
	if (ElevatorProperty != nullptr)
	{
		ElevatorActor = Cast<AActor>(ElevatorProperty->GetObjectPropertyValue_InContainer(GimmickActor));
	}

	if (ElevatorActor == nullptr)
	{
		ElevatorActor = GimmickActor;
	}

	const bool bMovedFloor = TryCallFunctionByName(ElevatorActor, TEXT("MoveElevator"))
		|| TryCallFunctionByName(ElevatorActor, TEXT("MoveUp"));
	const bool bStartedAction = TryCallFunctionByName(ElevatorActor, TEXT("Action"));

	return bMovedFloor || bStartedAction;
}

bool UNuiEyeSightComponent::TryCallFunctionByName(AActor* TargetActor, const FName FunctionName) const
{
	if (IsValid(TargetActor) == false)
	{
		return false;
	}

	UFunction* Function = TargetActor->FindFunction(FunctionName);
	if (Function == nullptr)
	{
		return false;
	}

	uint8* Params = nullptr;
	TArray<uint8> ParamBuffer;
	if (Function->ParmsSize > 0)
	{
		ParamBuffer.SetNumZeroed(Function->ParmsSize);
		Params = ParamBuffer.GetData();

		if (FBoolProperty* BoolParam = CastField<FBoolProperty>(Function->ChildProperties))
		{
			BoolParam->SetPropertyValue_InContainer(Params, true);
		}
	}

	TargetActor->ProcessEvent(Function, Params);
	return true;
}

bool UNuiEyeSightComponent::IsDetectionTarget(AActor* TargetActor) const
{
	if (TargetActor == nullptr)
	{
		return false;
	}

	const bool bMatchesTag = bDetectTargetTag && TargetTag != NAME_None && TargetActor->ActorHasTag(TargetTag);
	const bool bMatchesGimmick = bDetectGimmickInterface && IsGimmickActor(TargetActor);

	return bMatchesTag || bMatchesGimmick;
}

bool UNuiEyeSightComponent::IsGimmickActor(AActor* TargetActor) const
{
	return IsValid(TargetActor) && TargetActor->GetClass()->ImplementsInterface(UGimmickInterface::StaticClass());
}


