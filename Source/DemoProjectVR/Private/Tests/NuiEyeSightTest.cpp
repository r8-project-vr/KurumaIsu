#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Actor/Nuigurumi/NuiEyeSightComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNuiEyeSightSurfaceTest, "Project.Nuigurumi.SurfaceDetection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNuiEyeSightSurfaceTest::RunTest(const FString& Parameters)
{
	UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::Editor)
		{
			World = Context.World();
			break;
		}
	}
	if (!TestNotNull(TEXT("Editor world"), World)) return false;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.ObjectFlags |= RF_Transient;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector Origin(100000.0, 100000.0, 100000.0);
	AActor* Owner = World->SpawnActor<AActor>(SpawnParameters);
	AActor* Target = World->SpawnActor<AActor>(SpawnParameters);
	ON_SCOPE_EXIT
	{
		if (Owner) Owner->Destroy();
		if (Target) Target->Destroy();
	};
	if (!TestNotNull(TEXT("Owner"), Owner) || !TestNotNull(TEXT("Target"), Target)) return false;

	const auto AddRoot = [](AActor* Actor)
	{
		USceneComponent* Root = NewObject<USceneComponent>(Actor);
		Actor->AddInstanceComponent(Root);
		Actor->SetRootComponent(Root);
		Root->RegisterComponent();
	};
	AddRoot(Owner);
	AddRoot(Target);
	Owner->SetActorLocation(Origin);
	Target->SetActorLocation(Origin + FVector(1000.0, 1000.0, 0.0));
	Target->Tags.Add(TEXT("NuiSurfaceTest"));

	UNuiEyeSightComponent* Sight = NewObject<UNuiEyeSightComponent>(Owner);
	Owner->AddInstanceComponent(Sight);
	Sight->RegisterComponent();
	Sight->SightDistance = 500.0f;
	Sight->SightAngle = 60.0f;
	Sight->TargetTag = TEXT("NuiSurfaceTest");
	Sight->bDetectGimmickInterface = false;
	Sight->bShowDetectionDebug = false;

	UBoxComponent* Box = NewObject<UBoxComponent>(Target);
	Target->AddInstanceComponent(Box);
	Box->SetupAttachment(Target->GetRootComponent());
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionObjectType(ECC_WorldDynamic);
	Box->SetCollisionResponseToAllChannels(ECR_Overlap);
	Box->SetGenerateOverlapEvents(false); // Explicit queries must not depend on overlap events.
	Box->RegisterComponent();
	const auto PlaceBox = [&](const FVector& Center, const FVector& Extent)
	{
		Box->SetBoxExtent(Extent);
		Box->SetWorldLocation(Origin + Center);
	};
	const auto ExpectTarget = [&](const TCHAR* Description, bool bExpected)
	{
		Sight->DetectObject();
		TestTrue(Description, bExpected ? Sight->GetDetectedActor() == Target : Sight->GetDetectedActor() == nullptr);
	};

	PlaceBox(FVector(300.0, 0.0, 0.0), FVector(20.0));
	ExpectTarget(TEXT("Offset component is detected even though actor pivot is outside range and angle"), true);
	PlaceBox(FVector(550.0, 0.0, 0.0), FVector(60.0, 20.0, 20.0));
	ExpectTarget(TEXT("Surface inside range with component center outside range"), true);
	PlaceBox(FVector(300.0, 230.0, 0.0), FVector(30.0, 100.0, 20.0));
	ExpectTarget(TEXT("Surface inside cone with component center outside angle"), true);
	PlaceBox(FVector(150.0, 120.0, 0.0), FVector(140.0, 20.0, 20.0));
	ExpectTarget(TEXT("Long side surface intersects cone although its nearest point is outside angle"), true);
	PlaceBox(FVector(300.0, 300.0, 0.0), FVector(20.0));
	ExpectTarget(TEXT("Surface entirely outside cone is rejected"), false);
	PlaceBox(FVector(530.0, 0.0, 0.0), FVector(20.0));
	ExpectTarget(TEXT("Surface entirely beyond range is rejected"), false);
	PlaceBox(FVector(-100.0, 0.0, 0.0), FVector(20.0));
	ExpectTarget(TEXT("Object behind doll is rejected"), false);
	PlaceBox(FVector::ZeroVector, FVector(20.0));
	ExpectTarget(TEXT("Origin inside collision is detected without normalizing a zero vector"), true);
	PlaceBox(FVector(300.0, 0.0, 0.0), FVector(20.0));
	Target->Tags.Reset();
	ExpectTarget(TEXT("Untagged non-gimmick remains excluded"), false);
	Target->Tags.Add(TEXT("NuiSurfaceTest"));
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ExpectTarget(TEXT("Disabled query collision remains excluded"), false);
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ExpectTarget(TEXT("Restoring query collision restores detection"), true);
	Owner->SetActorRotation(FRotator(0.0, 180.0, 0.0));
	ExpectTarget(TEXT("Turning away clears previous detection"), false);
	return true;
}

#endif
