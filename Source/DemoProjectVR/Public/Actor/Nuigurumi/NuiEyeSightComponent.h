// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "NuiEyeSightComponent.generated.h"

// 今DetectedActorがあるかを確認するDelegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDetectionChanged, bool, bHasDetectedActor);

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

	// モンスターだけをDetectedActorに入るようにします
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EyeSight")
	FName TargetTag = TEXT("Monster");

	UPROPERTY(BlueprintAssignable, Category="EyeSight")
	FOnDetectionChanged OnDetectionChanged;

public:
	UFUNCTION(BlueprintCallable,Category = "EyeSight")
	void DetectObject();

	void SetDetectedActor(AActor* NewActor);

	UFUNCTION(BlueprintCallable, Category = "EyeSight")
	bool HasDetectedActor() const;


private:
	UPROPERTY()
	AActor* DetectedActor = nullptr;
};
