// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Monster/MonsterAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

void AMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}
}

// 呼び出すとプレイヤーを追いかける
void AMonsterAIController::StartFollowing(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	UBlackboardComponent* BlackboardComp = GetBlackboardComponent();

	if (!BlackboardComp)
	{
		return;
	}

	BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Target);
	BlackboardComp->SetValueAsBool(TEXT("ShouldFollow"),true);
}

// 停止
void AMonsterAIController::StopFollowing()
{
	UBlackboardComponent* BlackboardComp = GetBlackboardComponent();

	if (!BlackboardComp)
	{
		return;
	}

	BlackboardComp->SetValueAsBool(TEXT("ShouldFollow"), false);

	BlackboardComp->ClearValue(TEXT("TargetActor"));

	StopMovement();
}