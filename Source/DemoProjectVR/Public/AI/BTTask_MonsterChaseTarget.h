#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MonsterChaseTarget.generated.h"

UCLASS()
class DEMOPROJECTVR_API UBTTask_MonsterChaseTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MonsterChaseTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Monster AI")
	FName TargetActorKeyName = TEXT("TargetActor");

	UPROPERTY(EditAnywhere, Category = "Monster AI", meta = (ClampMin = "0"))
	float AcceptanceRadius = 140.0f;
};
