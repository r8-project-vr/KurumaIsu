// Fill out your copyright notice in the Description page of Project Settings.


#include "InfoMessageComponent.h"
#include "DebugHelper.h"

// Sets default values for this component's properties
UInfoMessageComponent::UInfoMessageComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInfoMessageComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UInfoMessageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

FString UInfoMessageComponent::MakeString()
{
	UpdateMessage();

	return ShowMessage;
}

void UInfoMessageComponent::UpdateMessage()
{
	ShowMessage = MasterMessage[MasterSequence];
	ShowMessage += ProcessMessage[Process];
}

void UInfoMessageComponent::DeleteMessage()
{
	ShowMessage = "";
}

void UInfoMessageComponent::NextSequence()
{
	MasterSequence++;

	ResetInputSum();
	Process = 0;
}

void UInfoMessageComponent::NextProcess()
{
	Process++;
}

void UInfoMessageComponent::ResetProcess()
{
	Process = processRestartNum;
	ResetInputSum();
}

void UInfoMessageComponent::SendDeviceValue(float input)
{
	bool isForward = Process == 1;
	if (isForward)
	{
		forwardInputSum += input;
	}
	else
	{
		backInputSum += input;
	}
}

bool UInfoMessageComponent::IsDeviceNormal()
{
	bool result = true;
	
	float range = 0.5f;
	bool isMoved = !FMath::IsNearlyEqual(forwardInputSum, 0.0f, range);
	isMoved &= !FMath::IsNearlyEqual(backInputSum, 0.0f, range);

	if (!isMoved)
	{
		result = false;
	}

	if (forwardInputSum < 0 && backInputSum < 0)
	{
		result = false;
	}
	if (forwardInputSum > 0 && backInputSum > 0)
	{
		result = false;
	}

	return result;
}

bool UInfoMessageComponent::IsNeedInversion()
{
	bool result = false;
	
	// 入力値が反対だったらtrue（+入力じゃなかったら）
	result = forwardInputSum < 0;

	return result;
}

void UInfoMessageComponent::PrintInputValues()
{
	DEBUG_PRINT("前入力合計：%lf / 後ろ入力合計：%lf", forwardInputSum, backInputSum);
}

void UInfoMessageComponent::ResetInputSum()
{
	forwardInputSum = 0.0f;
	backInputSum = 0.0f;
}