// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MonsterAIController.generated.h"
class UBehaviorTree;

/**
 * 
 */
UCLASS()
class DEMOPROJECTVR_API AMonsterAIController : public AAIController
{
	GENERATED_BODY()

public:
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION(BlueprintCallable, Category = "Monster|AI")
	void StartFollowing(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Monster|AI")
	void StopFollowing();


protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;
};
