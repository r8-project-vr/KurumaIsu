#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "TimerManager.h"
#include "MonsterAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UBehaviorTree;

UCLASS()
class DEMOPROJECTVR_API AMonsterAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMonsterAIController();

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster AI")
	TObjectPtr<UAIPerceptionComponent> AIPerception;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Wander", meta = (ClampMin = "0"))
	float WanderRadius = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Wander", meta = (ClampMin = "0"))
	float WanderAcceptanceRadius = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Wander", meta = (ClampMin = "0"))
	float WanderPauseMin = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Wander", meta = (ClampMin = "0"))
	float WanderPauseMax = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Chase", meta = (ClampMin = "0"))
	float ChaseAcceptanceRadius = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Sight", meta = (ClampMin = "0"))
	float SightRadius = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Sight", meta = (ClampMin = "0"))
	float LoseSightRadius = 2200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Blackboard")
	FName TargetActorKeyName = TEXT("TargetActor");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Fallback")
	bool bUseNativeMovementFallback = true;

	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
	void StartNativeWander();
	void StartNativeChase(AActor* TargetActor);
	void ScheduleNativeWander();
	void RestartBehaviorTreeLogic();
	bool IsPlayerPawn(const AActor* Actor) const;

	TWeakObjectPtr<AActor> CurrentTarget;
	FTimerHandle WanderTimerHandle;
};
