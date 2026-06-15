// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "GimmickInterface.generated.h"

UINTERFACE(MinimalAPI)
class UGimmickInterface : public UInterface
{
	GENERATED_BODY()
};

class DEMOPROJECTVR_API IGimmickInterface
{
	GENERATED_BODY()

public:
	virtual void Action() = 0;
};
