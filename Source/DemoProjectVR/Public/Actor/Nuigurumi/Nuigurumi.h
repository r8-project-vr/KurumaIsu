// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Actor.h"
#include "Nuigurumi.generated.h"

class ADeviceIMUReader;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UMeshComponent;

USTRUCT()
struct FNuiRimLightMeshState
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<UMeshComponent> RimLightMesh;
};

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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleDetectedActorChanged(AActor* NewActor);

	UFUNCTION()
	void HandleGimmickFocusChanged(bool bCanAction);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Captures the current physical device pose as the new forward/neutral pose. */
	UFUNCTION(BlueprintCallable, Category = "Device|IMU")
	void RecenterIMU();

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

	/** Mirrors the physical doll's rotation one-to-one, bypassing tilt scaling, clamping, dead zone, and interpolation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|IMU")
	bool bMirrorPhysicalPoseOneToOne = true;

	/** Legacy tuned-control settings used only when one-to-one mirroring is disabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|IMU", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float IMUTiltSensitivity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|IMU", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MaxIMUTiltDegrees = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|IMU", meta = (ClampMin = "0.1"))
	float IMURotationInterpSpeed = 3.0f;

	/** Pitch/Yaw/Roll multipliers after converting the sensor to Unreal coordinates. Use -1 to invert an axis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|IMU")
	FRotator IMURotationAxisScale = FRotator(1.0f, 1.0f, 1.0f);

	/** Ignores tiny absolute-angle changes without blocking slow deliberate movement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|IMU", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float IMUAngleDeadZone = 0.35f;

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
	float IMUCalibrationDuration = 2.0f;

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

	/** Applies an inward-fading rim glow only to the object currently found by NuiEyeSightComponent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Rim Light")
	bool bUseDetectedObjectRimLight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Rim Light")
	TObjectPtr<UMaterialInterface> DetectedObjectInnerGlowMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Rim Light")
	FLinearColor DetectedObjectRimLightColor = FLinearColor(0.08f, 0.65f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Rim Light", meta = (ClampMin = "0.0", DisplayName = "Rim Glow Intensity"))
	float DetectedObjectInnerGlowIntensity = 2.8f;

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

	UPROPERTY()
	UMaterialInstanceDynamic* DetectedObjectInnerGlowInstance = nullptr;

	UPROPERTY()
	TArray<FNuiRimLightMeshState> RimLightMeshStates;

	FQuat IMUReferenceOrientation = FQuat::Identity;
	FQuat CurrentIMUOffset = FQuat::Identity;
	FQuat TargetIMUOffset = FQuat::Identity;
	FVector CurrentIMULocationOffset = FVector::ZeroVector;
	float IMUCalibrationElapsed = 0.0f;
	bool bHasIMUReference = false;

	void UpdateIMUTransform(float DeltaTime, const FVector& BaseLocation, const FRotator& BaseRotation);
	void ApplyDetectedObjectRimLight(AActor* TargetActor);
	void ClearDetectedObjectRimLight();
};
