#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Actor/Gimmick/IElevator.h"
#include "Actor/Gimmick/IElevatorButton.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sound/SoundBase.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FElevatorAudioTest, "Project.Elevator.AudioTransitions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FElevatorAudioTest::RunTest(const FString& Parameters)
{
	TGuardValue<bool> ScriptExecutionGuard(GAllowActorScriptExecutionInEditor, true);
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

	UClass* ElevatorClass = LoadClass<AIElevator>(nullptr, TEXT("/Game/Yoshida/BP_Elevator.BP_Elevator_C"));
	if (!TestNotNull(TEXT("Elevator blueprint"), ElevatorClass)) return false;
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.ObjectFlags |= RF_Transient;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIElevator* Elevator = World->SpawnActor<AIElevator>(ElevatorClass, FVector::ZeroVector,
		FRotator::ZeroRotator, SpawnParameters);
	if (!TestNotNull(TEXT("Test elevator"), Elevator)) return false;
	ON_SCOPE_EXIT { Elevator->Destroy(); };

	UAudioComponent* Engine = Elevator->FindComponentByClass<UAudioComponent>();
	if (!TestNotNull(TEXT("Engine audio component"), Engine)) return false;
	TestFalse(TEXT("Engine is silent before a call"), Engine->IsPlaying());
	TestTrue(TEXT("Engine follows elevator"), Engine->GetAttachParent() == Elevator->GetRootComponent());

	const auto RequestFloor = [Elevator](int32 Next)
	{
		struct { int32 Next; bool ReturnValue; } Params{Next, false};
		Elevator->ProcessEvent(Elevator->FindFunctionChecked(TEXT("MoveSet")), &Params);
		return Params.ReturnValue;
	};
	const auto Act = [Elevator]()
	{
		Elevator->ProcessEvent(Elevator->FindFunctionChecked(TEXT("Action")), nullptr);
	};
	const auto DoorSounds = [Elevator]()
	{
		TArray<UAudioComponent*> Components;
		Elevator->GetComponents(Components);
		int32 Count = 0;
		for (const UAudioComponent* Component : Components)
		{
			if (Component->Sound && Component->Sound->GetPathName() == TEXT("/Game/Sound/SFX/ElevatorDoor.ElevatorDoor"))
			{
				++Count;
			}
		}
		return Count;
	};
	FFloatProperty* MoveTimeProperty = FindFProperty<FFloatProperty>(ElevatorClass, TEXT("moveTime"));
	if (!TestNotNull(TEXT("Movement duration property"), MoveTimeProperty)) return false;
	const float Step = MoveTimeProperty->GetPropertyValue_InContainer(Elevator) + 0.1f;
	Elevator->floorMin = 1;
	Elevator->floorMax = 3;
	Elevator->floor = 3;
	TestTrue(TEXT("Request descent"), RequestFloor(1));
	Act();
	TestTrue(TEXT("Engine starts on descent"), Engine->IsPlaying());
	TestTrue(TEXT("Correct engine clip"), Engine->Sound &&
		Engine->Sound->GetPathName() == TEXT("/Game/Sound/SFX/ElevatorEngine.ElevatorEngine"));
	TestEqual(TEXT("No opening sound during descent"), DoorSounds(), 0);

	Elevator->Tick(Step);
	TestEqual(TEXT("Intermediate floor reached"), Elevator->floor, 2);
	TestTrue(TEXT("Engine continues between floors"), Engine->IsPlaying());
	Elevator->Tick(Step);
	TestEqual(TEXT("Destination reached"), Elevator->floor, 1);
	TestFalse(TEXT("Engine stops at destination"), Engine->IsPlaying());
	TestEqual(TEXT("One door sound at opening"), DoorSounds(), 1);
	Act();
	TestEqual(TEXT("Repeated call while opening does not repeat sound"), DoorSounds(), 1);
	Elevator->Tick(Step);

	TestTrue(TEXT("Request ascent"), RequestFloor(3));
	Act();
	TestFalse(TEXT("Engine silent while closing"), Engine->IsPlaying());
	TestEqual(TEXT("Closing does not play opening sound"), DoorSounds(), 1);
	Elevator->Tick(Step);
	TestFalse(TEXT("Engine silent during ascent"), Engine->IsPlaying());
	Elevator->Tick(Step);
	Elevator->Tick(Step);
	TestEqual(TEXT("Upper destination reached"), Elevator->floor, 3);
	TestFalse(TEXT("Engine silent at upper destination"), Engine->IsPlaying());
	Elevator->Tick(Step);

	RequestFloor(1);
	Act();
	TestFalse(TEXT("Descent waits for doors to close"), Engine->IsPlaying());
	Elevator->Tick(Step);
	TestTrue(TEXT("Engine starts after doors close"), Engine->IsPlaying());

	// Exercise the real button blueprint entry point without saving any level.
	UClass* ButtonClass = LoadClass<AIElevatorButton>(nullptr, TEXT("/Game/Yoshida/BP_ElevatorButton.BP_ElevatorButton_C"));
	if (!TestNotNull(TEXT("Button blueprint"), ButtonClass)) return false;
	AIElevatorButton* Button = World->SpawnActor<AIElevatorButton>(ButtonClass, FVector::ZeroVector,
		FRotator::ZeroRotator, SpawnParameters);
	if (!TestNotNull(TEXT("Test button"), Button)) return false;
	ON_SCOPE_EXIT { Button->Destroy(); };
	FObjectPropertyBase* TargetProperty = FindFProperty<FObjectPropertyBase>(ButtonClass, TEXT("elevatorActor"));
	if (!TestNotNull(TEXT("Button target property"), TargetProperty)) return false;
	TargetProperty->SetObjectPropertyValue_InContainer(Button, nullptr);
	Button->ProcessEvent(Button->FindFunctionChecked(TEXT("Action")), nullptr);
	TargetProperty->SetObjectPropertyValue_InContainer(Button, Elevator);
	Button->ProcessEvent(Button->FindFunctionChecked(TEXT("Action")), nullptr);
	FObjectPropertyBase* SoundProperty = FindFProperty<FObjectPropertyBase>(ButtonClass, TEXT("ElevatorButtonSound"));
	if (!TestNotNull(TEXT("Button sound property"), SoundProperty)) return false;
	UObject* ButtonSound = SoundProperty->GetObjectPropertyValue_InContainer(Button);
	TestTrue(TEXT("Correct button clip"), ButtonSound &&
		ButtonSound->GetPathName() == TEXT("/Game/Sound/SFX/ElevatorButton.ElevatorButton"));
	return true;
}

#endif
