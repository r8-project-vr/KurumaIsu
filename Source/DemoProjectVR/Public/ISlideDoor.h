// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GimmickInterface.h"

#include "ISlideDoor.generated.h"

UCLASS()
class DEMOPROJECTVR_API AISlideDoor : public AActor, public IGimmickInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AISlideDoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void Action() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interface")
	float moveTime = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interface")
	float moveDistance = 20.0f;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	float actionRunningTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interface")
	bool isLeftOpen = false;

	bool isAction = false;
	bool isOpen = false;

private:
	FVector beforeLocation = FVector::Zero();

};
