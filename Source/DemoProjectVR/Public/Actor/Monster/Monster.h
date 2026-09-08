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
class UStaticMeshComponent;

UCLASS(Blueprintable)
class DEMOPROJECTVR_API AMonster : public ACharacter
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	AMonster();
	virtual void Tick(float DeltaTime) override;

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

	UFUNCTION(BlueprintCallable, Category = "Monster")
	void PauseFollowingTemporarily();

	void ResumeFollowing();

	//タイマー調節はここから
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|AI")
	float FollowPauseDuration = 15.0f;

	FTimerHandle FollowPauseTimerHandle;

	bool bConditionActive = false;
	bool bTemporarilyPaused = false;
	bool bIsFollowing = false;

protected:
	// 動きさ関連関数、変数
	FVector BaseMeshRelativeLocation = FVector::ZeroVector;
	FRotator BaseMeshRelativeRotation = FRotator::ZeroRotator;

	float ProceduralMotionTime = 0.0f;
	float MovementBlend = 0.0f;

	float TwitchCountdown = 2.0f;
	float TwitchElapsed = 0.0f;

	bool bTwitching = false;

	float CurrentTwitchRoll = 0.0f;
	float CurrentTwitchPitch = 0.0f;
	float CurrentTwitchYaw = 0.0f;
	float CurrentTwitchY = 0.0f;
	float CurrentTwitchZ = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Motion")
	float IdleBobAmount = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Motion")
	float IdleSwayAmount = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Motion")
	float IdleMotionSpeed = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Motion")
	float MoveBobAmount = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Motion")
	float MoveSwayAmount = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Motion")
	float MoveMotionSpeed = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Motion|Twitch")
	float TwitchDuration = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Motion|Twitch")
	float TwitchIntervalMin = 1.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Motion|Twitch")
	float TwitchIntervalMax = 4.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Motion|Twitch")
	float TwitchRotationAmount = 7.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Motion|Twitch")
	float TwitchLocationAmount = 2.0f;

	// Resolved to BPMonster's visible StaticMeshComponent during BeginPlay.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Monster")
	TObjectPtr<UStaticMeshComponent> MonsterMesh;
};
