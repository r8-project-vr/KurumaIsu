#include "AI/BTTask_MonsterMoveRandomLocation.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_MonsterMoveRandomLocation::UBTTask_MonsterMoveRandomLocation()
{
	NodeName = TEXT("Monster Move Random Location");
}

EBTNodeResult::Type UBTTask_MonsterMoveRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetCurrent(ControlledPawn->GetWorld());
	if (!NavigationSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Monster AI: NavigationSystem is missing."));
		return EBTNodeResult::Failed;
	}

	FNavLocation Destination;
	if (!NavigationSystem->GetRandomReachablePointInRadius(ControlledPawn->GetActorLocation(), WanderRadius, Destination))
	{
		UE_LOG(LogTemp, Warning, TEXT("Monster AI: Could not find random reachable point. Check NavMesh around BP_Monster."));
		return EBTNodeResult::Failed;
	}

	FAIMoveRequest MoveRequest(Destination.Location);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetProjectGoalLocation(true);

	const FPathFollowingRequestResult MoveResult = AIController->MoveTo(MoveRequest);
	if (MoveResult.Code == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogTemp, Warning, TEXT("Monster AI: Random MoveTo request failed."));
		return EBTNodeResult::Failed;
	}

	if (MoveResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		return EBTNodeResult::Succeeded;
	}

	WaitForMessage(OwnerComp, UBrainComponent::AIMessage_MoveFinished, MoveResult.MoveId);
	return EBTNodeResult::InProgress;
}
