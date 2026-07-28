// Fill out your copyright notice in the Description page of Project Settings.
// 他ギミック実行用トリガー

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "GimmickInterface.h"

#include "IGimmickTrigger.generated.h"

class IGimmickInterface;

UCLASS()
class DEMOPROJECTVR_API AIGimmickTrigger : public AActor , public IGimmickInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AIGimmickTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
    void OnBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

private:
	UFUNCTION(BlueprintCallable)
	void Action() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// このアクターが実行するギミックリスト
	UPROPERTY(EditAnywhere, Category = "Setting")
	TArray<TScriptInterface<IGimmickInterface>> gimmickes;

	// 起動させるトリガー
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	TObjectPtr<UBoxComponent> TriggerBox;
};
