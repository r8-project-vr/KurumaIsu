#include "KurumaIsuGameInstance.h"

#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"

void UKurumaIsuGameInstance::StartGameInstance()
{
	if (!GIsEditor && !IsRunningDedicatedServer() && GEngine)
	{
		if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
		{
			// Apply the packaged game's Medium preset even when an older saved setting exists.
			UserSettings->SetOverallScalabilityLevel(1);
			UserSettings->ApplyNonResolutionSettings();
			UserSettings->SaveSettings();
			UE_LOG(LogTemp, Display, TEXT("Game scalability preset: %d (Medium=1)"), UserSettings->GetOverallScalabilityLevel());
		}
	}

	Super::StartGameInstance();
}
