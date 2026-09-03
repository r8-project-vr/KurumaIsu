// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Nuigurumi/Nuigurumi.h"

#include "Actor/Nuigurumi/NuiEyeSightComponent.h"
#include "Device/DeviceIMUReader.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/SphereComponent.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "EngineUtils.h"


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



void UNuiInteractionPromptWidget::BuildPrompt(
	const FText& Text,
	const FSlateFontInfo& Font,
	const FLinearColor& TextColor,
	const FLinearColor& BackgroundColor)
{
	UBorder* Background = WidgetTree
		? Cast<UBorder>(
			WidgetTree->FindWidget(
				TEXT("Background")))
		: nullptr;

	UTextBlock* PromptText = WidgetTree
		? Cast<UTextBlock>(
			WidgetTree->FindWidget(
				TEXT("PromptText")))
		: nullptr;


	if (WidgetTree &&
		WidgetTree->RootWidget == nullptr)
	{
		Background =
			WidgetTree->ConstructWidget<UBorder>(
				UBorder::StaticClass(),
				TEXT("Background"));

		PromptText =
			WidgetTree->ConstructWidget<UTextBlock>(
				UTextBlock::StaticClass(),
				TEXT("PromptText"));

		// パネルサイズ
		Background->SetPadding(
			FMargin(6.0f, 3.0f));

		Background->SetContent(
			PromptText);

		WidgetTree->RootWidget =
			Background;
	}


	if (PromptText)
	{
		PromptText->SetText(
			Text);

		PromptText->SetFont(
			Font);

		PromptText->SetColorAndOpacity(
			FSlateColor(TextColor));

		PromptText->SetJustification(
			ETextJustify::Center);
	}


	if (Background)
	{
		Background->SetBrushColor(
			BackgroundColor);
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


	InteractionPromptWidget =
		CreateDefaultSubobject<UWidgetComponent>(
			TEXT("InteractionPromptWidget"));

	InteractionPromptWidget
		->SetupAttachment(
			Collider);

	InteractionPromptWidget
		->SetWidgetSpace(
			EWidgetSpace::World);

	InteractionPromptWidget
		->SetDrawAtDesiredSize(
			true);

	InteractionPromptWidget
		->SetTwoSided(
			true);

	InteractionPromptWidget
		->SetPivot(
			FVector2D(
				0.5f,
				0.5f));

	InteractionPromptWidget
		->SetRelativeScale3D(
			FVector(0.75f));

	InteractionPromptWidget
		->SetCollisionEnabled(
			ECollisionEnabled::NoCollision);

	InteractionPromptWidget
		->SetVisibility(
			false);

	InteractionPromptWidget
		->SetWidgetClass(
			UNuiInteractionPromptWidget::StaticClass());


	InteractionPromptText =
		FText::GetEmpty();

	InteractionPromptWidgetClass =
		UNuiInteractionPromptWidget::StaticClass();

	// フォントサイズ
	InteractionPromptFont =
		FCoreStyle::GetDefaultFontStyle(
			TEXT("Regular"),
			13);
}



// Called when the game starts or when spawned
void ANuigurumi::BeginPlay()
{
	Super::BeginPlay();


	UClass* PromptWidgetClass =
		InteractionPromptWidgetClass
		? InteractionPromptWidgetClass.Get()
		: UNuiInteractionPromptWidget::StaticClass();


	InteractionPromptWidget
		->SetWidgetClass(
			PromptWidgetClass);


	InteractionPromptWidget
		->InitWidget();


	if (UNuiInteractionPromptWidget* PromptWidget =
		Cast<UNuiInteractionPromptWidget>(
			InteractionPromptWidget
			->GetUserWidgetObject()))
	{
		PromptWidget->BuildPrompt(
			InteractionPromptText,
			InteractionPromptFont,
			InteractionPromptTextColor,
			InteractionPromptBackgroundColor);
	}


	if (UNuiEyeSightComponent* EyeSight =
		FindComponentByClass<
		UNuiEyeSightComponent>())
	{
		EyeSight
			->OnDetectedActorChanged
			.AddDynamic(
				this,
				&ANuigurumi::HandleDetectedActorChanged);

		EyeSight
			->OnGimmickFocusChanged
			.AddDynamic(
				this,
				&ANuigurumi::HandleGimmickFocusChanged);
	}
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


	if (IsValid(InteractionPromptTarget) &&
		PlayerController != nullptr &&
		PlayerController->PlayerCameraManager != nullptr)
	{
		FVector TargetOrigin;
		FVector TargetExtent;

		InteractionPromptTarget
			->GetActorBounds(
				false,
				TargetOrigin,
				TargetExtent);


		const FVector PromptLocation =
			TargetOrigin
			+ InteractionPromptOffset
			+ FVector(
				0.0f,
				0.0f,
				TargetExtent.Z);


		InteractionPromptWidget
			->SetWorldLocation(
				PromptLocation);


		const FVector ToCamera =
			PlayerController
			->PlayerCameraManager
			->GetCameraLocation()
			- PromptLocation;


		if (!ToCamera.IsNearlyZero())
		{
			InteractionPromptWidget
				->SetWorldRotation(
					ToCamera.Rotation());
		}
	}


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
	InteractionPromptTarget =
		NewActor;


	if (!IsValid(
		InteractionPromptTarget))
	{
		InteractionPromptWidget
			->SetVisibility(
				false);
	}
}



void ANuigurumi::HandleGimmickFocusChanged(
	bool bCanAction)
{
	InteractionPromptWidget
		->SetVisibility(
			bCanAction &&
			IsValid(
				InteractionPromptTarget));
}