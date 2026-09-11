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
	stringChap1.taskString.Add(TEXT("エレベーターのドアを開けよう"));
	stringChap1.taskString.Add(TEXT("エレベーターで下の階に向かおう"));
	stringChap2.taskString.Add(TEXT("chap2"));
	stringChap3.taskString.Add(TEXT("chap3"));

	clearChap1.classNum.Add(0);
	clearChap1.classNum.Add(1);
	clearChap1.classNum.Add(2);
	clearChap2.classNum.Add(1);
	clearChap3.classNum.Add(2);

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

	flag = taskClearNum[chap - 1].classNum[num - 1] == clearNum;

	return flag;
}

void UTaskComponent::ToNextRunTask()
{
	runTaskMin++;
	runTaskMax++;
}
