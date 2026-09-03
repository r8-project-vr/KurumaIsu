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

void UNuiInteractionPromptWidget::BuildPrompt(
	const FText& Text,
	const FSlateFontInfo& Font,
	const FLinearColor& TextColor,
	const FLinearColor& BackgroundColor)
{
	UBorder* Background = WidgetTree
		? Cast<UBorder>(WidgetTree->FindWidget(TEXT("Background")))
		: nullptr;
	UTextBlock* PromptText = WidgetTree
		? Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("PromptText")))
		: nullptr;

	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
		PromptText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PromptText"));

		//　パネルサイズ
		Background->SetPadding(FMargin(6.0f, 3.0f));
		Background->SetContent(PromptText);
		WidgetTree->RootWidget = Background;
	}

	if (PromptText)
	{
		PromptText->SetText(Text);
		PromptText->SetFont(Font);
		PromptText->SetColorAndOpacity(FSlateColor(TextColor));
		PromptText->SetJustification(ETextJustify::Center);
	}

	if (Background)
	{
		Background->SetBrushColor(BackgroundColor);
	}
}

// Sets default values
ANuigurumi::ANuigurumi()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Collider = CreateDefaultSubobject<USphereComponent>(TEXT("Collider"));
	RootComponent = Collider;

	InteractionPromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionPromptWidget"));
	InteractionPromptWidget->SetupAttachment(Collider);
	InteractionPromptWidget->SetWidgetSpace(EWidgetSpace::World);
	InteractionPromptWidget->SetDrawAtDesiredSize(true);
	InteractionPromptWidget->SetTwoSided(true);
	InteractionPromptWidget->SetPivot(FVector2D(0.5f, 0.5f));
	InteractionPromptWidget->SetRelativeScale3D(FVector(0.75f));
	InteractionPromptWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractionPromptWidget->SetVisibility(false);
	InteractionPromptWidget->SetWidgetClass(UNuiInteractionPromptWidget::StaticClass());

	InteractionPromptText = FText::GetEmpty();
	InteractionPromptWidgetClass = UNuiInteractionPromptWidget::StaticClass();
	// フォントサイズ
	InteractionPromptFont = FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 13);
}

// Called when the game starts or when spawned
void ANuigurumi::BeginPlay()
{
	Super::BeginPlay();

	UClass* PromptWidgetClass = InteractionPromptWidgetClass
		? InteractionPromptWidgetClass.Get()
		: UNuiInteractionPromptWidget::StaticClass();
	InteractionPromptWidget->SetWidgetClass(PromptWidgetClass);

	InteractionPromptWidget->InitWidget();
	if (UNuiInteractionPromptWidget* PromptWidget = Cast<UNuiInteractionPromptWidget>(InteractionPromptWidget->GetUserWidgetObject()))
	{
		PromptWidget->BuildPrompt(
			InteractionPromptText,
			InteractionPromptFont,
			InteractionPromptTextColor,
			InteractionPromptBackgroundColor);
	}

	if (UNuiEyeSightComponent* EyeSight = FindComponentByClass<UNuiEyeSightComponent>())
	{
		EyeSight->OnDetectedActorChanged.AddDynamic(this, &ANuigurumi::HandleDetectedActorChanged);
		EyeSight->OnGimmickFocusChanged.AddDynamic(this, &ANuigurumi::HandleGimmickFocusChanged);
	}
}

// Called every frame
void ANuigurumi::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (IsValid(InteractionPromptTarget) && PlayerController != nullptr && PlayerController->PlayerCameraManager != nullptr)
	{
		FVector TargetOrigin;
		FVector TargetExtent;
		InteractionPromptTarget->GetActorBounds(false, TargetOrigin, TargetExtent);

		const FVector PromptLocation = TargetOrigin
			+ InteractionPromptOffset
			+ FVector(0.0f, 0.0f, TargetExtent.Z);
		InteractionPromptWidget->SetWorldLocation(PromptLocation);

		const FVector ToCamera = PlayerController->PlayerCameraManager->GetCameraLocation() - PromptLocation;
		if (ToCamera.IsNearlyZero() == false)
		{
			InteractionPromptWidget->SetWorldRotation(ToCamera.Rotation());
		}
	}

	if (bFollowPlayerView == false)
	{
		UpdateIMUTransform(DeltaTime, GetActorLocation() - CurrentIMULocationOffset, GetActorRotation() - CurrentIMUOffset);
		return;
	}

	if (PlayerController == nullptr || PlayerController->PlayerCameraManager == nullptr)
	{
		return;
	}

	const FVector CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
	const FRotator CameraRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
	const FRotationMatrix CameraMatrix(CameraRotation);

	const FVector TargetLocation = CameraLocation
		+ CameraRotation.Vector() * PlayerViewOffset.X
		+ CameraMatrix.GetScaledAxis(EAxis::Y) * PlayerViewOffset.Y
		+ (CameraMatrix.GetScaledAxis(EAxis::Z) * (PlayerViewOffset.Z - 30.0f));

	const FVector NewLocation = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, FollowInterpSpeed);
	if (bMatchPlayerViewRotation)
	{
		if (bUseIMUTiltControl)
		{
			// Keep the camera as the stable base. Interpolating the combined actor rotation
			// toward the camera would gradually erase the held IMU pose every frame.
			UpdateIMUTransform(DeltaTime, NewLocation, CameraRotation);
		}
		else
		{
			const FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), CameraRotation, DeltaTime, FollowInterpSpeed);
			UpdateIMUTransform(DeltaTime, NewLocation, NewRotation);
		}
	}
	else
	{
		UpdateIMUTransform(DeltaTime, NewLocation, GetActorRotation() - CurrentIMUOffset);
	}
}

