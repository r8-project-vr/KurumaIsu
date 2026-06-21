// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "NuiEyeSightComponent.generated.h"

// 今DetectedActorがあるかを確認するDelegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDetectionChanged, bool, bHasDetectedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDetectedActorChanged, AActor*, DetectedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGimmickFocusChanged, bool, bCanAction);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEMOPROJECTVR_API UNuiEyeSightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNuiEyeSightComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// 視野チェック
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeSight")
	float SightDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeSight")
	float SightAngle = 60.0f;

	// このタグを持つActorもDetectedActorに入れます
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeSight")
	FName TargetTag = TEXT("Monster");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeSight")
	bool bDetectTargetTag = true;

	// GimmickInterfaceを持つActorを検出対象にします
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
	bool bDetectGimmickInterface = false;

	// VR機器なしで確認するためのキーボードデバッグ入力です
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Debug")
	bool bEnableKeyboardDebugAction = true;

	UPROPERTY(BlueprintAssignable, Category="EyeSight")
	FOnDetectionChanged OnDetectionChanged;

	UPROPERTY(BlueprintAssignable, Category="EyeSight")
	FOnDetectedActorChanged OnDetectedActorChanged;

	// UI側はこのDelegateで「作動」表示のON/OFFを切り替えます
	UPROPERTY(BlueprintAssignable, Category="Gimmick")
	FOnGimmickFocusChanged OnGimmickFocusChanged;

public:
	UFUNCTION(BlueprintCallable,Category = "EyeSight")
	void DetectObject();

	void SetDetectedActor(AActor* NewActor);

	UFUNCTION(BlueprintCallable, Category = "EyeSight")
	bool HasDetectedActor() const;

	UFUNCTION(BlueprintPure, Category = "EyeSight")
	AActor* GetDetectedActor() const;

	UFUNCTION(BlueprintPure, Category = "Gimmick")
	bool HasDetectedGimmick() const;

	// 入力ボタンから呼ぶと、今見ているギミックを作動させます
	UFUNCTION(BlueprintCallable, Category = "Gimmick")
	bool TryActionDetectedGimmick(AActor* InstigatorActor = nullptr);

private:
	void HandleDebugActionInput();
	bool TryActionElevatorDebug(AActor* GimmickActor) const;
	bool TryCallFunctionByName(AActor* TargetActor, const FName FunctionName) const;
	bool IsDetectionTarget(AActor* TargetActor) const;
	bool IsGimmickActor(AActor* TargetActor) const;

private:
	UPROPERTY()
	AActor* DetectedActor = nullptr;
};


