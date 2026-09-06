#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Monster.generated.h"

// モンスタータイプ
UENUM(BlueprintType)
enum class EMonsterActivationType : uint8
{
	Sight       UMETA(DisplayName = "Sight"),
	Condition   UMETA(DisplayName = "Condition")
};

UCLASS(Blueprintable)
class DEMOPROJECTVR_API AMonster : public ACharacter
{
	GENERATED_BODY()

public:
	AMonster();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster")
	EMonsterActivationType ActivationType = EMonsterActivationType::Sight;

	UFUNCTION(BlueprintCallable, Category = "Monster")
	void ActivateCondition(APawn* FollowTarget);

	UFUNCTION(BlueprintCallable, Category = "Monster")
	void DeactivateCondition();


	// 行動決める関数
	UFUNCTION(BlueprintCallable)
	void StartFollowing(AActor* Target);

	UFUNCTION(BlueprintCallable)
	void StopFollowing();

	UFUNCTION(BlueprintPure)
	bool IsFollowing() const;
};
