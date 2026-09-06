// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Nuigurumi/Nuigurumi.h"

#include "Actor/Nuigurumi/NuiEyeSightComponent.h"
#include "Device/DeviceIMUReader.h"
#include "Components/SphereComponent.h"
#include "Components/MeshComponent.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"


namespace
{
	float ApplyAngularDeadZone(
		float AngleDegrees,
		float DeadZoneDegrees)
	{
		return FMath::Abs(AngleDegrees) < DeadZoneDegrees
			? 0.0f
			: AngleDegrees;
	}


	FQuat ConvertSensorOrientationToUnreal(
		const FRotator& SensorOrientation)
	{
		// レガシーORIプロトコル用の座標変換
		//
		// QUATプロトコルを使用している場合、
		// この処理は基本的に使用されない

		const FRotator UnrealOrientation(
			SensorOrientation.Roll,
			-SensorOrientation.Yaw,
			-SensorOrientation.Pitch
		);

		return UnrealOrientation
			.Quaternion()
			.GetNormalized();
	}


	FQuat ConvertRelativeSensorQuaternionToUnreal(
		const FQuat& RelativeSensorRotation)
	{
		// センサーの相対回転をUnreal Engineの座標系へ変換する
		//
		// Sensor X -> Unreal X
		// Sensor Z -> Unreal Y
		// Sensor Y -> Unreal Z
		//
		// 実機の取り付け方向に合わせて
		// PitchとYawの回転方向を反転する

		FQuat UnrealRotation(
			-RelativeSensorRotation.X,
			RelativeSensorRotation.Z,
			RelativeSensorRotation.Y,
			RelativeSensorRotation.W
		);

		return UnrealRotation.GetNormalized();
	}


	FVector QuaternionToRotationVectorDegrees(
		FQuat Rotation)
	{
		Rotation.Normalize();

		// 常に最短回転を使用する
		if (Rotation.W < 0.0f)
		{
			Rotation.X *= -1.0f;
			Rotation.Y *= -1.0f;
			Rotation.Z *= -1.0f;
			Rotation.W *= -1.0f;
		}

		const float ClampedW =
			FMath::Clamp(
				Rotation.W,
				-1.0f,
				1.0f);

		const float AngleRadians =
			2.0f * FMath::Acos(ClampedW);

		if (AngleRadians < KINDA_SMALL_NUMBER)
		{
			return FVector::ZeroVector;
		}

		const FVector Axis =
			Rotation
			.GetRotationAxis()
			.GetSafeNormal();

		return Axis *
			FMath::RadiansToDegrees(
				AngleRadians);
	}


	FQuat RotationVectorDegreesToQuaternion(
		const FVector& RotationVector)
	{
		const float AngleDegrees =
			RotationVector.Size();

		if (AngleDegrees < KINDA_SMALL_NUMBER)
		{
			return FQuat::Identity;
		}

		const FVector Axis =
			RotationVector /
			AngleDegrees;

		return FQuat(
			Axis,
			FMath::DegreesToRadians(
				AngleDegrees))
			.GetNormalized();
	}
}



// Sets default values
ANuigurumi::ANuigurumi()
{
	PrimaryActorTick.bCanEverTick = true;


	Collider =
		CreateDefaultSubobject<USphereComponent>(
			TEXT("Collider"));

	RootComponent =
		Collider;


	DetectedObjectPostProcess =
		CreateDefaultSubobject<UPostProcessComponent>(
			TEXT("DetectedObjectPostProcess"));

	DetectedObjectPostProcess->bUnbound = true;
	DetectedObjectPostProcess->BlendWeight = 1.0f;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PostProcessMaterialFinder(
		TEXT("/Game/Characters/Nuigurumi/PP_NuiInnerGlowScreenV2.PP_NuiInnerGlowScreenV2"));

	if (PostProcessMaterialFinder.Succeeded())
	{
		DetectedObjectPostProcessMaterial = PostProcessMaterialFinder.Object;
	}
}



