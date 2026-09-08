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
		// Keep condition-triggered monsters functional even when their behavior
		// tree or blackboard has not been initialized yet.
		MoveToActor(Target, 100.0f, true, true, true, nullptr, true);
		return;
	}

	BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Target);
	BlackboardComp->SetValueAsBool(TEXT("ShouldFollow"),true);
}

// 停止
void AMonsterAIController::StopFollowing()
{
	UBlackboardComponent* BlackboardComp = GetBlackboardComponent();

	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsBool(TEXT("ShouldFollow"), false);
		BlackboardComp->ClearValue(TEXT("TargetActor"));
	}

	StopMovement();
}

// 一時停止
void AMonsterAIController::PauseFollowing()
{
	UBlackboardComponent* BlackboardComp = GetBlackboardComponent();

	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsBool(TEXT("ShouldFollow"), false);
	}

	StopMovement();
}

// 追跡再開
void AMonsterAIController::ResumeFollowing()
{
	UBlackboardComponent* BlackboardComp = GetBlackboardComponent();

	if (!BlackboardComp)
	{
		return;
	}

	AActor* TargetActor =
		Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("TargetActor")));

	if (!TargetActor)
	{
		return;
	}

	BlackboardComp->SetValueAsBool(TEXT("ShouldFollow"), true);
}
