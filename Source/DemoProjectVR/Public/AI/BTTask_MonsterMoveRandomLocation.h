#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MonsterMoveRandomLocation.generated.h"

UCLASS()
class DEMOPROJECTVR_API UBTTask_MonsterMoveRandomLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MonsterMoveRandomLocation();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Monster AI", meta = (ClampMin = "0"))
	float WanderRadius = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Monster AI", meta = (ClampMin = "0"))
	float AcceptanceRadius = 60.0f;
};
