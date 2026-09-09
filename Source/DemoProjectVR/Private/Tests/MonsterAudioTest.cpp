#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Actor/Monster/Monster.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Sound/SoundWave.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMonsterAudioTest, "Project.Monster.AudioTransitions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMonsterAudioTest::RunTest(const FString& Parameters)
{
	UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::Editor) { World = Context.World(); break; }
	}
	if (!TestNotNull(TEXT("Editor world"), World)) return false;
	UClass* MonsterClass = LoadClass<AMonster>(nullptr, TEXT("/Game/Characters/Monster/BPMonster.BPMonster_C"));
	if (!TestNotNull(TEXT("Monster blueprint"), MonsterClass)) return false;
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.ObjectFlags |= RF_Transient;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AMonster* Monster = World->SpawnActor<AMonster>(MonsterClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	if (!TestNotNull(TEXT("Test monster"), Monster)) return false;
	ON_SCOPE_EXIT { Monster->Destroy(); };
	UAudioComponent* Loop = nullptr;
	UAudioComponent* Transition = nullptr;
	TArray<UAudioComponent*> Components;
	Monster->GetComponents(Components);
	for (UAudioComponent* Audio : Components)
	{
		if (Audio->GetFName() == TEXT("MovementAudio")) Loop = Audio;
		if (Audio->GetFName() == TEXT("TransitionAudio")) Transition = Audio;
	}
	if (!TestNotNull(TEXT("Loop component"), Loop) || !TestNotNull(TEXT("Transition component"), Transition)) return false;
	const auto Step = [Monster](float Speed)
	{
		Monster->GetCharacterMovement()->Velocity = FVector(Speed, 0.0f, 0.0f);
		Monster->Tick(1.0f / 60.0f);
	};
	Step(0.0f);
	TestFalse(TEXT("No stop sound at initial idle"), Transition->IsPlaying());
	TestFalse(TEXT("No movement loop at initial idle"), Loop->IsPlaying());
	Step(100.0f);
	TestTrue(TEXT("Start sound on movement"), Transition->IsPlaying());
	TestTrue(TEXT("Movement loop starts"), Loop->IsPlaying());
	TestTrue(TEXT("Correct start/stop clip"), Transition->Sound && Transition->Sound->GetPathName() == TEXT("/Game/Sound/SFX/MonsterFirst.MonsterFirst"));
	USoundWave* Wave = Cast<USoundWave>(Loop->Sound);
	TestTrue(TEXT("Correct looping clip"), Wave && Wave->bLooping && Wave->GetPathName() == TEXT("/Game/Sound/SFX/MonsterSound.MonsterSound"));
	TestTrue(TEXT("Loop follows monster"), Loop->GetAttachParent() == Monster->GetRootComponent());
	Transition->Stop();
	Step(100.0f);
	TestFalse(TEXT("Start sound does not repeat every tick"), Transition->IsPlaying());
	Step(7.0f);
	TestTrue(TEXT("Small speed fluctuations do not stop loop"), Loop->IsPlaying());
	Step(0.0f);
	TestFalse(TEXT("Stop ends the loop"), Loop->IsPlaying());
	TestTrue(TEXT("Stop plays transition sound"), Transition->IsPlaying());
	Transition->Stop();
	Step(0.0f);
	TestFalse(TEXT("Idle does not repeat stop sound"), Transition->IsPlaying());
	Step(7.0f);
	TestFalse(TEXT("Idle jitter does not start movement sound"), Loop->IsPlaying());
	Step(100.0f);
	TestTrue(TEXT("Resuming restarts loop"), Loop->IsPlaying());
	TestTrue(TEXT("Resuming plays start sound"), Transition->IsPlaying());
	return true;
}

#endif
