#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "KurumaIsuGameInstance.generated.h"

UCLASS()
class DEMOPROJECTVR_API UKurumaIsuGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void StartGameInstance() override;
};
