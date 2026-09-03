// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Actor.h"
#include "Nuigurumi.generated.h"

class ADeviceIMUReader;

UCLASS(Blueprintable)
class DEMOPROJECTVR_API UNuiInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void BuildPrompt(
		const FText& Text,
		const FSlateFontInfo& Font,
		const FLinearColor& TextColor,
		const FLinearColor& BackgroundColor);
};

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

	UFUNCTION()
	void HandleDetectedActorChanged(AActor* NewActor);

	UFUNCTION()
	void HandleGimmickFocusChanged(bool bCanAction);

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

	/** Test-only IMU tilt control. Enabled only on the NuiTest level instance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|IMU")
	bool bUseIMUTiltControl = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|IMU", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float IMUTiltSensitivity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|IMU", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MaxIMUTiltDegrees = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|IMU", meta = (ClampMin = "0.1"))
	float IMURotationInterpSpeed = 3.0f;

	/** Unreal units (cm) moved per 1 g of acceleration change. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|IMU", meta = (ClampMin = "0.0"))
	float IMUMovementScale = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|IMU", meta = (ClampMin = "0.0"))
	float MaxIMUMovementDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|IMU", meta = (ClampMin = "0.1"))
	float IMUMovementInterpSpeed = 3.0f;

	/** Axis signs and remapping can be tuned per level. Defaults reverse the horizontal sensor axes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|IMU")
	FVector IMUMovementAxisScale = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|IMU")
	bool bShowIMUDebug = true;

	/** Keep the physical model still for this long after Play to establish its neutral pose. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|IMU", meta = (ClampMin = "0.2", ClampMax = "5.0"))
	float IMUCalibrationDuration = 1.0f;

	/** Only physical rotations faster than this update the held pose. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|IMU", meta = (ClampMin = "0.0"))
	float IMUMotionThreshold = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction UI")
	FText InteractionPromptText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction UI")
	TSubclassOf<UUserWidget> InteractionPromptWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction UI")
	FVector InteractionPromptOffset = FVector(0.0f, 0.0f, 30.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction UI")
	FSlateFontInfo InteractionPromptFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction UI")
	FLinearColor InteractionPromptTextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction UI")
	FLinearColor InteractionPromptBackgroundColor = FLinearColor(0.02f, 0.02f, 0.02f, 0.75f);

private:
	// 基本Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* Collider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* InteractionPromptWidget;

	UPROPERTY()
	AActor* InteractionPromptTarget = nullptr;

	UPROPERTY()
	ADeviceIMUReader* IMUReader = nullptr;

	FQuat IMUReferenceOrientation = FQuat::Identity;
	FRotator CurrentIMUOffset = FRotator::ZeroRotator;
	FRotator HeldIMUTarget = FRotator::ZeroRotator;
	FVector CurrentIMULocationOffset = FVector::ZeroVector;
	float IMUCalibrationElapsed = 0.0f;
	bool bHasIMUReference = false;

	void UpdateIMUTransform(float DeltaTime, const FVector& BaseLocation, const FRotator& BaseRotation);
};
