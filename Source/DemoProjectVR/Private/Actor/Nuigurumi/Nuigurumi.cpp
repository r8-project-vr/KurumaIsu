// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Nuigurumi/Nuigurumi.h"

#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ANuigurumi::ANuigurumi()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Collider = CreateDefaultSubobject<USphereComponent>(TEXT("Collider"));
	RootComponent = Collider;
}

// Called when the game starts or when spawned
void ANuigurumi::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ANuigurumi::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bFollowPlayerView == false)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
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
