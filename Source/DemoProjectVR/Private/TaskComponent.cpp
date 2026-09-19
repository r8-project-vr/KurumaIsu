// Fill out your copyright notice in the Description page of Project Settings.


#include "TaskComponent.h"

// Sets default values for this component's properties
UTaskComponent::UTaskComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTaskComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
	stringChap1.taskString.Add(TEXT("目の前のドアを開けてみよう"));
	stringChap1.taskString.Add(TEXT("エレベーターの前まで移動しよう"));
	stringChap1.taskString.Add(TEXT("エレベーターで下の階に向かおう"));

	stringChap2.taskString.Add(TEXT("ダイヤの鍵を手に入れよう"));
	stringChap2.taskString.Add(TEXT("スペードの鍵を手に入れよう"));
	stringChap2.taskString.Add(TEXT("ハートの鍵を手に入れよう"));
	stringChap2.taskString.Add(TEXT("クローバーの鍵を手に入れよう"));
	stringChap2.taskString.Add(TEXT("思い出の写真を手に入れよう"));
	stringChap2.taskString.Add(TEXT("エレベーターに急げ！"));

	stringChap3.taskString.Add(TEXT("下の階に戻ろう"));
	stringChap3.taskString.Add(TEXT("玄関まで急ごう！"));

	clearChap1.classNum.Add(0);
	clearChap1.classNum.Add(1);
	clearChap1.classNum.Add(2);

	clearChap2.classNum.Add(3);
	clearChap2.classNum.Add(4);
	clearChap2.classNum.Add(5);
	clearChap2.classNum.Add(6);
	clearChap2.classNum.Add(7);
	clearChap2.classNum.Add(8);

	clearChap3.classNum.Add(9);
	clearChap3.classNum.Add(10);

	tasks.Add(stringChap1);
	tasks.Add(stringChap2);
	tasks.Add(stringChap3);

	taskClearNum.Add(clearChap1);
	taskClearNum.Add(clearChap2);
	taskClearNum.Add(clearChap3);
}


// Called every frame
void UTaskComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

FString UTaskComponent::MakeTaskString()
{
	FString output = "";
	output += TEXT("\n");
	int showTaskNum = showTaskMin;

	while (showTaskNum <= showTaskMax)
	{
		if (IsTaskClear(chapter, showTaskNum))
		{
			output += taskMarks[1];
		}
		else
		{
			output += taskMarks[0];
		}

		output += tasks[chapter - 1].taskString[showTaskNum - 1];
		output += TEXT("\n");

		showTaskNum++;
	}

	return output;
}

bool UTaskComponent::TaskClearCheck(AActor* checkActor)
{
	bool flag = false;
	int checkTaskNum = runTaskMin;

	while (checkTaskNum <= runTaskMax)
	{
		int index = taskClearNum[chapter - 1].classNum[checkTaskNum - 1];
		if (index == -2)
		{
			checkTaskNum++;
			continue;
		}

		bool isClear = needClearClass[index]->GetClass() == checkActor->GetClass();
		if (isClear)
		{
			taskClearNum[chapter - 1].classNum[checkTaskNum - 1] = clearNum;
			flag = true;
		}

		checkTaskNum++;
	}

	return flag;
}

bool UTaskComponent::IsTaskClear(int chap, int num)
{
	bool flag = false;

	bool isNotNull = taskClearNum.Num() >= chap;
	if (isNotNull)
	{
		isNotNull &= taskClearNum[chap - 1].classNum.Num() >= num;
		if (isNotNull)
		{
			flag = taskClearNum[chap - 1].classNum[num - 1] == clearNum;
		}
		else
		{
			DEBUG_PRINT("%s：タスク番号が範囲外です。", *GetName());
		}
	}
	else 
	{
		DEBUG_PRINT("%s：チャプター番号が範囲外です。", *GetName());
	}
	
	return flag;
}

void UTaskComponent::ToNextRunTask()
{
	bool isNotNull = taskClearNum[chapter - 1].classNum.Num() > runTaskMax;

	if (isNotNull)
	{
		runTaskMin++;
		runTaskMax++;
	}
}


void UTaskComponent::ToNextShowTask()
{
	bool isNotNull = taskClearNum[chapter - 1].classNum.Num() > showTaskMax;

	if (isNotNull)
	{
		showTaskMax++;
	}
}

void UTaskComponent::ResetTask()
{
	showTaskMin = 1;
	showTaskMax = 1;
	runTaskMin = 1;
	runTaskMax = 1;
}

void UTaskComponent::SetRunTask(int min, int max)
{
	bool isNotNull = taskClearNum[chapter - 1].classNum.Num() > max;

	if (isNotNull)
	{
		runTaskMin = min;
		runTaskMax = max;
	}
}

void UTaskComponent::AddChapter()
{
	ResetTask();

	chapter++;
}