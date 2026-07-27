#include "AI/BTTask_MonsterChaseTarget.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_MonsterChaseTarget::UBTTask_MonsterChaseTarget()
{
	NodeName = TEXT("Monster Chase Target");
}

EBTNodeResult::Type UBTTask_MonsterChaseTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = BlackboardComponent
		? Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKeyName))
		: nullptr;

	if (!IsValid(TargetActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Monster AI: Chase failed because TargetActor is not valid."));
		return EBTNodeResult::Failed;
	}

	FAIMoveRequest MoveRequest(TargetActor);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetAllowPartialPath(true);

	const FPathFollowingRequestResult MoveResult = AIController->MoveTo(MoveRequest);
	if (MoveResult.Code == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogTemp, Warning, TEXT("Monster AI: Chase MoveTo request failed."));
		return EBTNodeResult::Failed;
	}

	if (MoveResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		return EBTNodeResult::Succeeded;
	}

	WaitForMessage(OwnerComp, UBrainComponent::AIMessage_MoveFinished, MoveResult.MoveId);
	return EBTNodeResult::InProgress;
}
