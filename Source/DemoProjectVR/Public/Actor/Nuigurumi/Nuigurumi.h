// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Nuigurumi.generated.h"

UCLASS()
class DEMOPROJECTVR_API ANuigurumi : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANuigurumi();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// プレイヤーの視点前方にぬいぐるみを追従させます
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	bool bFollowPlayerView = true;

	// X=前, Y=右, Z=上。手元に置きたいので少し右下に寄せます
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	FVector PlayerViewOffset = FVector(170.0f, 0.0f, -45.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	float FollowInterpSpeed = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	bool bMatchPlayerViewRotation = true;

private:
	// 基本Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* Collider;
};