void ANuigurumi::UpdateIMUTransform(float DeltaTime, const FVector& BaseLocation, const FRotator& BaseRotation)
{
	if (!bUseIMUTiltControl)
	{
		CurrentIMUOffset = FRotator::ZeroRotator;
		CurrentIMULocationOffset = FVector::ZeroVector;
		SetActorLocation(BaseLocation);
		SetActorRotation(BaseRotation);
		return;
	}

	if (!IsValid(IMUReader))
	{
		for (TActorIterator<ADeviceIMUReader> It(GetWorld()); It; ++It)
		{
			IMUReader = *It;
			break;
		}
	}

	if (!IsValid(IMUReader) || !IMUReader->bHasDeviceOrientation)
	{
		return;
	}

	if (!bHasIMUReference)
	{
		IMUCalibrationElapsed += DeltaTime;
		if (IMUCalibrationElapsed >= IMUCalibrationDuration)
		{
			IMUReferenceOrientation = IMUReader->DeviceOrientation.Quaternion();
			bHasIMUReference = true;
		}
	}
	if (!bHasIMUReference)
	{
		if (bShowIMUDebug && GEngine != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(
				static_cast<uint64>(reinterpret_cast<UPTRINT>(this)) + 1, 0.0f, FColor::Yellow,
				FString::Printf(TEXT("Nuigurumi IMU calibrating... %.1f / %.1f sec"), IMUCalibrationElapsed, IMUCalibrationDuration));
		}
		return;
	}

	const FQuat CurrentOrientation = IMUReader->DeviceOrientation.Quaternion();
	const FRotator RelativeOrientation = (CurrentOrientation * IMUReferenceOrientation.Inverse()).Rotator().GetNormalized();
	const FRotator MeasuredOffset(
		FMath::Clamp(RelativeOrientation.Pitch * IMUTiltSensitivity, -MaxIMUTiltDegrees, MaxIMUTiltDegrees),
		FMath::Clamp(-RelativeOrientation.Yaw * IMUTiltSensitivity, -MaxIMUTiltDegrees, MaxIMUTiltDegrees),
		FMath::Clamp(-RelativeOrientation.Roll * IMUTiltSensitivity, -MaxIMUTiltDegrees, MaxIMUTiltDegrees));
	if (IMUReader->Gyroscope.Size() >= IMUMotionThreshold)
	{
		HeldIMUTarget = MeasuredOffset;
	}

	CurrentIMUOffset = FMath::RInterpTo(CurrentIMUOffset, HeldIMUTarget, DeltaTime, IMURotationInterpSpeed);
	CurrentIMULocationOffset = FVector::ZeroVector;
	SetActorLocation(BaseLocation);
	SetActorRotation(BaseRotation + CurrentIMUOffset);

	if (bShowIMUDebug && GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(
			static_cast<uint64>(reinterpret_cast<UPTRINT>(this)) + 1,
			0.0f,
			FColor::Yellow,
			FString::Printf(
				TEXT("Nuigurumi Pose  Pitch %.1f  Yaw %.1f  Roll %.1f"),
				CurrentIMUOffset.Pitch, CurrentIMUOffset.Yaw, CurrentIMUOffset.Roll));
	}
}

void ANuigurumi::HandleDetectedActorChanged(AActor* NewActor)
{
	InteractionPromptTarget = NewActor;

	if (IsValid(InteractionPromptTarget) == false)
	{
		InteractionPromptWidget->SetVisibility(false);
	}
}

void ANuigurumi::HandleGimmickFocusChanged(bool bCanAction)
{
	InteractionPromptWidget->SetVisibility(bCanAction && IsValid(InteractionPromptTarget));
}
