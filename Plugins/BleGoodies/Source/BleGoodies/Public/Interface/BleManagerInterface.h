#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interface/BleDeviceInterface.h"
#include "BleManagerInterface.generated.h"

DECLARE_DELEGATE_OneParam(FBleOnDeviceFoundDelegate, TScriptInterface<IBleDeviceInterface>);

UINTERFACE(BlueprintType)
class BLEGOODIES_API UBleManagerInterface : public UInterface
{
    GENERATED_BODY()
};

class BLEGOODIES_API IBleManagerInterface
{
    GENERATED_BODY()

public:
    virtual bool IsBleSupported() { return false; }
    virtual bool IsBluetoothEnabled() { return false; }
    virtual void SetBluetoothState(bool bEnabled) {}
    virtual void ScanForDevices(const TArray<FString>& ServiceUUIDs, FBleOnDeviceFoundDelegate OnDeviceFound) {}
    virtual void StopScan() {}
};
