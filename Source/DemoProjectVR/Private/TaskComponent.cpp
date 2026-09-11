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
	
	stringChap1.taskString.Add("目の前のドアを開けてみよう");
	stringChap2.taskString.Add("chap2");
	stringChap3.taskString.Add("chap3");

	clearChap1.classNum.Add(0);
	clearChap2.classNum.Add(-1);
	clearChap3.classNum.Add(-2);

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
		output += "\n";

		showTaskNum++;
	}

	return output;
}

void UTaskComponent::TaskClearCheck(AActor* checkActor)
{
	int checkTaskNum = runTaskMin;

	while (checkTaskNum <= runTaskMax)
	{
		bool isClear = needClearClass[taskClearNum[chapter - 1].classNum[checkTaskNum - 1]] == checkActor;
		if (isClear)
		{
			taskClearNum[chapter - 1].classNum[checkTaskNum - 1] = clearNum;
		}

		checkTaskNum++;
	}
}

bool UTaskComponent::IsTaskClear(int chap, int num)
{
	bool flag = false;

	flag = taskClearNum[chap].classNum[num] == clearNum;

	return flag;
}
