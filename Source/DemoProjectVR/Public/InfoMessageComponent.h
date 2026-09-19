// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InfoMessageComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DEMOPROJECTVR_API UInfoMessageComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInfoMessageComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	FString MakeString();

	UFUNCTION(BlueprintCallable)
	void DeleteMessage();

	UFUNCTION(BlueprintCallable)
	void NextSequence();

	UFUNCTION(BlueprintCallable)
	void NextProcess();

	UFUNCTION(BlueprintCallable)
	void ResetProcess();

	// デバイスの入力値を受け取る（入力正誤・反転に用いる）
	UFUNCTION(BlueprintCallable)
	void SendDeviceValue(float input);

	// デバイスの入力が正常かどうか（前後が完全に反転している場合は成功判定に含む）
	UFUNCTION(BlueprintCallable)
	bool IsDeviceNormal();

	// デバイスの入力値の反転が必要かどうか
	UFUNCTION(BlueprintCallable)
	bool IsNeedInversion();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	void UpdateMessage();
public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	FString ShowMessage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	int MasterSequence = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	int Process = 0;

private:
	TArray<FString> MasterMessage = { TEXT("左車輪を操作してください\nOperate the LEFT wheel\n"), TEXT("右車輪を操作してください\nOperate the RIGHT wheel\n")};
	TArray<FString> ProcessMessage = {
		TEXT("デバイス番号を選択してください\nスタッフが入力します\n少々お待ちください\nStaff now inputting. PLEASE WAIT..."),
		TEXT("前に回してください\nTurn the wheel FORWARD"),
		TEXT("後ろに回してください\nTurn the wheel BACK"),
		TEXT("デバイスを再接続してください\nスタッフが操作します\n少々お待ちください\nStaff now Operating. PLEASE WAIT...")
	};

	// プロセスのリスタート番号（配列のメッセージ参照用）
	int processRestartNum = 1;
	
	float forwardInputSum = 0.0f;
	float backInputSum = 0.0f;
};
