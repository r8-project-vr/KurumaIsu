#include "Actor/Monster/Monster.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Actor/Monster/MonsterAIController.h"

AMonster::AMonster()
{

}

void AMonster::ActivateCondition(APawn* FollowTarget)
{
	if (ActivationType != EMonsterActivationType::Condition)
	{
		return;
	}

	if (!FollowTarget)
	{
		return;
	}

	if (AMonsterAIController* AIController =
		Cast<AMonsterAIController>(GetController()))
	{
		AIController->StartFollowing(FollowTarget);
	}
}

void AMonster::DeactivateCondition()
{
	if (ActivationType != EMonsterActivationType::Condition)
	{
		return;
	}

	if (AMonsterAIController* AIController =
		Cast<AMonsterAIController>(GetController()))
	{
		AIController->StopFollowing();
	}
}

void AMonster::StartFollowing(AActor* Target)
{
}

void AMonster::StopFollowing()
{
}

bool AMonster::IsFollowing() const
{
	return false;
}