// Called when the game starts or when spawned
void ANuigurumi::BeginPlay()
{
	Super::BeginPlay();


	if (UNuiEyeSightComponent* EyeSight =
		FindComponentByClass<
		UNuiEyeSightComponent>())
	{
		EyeSight
			->OnDetectedActorChanged
			.AddDynamic(
				this,
				&ANuigurumi::HandleDetectedActorChanged);
	}

	if (DetectedObjectPostProcessMaterial)
	{
		DetectedObjectPostProcessInstance =
			UMaterialInstanceDynamic::Create(
				DetectedObjectPostProcessMaterial,
				this);

		DetectedObjectPostProcessInstance->SetVectorParameterValue(
			TEXT("GlowColor"),
			DetectedObjectRimLightColor *
			DetectedObjectInnerGlowIntensity);

		DetectedObjectPostProcess->Settings.AddBlendable(
			DetectedObjectPostProcessInstance,
			1.0f);
	}
}



void ANuigurumi::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	ClearDetectedObjectRimLight();
	Super::EndPlay(EndPlayReason);
}



// Called every frame
void ANuigurumi::Tick(
	float DeltaTime)
{
	Super::Tick(
		DeltaTime);


	APlayerController* PlayerController =
		UGameplayStatics::GetPlayerController(
			this,
			0);


	if (bFollowPlayerView == false)
	{
		const FQuat BaseRotation =
			(
				GetActorQuat()
				* CurrentIMUOffset.Inverse()
				)
			.GetNormalized();


		UpdateIMUTransform(
			DeltaTime,
			GetActorLocation()
			- CurrentIMULocationOffset,
			BaseRotation.Rotator());

		return;
	}


	if (PlayerController == nullptr ||
		PlayerController->PlayerCameraManager == nullptr)
	{
		return;
	}


	const FVector CameraLocation =
		PlayerController
		->PlayerCameraManager
		->GetCameraLocation();


	const FRotator CameraRotation =
		PlayerController
		->PlayerCameraManager
		->GetCameraRotation();


	const FRotationMatrix CameraMatrix(
		CameraRotation);


	const FVector TargetLocation =
		CameraLocation
		+ CameraRotation.Vector()
		* PlayerViewOffset.X
		+ CameraMatrix
		.GetScaledAxis(EAxis::Y)
		* PlayerViewOffset.Y
		+ CameraMatrix
		.GetScaledAxis(EAxis::Z)
		* (PlayerViewOffset.Z - 30.0f);


	const FVector NewLocation =
		FMath::VInterpTo(
			GetActorLocation(),
			TargetLocation,
			DeltaTime,
			FollowInterpSpeed);


	if (bMatchPlayerViewRotation)
	{
		if (bUseIMUTiltControl)
		{
			// IMU回転を保持したまま
			// カメラ姿勢を基準として使用する
			UpdateIMUTransform(
				DeltaTime,
				NewLocation,
				CameraRotation);
		}
		else
		{
			const FRotator NewRotation =
				FMath::RInterpTo(
					GetActorRotation(),
					CameraRotation,
					DeltaTime,
					FollowInterpSpeed);


			UpdateIMUTransform(
				DeltaTime,
				NewLocation,
				NewRotation);
		}
	}
	else
	{
		const FQuat BaseRotation =
			(
				GetActorQuat()
				* CurrentIMUOffset.Inverse()
				)
			.GetNormalized();


		UpdateIMUTransform(
			DeltaTime,
			NewLocation,
			BaseRotation.Rotator());
	}
}



void ANuigurumi::RecenterIMU()
{
	IMUReferenceOrientation =
		FQuat::Identity;

	CurrentIMUOffset =
		FQuat::Identity;

	TargetIMUOffset =
		FQuat::Identity;

	CurrentIMULocationOffset =
		FVector::ZeroVector;

	IMUCalibrationElapsed =
		0.0f;

	bHasIMUReference =
		false;
}



