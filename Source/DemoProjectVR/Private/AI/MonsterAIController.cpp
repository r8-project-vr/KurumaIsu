#include "AI/MonsterAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "UObject/ConstructorHelpers.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

AMonsterAIController::AMonsterAIController()
{
	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BehaviorTreeFinder(TEXT("/Game/Characters/Monster/BT_Monster.BT_Monster"));
	if (BehaviorTreeFinder.Succeeded())
	{
		BehaviorTreeAsset = BehaviorTreeFinder.Object;
	}

	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*AIPerception);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = 180.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
	AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AMonsterAIController::HandleTargetPerceptionUpdated);
}

void AMonsterAIController::BeginPlay()
{
	Super::BeginPlay();

	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = LoseSightRadius;
	AIPerception->RequestStimuliListenerUpdate();
}

void AMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BehaviorTreeAsset)
	{
		const bool bStartedBehaviorTree = RunBehaviorTree(BehaviorTreeAsset);
		UE_LOG(LogTemp, Log, TEXT("Monster AI: RunBehaviorTree %s"), bStartedBehaviorTree ? TEXT("succeeded") : TEXT("failed"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Monster AI: BehaviorTreeAsset is not set."));
	}

	if (bUseNativeMovementFallback)
	{
		FTimerHandle PlayerScanTimerHandle;
		GetWorldTimerManager().SetTimer(
			PlayerScanTimerHandle,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (!bUseNativeMovementFallback || CurrentTarget.IsValid() || !GetPawn())
				{
					return;
				}

				APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
				if (!IsValid(PlayerPawn))
				{
					return;
				}

				const float DistanceSquared = FVector::DistSquared(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());
				if (DistanceSquared <= FMath::Square(SightRadius))
				{
					if (UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
					{
						BlackboardComponent->SetValueAsObject(TargetActorKeyName, PlayerPawn);
					}

					StopMovement();
					RestartBehaviorTreeLogic();
					StartNativeChase(PlayerPawn);
					UE_LOG(LogTemp, Log, TEXT("Monster AI: Distance scan found player %s."), *PlayerPawn->GetName());
				}
			}),
			0.25f,
			true);

		StartNativeWander();
	}
}

void AMonsterAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (bUseNativeMovementFallback && !CurrentTarget.IsValid())
	{
		ScheduleNativeWander();
	}
}

void AMonsterAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!IsPlayerPawn(Actor))
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		if (UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
		{
			BlackboardComponent->SetValueAsObject(TargetActorKeyName, Actor);
			UE_LOG(LogTemp, Log, TEXT("Monster AI: TargetActor set to %s"), *Actor->GetName());
		}

		StopMovement();
		RestartBehaviorTreeLogic();
		StartNativeChase(Actor);
		return;
	}

	// Keep chasing after the first sighting. A brief perception loss can happen when
	// the player turns, clips behind geometry, or leaves the sight cone for a moment.
	if (CurrentTarget.Get() == Actor)
	{
		StartNativeChase(Actor);
	}
}

void AMonsterAIController::StartNativeWander()
{
	if (!bUseNativeMovementFallback || CurrentTarget.IsValid() || !GetPawn())
	{
		return;
	}

	UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavigationSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Monster AI: Native fallback could not find NavigationSystem."));
		ScheduleNativeWander();
		return;
	}

	FNavLocation Destination;
	if (!NavigationSystem->GetRandomReachablePointInRadius(GetPawn()->GetActorLocation(), WanderRadius, Destination))
	{
		UE_LOG(LogTemp, Warning, TEXT("Monster AI: Native fallback could not find reachable wander point."));
		ScheduleNativeWander();
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Monster AI: Native fallback wandering."));
	MoveToLocation(Destination.Location, WanderAcceptanceRadius);
}

void AMonsterAIController::StartNativeChase(AActor* TargetActor)
{
	if (!bUseNativeMovementFallback || !IsValid(TargetActor))
	{
		return;
	}

	CurrentTarget = TargetActor;
	GetWorldTimerManager().ClearTimer(WanderTimerHandle);
	UE_LOG(LogTemp, Log, TEXT("Monster AI: Native fallback chasing %s."), *TargetActor->GetName());
	MoveToActor(TargetActor, ChaseAcceptanceRadius, true, true, true, nullptr, true);
}

void AMonsterAIController::ScheduleNativeWander()
{
	if (!bUseNativeMovementFallback || CurrentTarget.IsValid())
	{
		return;
	}

	const float Delay = FMath::FRandRange(WanderPauseMin, FMath::Max(WanderPauseMin, WanderPauseMax));
	GetWorldTimerManager().SetTimer(WanderTimerHandle, this, &AMonsterAIController::StartNativeWander, Delay, false);
}

void AMonsterAIController::RestartBehaviorTreeLogic()
{
	if (UBrainComponent* BrainComp = GetBrainComponent())
	{
		BrainComp->RestartLogic();
	}
}

bool AMonsterAIController::IsPlayerPawn(const AActor* Actor) const
{
	const APawn* PlayerPawn = Cast<APawn>(Actor);
	return IsValid(PlayerPawn) && PlayerPawn->IsPlayerControlled();
}
