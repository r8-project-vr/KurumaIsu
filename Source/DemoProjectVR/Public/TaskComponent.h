// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DebugHelper.h"
#include "Actor/Gimmick/GimmickInterface.h"

#include "TaskComponent.generated.h"

USTRUCT()
struct FTaskStringRow
{
	GENERATED_BODY()

	TArray<FString> taskString;
};

USTRUCT()
struct FTaskClearRow
{
	GENERATED_BODY()

	TArray<int32> classNum;
};

UCLASS( meta=(BlueprintSpawnableComponent) )
class DEMOPROJECTVR_API UTaskComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTaskComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	FString MakeTaskString();

	UFUNCTION(BlueprintCallable)
	void TaskClearCheck(AActor* checkActor);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	// タスクがクリアしてるか確認用
	bool IsTaskClear(int chap, int num);

private:
	// 各タスクの先頭につくマーク
	TArray<FString> taskMarks = { "・", "✓" };

	int clearNum = -2;

	FTaskStringRow stringChap1, stringChap2, stringChap3;
	FTaskClearRow clearChap1, clearChap2, clearChap3;

public:		
	// タスクのチャプター（チャプターをまたがって複数タスクを表示できない）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	int chapter = 1;
	
	// 表示するタスク番号の最小
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	int showTaskMin = 1;
	
	// 表示するタスク番号の最大
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	int showTaskMax = 1;
		
	// タスクのクリアチェックを行うタスク番号の最小
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	int runTaskMin = 1;
	
	// タスクのクリアチェックを行うタスク番号の最大
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	int runTaskMax = 1;

	// タスク内容
	// 縦：タスクのチャプター（おそらく階層分け）
	// 横：タスクの内容
	TArray<FTaskStringRow> tasks;

	// タスククリアに必要なギミッククラスの番号
	// 縦横は上記と同じ
	TArray<FTaskClearRow> taskClearNum;

	// タスククリアに必要なギミッククラス
	UPROPERTY(EditAnywhere, Category = "Setting")
	TArray<TScriptInterface<IGimmickInterface>> needClearClass;
};