void ANuigurumi::UpdateIMUTransform(
	float DeltaTime,
	const FVector& BaseLocation,
	const FRotator& BaseRotation)
{
	// IMU制御を使用しない場合
	if (!bUseIMUTiltControl)
	{
		RecenterIMU();

		SetActorLocation(
			BaseLocation);

		SetActorRotation(
			BaseRotation);

		return;
	}


	// IMU Readerを検索する
	if (!IsValid(IMUReader))
	{
		for (
			TActorIterator<ADeviceIMUReader>
			It(GetWorld());
			It;
			++It)
		{
			IMUReader = *It;
			break;
		}
	}


	// 姿勢データがまだ取得できていない場合
	if (!IsValid(IMUReader) ||
		!IMUReader->bHasDeviceOrientation)
	{
		SetActorLocation(
			BaseLocation);

		SetActorRotation(
			(
				BaseRotation.Quaternion()
				* CurrentIMUOffset
				)
			.GetNormalized());

		return;
	}


	// センサーの絶対姿勢を取得する
	//
	// QUATの場合はここでは
	// Unreal座標系へ変換しない
	const bool bUsingRawQuaternion =
		IMUReader->bHasDeviceQuaternion;


	const FQuat SensorOrientation =
		bUsingRawQuaternion
		? IMUReader
		->DeviceOrientationQuaternion
		.GetNormalized()
		: ConvertSensorOrientationToUnreal(
			IMUReader->DeviceOrientation);


	// 初期姿勢を基準姿勢として保存する
	if (!bHasIMUReference)
	{
		IMUCalibrationElapsed +=
			DeltaTime;


		if (IMUCalibrationElapsed >=
			IMUCalibrationDuration)
		{
			IMUReferenceOrientation =
				SensorOrientation;


			CurrentIMUOffset =
				FQuat::Identity;


			TargetIMUOffset =
				FQuat::Identity;


			bHasIMUReference =
				true;


			UE_LOG(
				LogTemp,
				Display,
				TEXT(
					"IMU reference orientation calibrated."
				)
			);
		}
	}


	// キャリブレーション中
	if (!bHasIMUReference)
	{
		if (bShowIMUDebug &&
			GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(
				static_cast<uint64>(
					reinterpret_cast<UPTRINT>(
						this)) + 1,
				0.0f,
				FColor::Yellow,
				FString::Printf(
					TEXT(
						"Nuigurumi IMU calibrating... %.1f / %.1f sec"
					),
					IMUCalibrationElapsed,
					IMUCalibrationDuration
				)
			);
		}


		SetActorLocation(
			BaseLocation);

		SetActorRotation(
			BaseRotation);

		return;
	}


	// センサー座標系で
	// 初期姿勢から現在姿勢までの
	// 相対回転を求める
	//
	// Reference^-1 * Current
	FQuat RelativeRotation =
		(
			IMUReferenceOrientation.Inverse()
			* SensorOrientation
			)
		.GetNormalized();


	// QUATプロトコルの場合、
	// 相対回転を求めた後で
	// Unreal座標系へ変換する
	if (bUsingRawQuaternion)
	{
		RelativeRotation =
			ConvertRelativeSensorQuaternionToUnreal(
				RelativeRotation);
	}


	FVector RotationVector =
		FVector::ZeroVector;


	// 現実のぬいぐるみの姿勢を
	// そのまま1:1で反映する
	if (bMirrorPhysicalPoseOneToOne)
	{
		TargetIMUOffset =
			RelativeRotation;

		CurrentIMUOffset =
			TargetIMUOffset;
	}
	else
	{
		// 調整モード用
		RotationVector =
			QuaternionToRotationVectorDegrees(
				RelativeRotation);


		// 小さい回転を無視する
		RotationVector.X =
			ApplyAngularDeadZone(
				RotationVector.X,
				IMUAngleDeadZone);

		RotationVector.Y =
			ApplyAngularDeadZone(
				RotationVector.Y,
				IMUAngleDeadZone);

		RotationVector.Z =
			ApplyAngularDeadZone(
				RotationVector.Z,
				IMUAngleDeadZone);


		// 各軸のスケールを適用する
		RotationVector.X *=
			IMURotationAxisScale.Roll;

		RotationVector.Y *=
			IMURotationAxisScale.Pitch;

		RotationVector.Z *=
			IMURotationAxisScale.Yaw;


		// 感度を適用する
		RotationVector *=
			IMUTiltSensitivity;


		// 最大回転角度を制限する
		const float RotationLength =
			RotationVector.Size();


		if (RotationLength >
			MaxIMUTiltDegrees &&
			RotationLength >
			KINDA_SMALL_NUMBER)
		{
			RotationVector =
				RotationVector
				.GetSafeNormal()
				* MaxIMUTiltDegrees;
		}


		// Quaternionへ戻す
		TargetIMUOffset =
			RotationVectorDegreesToQuaternion(
				RotationVector);


		// 滑らかに補間する
		const float RotationAlpha =
			1.0f -
			FMath::Exp(
				-IMURotationInterpSpeed
				* FMath::Max(
					DeltaTime,
					0.0f));


		CurrentIMUOffset =
			FQuat::Slerp(
				CurrentIMUOffset,
				TargetIMUOffset,
				FMath::Clamp(
					RotationAlpha,
					0.0f,
					1.0f))
			.GetNormalized();
	}


	CurrentIMULocationOffset =
		FVector::ZeroVector;


	SetActorLocation(
		BaseLocation);


	// カメラ基準の姿勢に
	// IMUの相対回転を追加する
	SetActorRotation(
		(
			BaseRotation.Quaternion()
			* CurrentIMUOffset
			)
		.GetNormalized()
	);


	// デバッグ表示
	if (bShowIMUDebug &&
		GEngine != nullptr)
	{
		const FRotator MirroredRotation =
			CurrentIMUOffset
			.Rotator()
			.GetNormalized();


		GEngine->AddOnScreenDebugMessage(
			static_cast<uint64>(
				reinterpret_cast<UPTRINT>(
					this)) + 1,
			0.0f,
			FColor::Yellow,
			FString::Printf(
				TEXT(
					"Nuigurumi IMU %s\n"
					"Pitch %.1f\n"
					"Yaw %.1f\n"
					"Roll %.1f"
				),
				bMirrorPhysicalPoseOneToOne
				? TEXT("1:1 QUAT")
				: TEXT("TUNED"),
				MirroredRotation.Pitch,
				MirroredRotation.Yaw,
				MirroredRotation.Roll
			)
		);
	}
}



