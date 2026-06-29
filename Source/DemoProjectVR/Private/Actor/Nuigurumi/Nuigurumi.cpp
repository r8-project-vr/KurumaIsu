// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Nuigurumi/Nuigurumi.h"

#include "Actor/Nuigurumi/NuiEyeSightComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/SphereComponent.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"

void UNuiInteractionPromptWidget::BuildPrompt(
	const FText& Text,
	const FSlateFontInfo& Font,
	const FLinearColor& TextColor,
	const FLinearColor& BackgroundColor)
{
	UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
	UTextBlock* PromptText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PromptText"));

	PromptText->SetText(Text);
	PromptText->SetFont(Font);
	PromptText->SetColorAndOpacity(FSlateColor(TextColor));
	PromptText->SetJustification(ETextJustify::Center);

	Background->SetBrushColor(BackgroundColor);
	Background->SetPadding(FMargin(14.0f, 7.0f));
	Background->SetContent(PromptText);
	WidgetTree->RootWidget = Background;
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
	InteractionPromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionPromptWidget->SetDrawAtDesiredSize(true);
	InteractionPromptWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractionPromptWidget->SetVisibility(false);
	InteractionPromptWidget->SetWidgetClass(UNuiInteractionPromptWidget::StaticClass());

	InteractionPromptText = FText::FromString(TEXT("작동"));
	InteractionPromptFont = FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 24);
}

// Called when the game starts or when spawned
void ANuigurumi::BeginPlay()
{
	Super::BeginPlay();

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
	}

	if (bFollowPlayerView == false)
	{
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
		+ CameraMatrix.GetScaledAxis(EAxis::Z) * PlayerViewOffset.Z;

	const FVector NewLocation = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, FollowInterpSpeed);
	SetActorLocation(NewLocation);

	if (bMatchPlayerViewRotation)
	{
		const FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), CameraRotation, DeltaTime, FollowInterpSpeed);
		SetActorRotation(NewRotation);
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
