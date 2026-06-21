#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Interface/BleManagerInterface.h"
#include "BleUtils.generated.h"

UCLASS()
class BLEGOODIES_API UBleUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Bluetooth")
    static TScriptInterface<IBleManagerInterface> CreateBleManager()
    {
        return TScriptInterface<IBleManagerInterface>();
    }
};