void ANuigurumi::HandleDetectedActorChanged(
	AActor* NewActor)
{
	ClearDetectedObjectRimLight();

	if (bUseDetectedObjectRimLight &&
		IsValid(NewActor))
	{
		ApplyDetectedObjectRimLight(
			NewActor);
	}
}



void ANuigurumi::ApplyDetectedObjectRimLight(
	AActor* TargetActor)
{
	if (!IsValid(TargetActor) ||
		!IsValid(DetectedObjectPostProcessInstance))
	{
		return;
	}

	TInlineComponentArray<UMeshComponent*> SourceMeshes;
	TargetActor->GetComponents(SourceMeshes);

	for (UMeshComponent* SourceMesh : SourceMeshes)
	{
		if (!IsValid(SourceMesh) ||
			!SourceMesh->IsVisible())
		{
			continue;
		}

		FNuiRimLightMeshState& State =
			RimLightMeshStates.AddDefaulted_GetRef();

		State.HighlightedMesh = SourceMesh;
		State.bPreviousRenderCustomDepth =
			SourceMesh->bRenderCustomDepth;
		State.PreviousCustomDepthStencilValue =
			SourceMesh->CustomDepthStencilValue;

		SourceMesh->SetRenderCustomDepth(true);
		SourceMesh->SetCustomDepthStencilValue(252);
	}
}



void ANuigurumi::ClearDetectedObjectRimLight()
{
	for (const FNuiRimLightMeshState& State : RimLightMeshStates)
	{
		if (UMeshComponent* HighlightedMesh = State.HighlightedMesh.Get())
		{
			HighlightedMesh->SetRenderCustomDepth(
				State.bPreviousRenderCustomDepth);

			HighlightedMesh->SetCustomDepthStencilValue(
				State.PreviousCustomDepthStencilValue);
		}
	}

	RimLightMeshStates.Reset();
}
