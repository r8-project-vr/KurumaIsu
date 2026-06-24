#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BleDeviceInterface.generated.h"

DECLARE_DELEGATE(FBleDelegate);
DECLARE_DELEGATE_OneParam(FBleErrorDelegate, FString);
DECLARE_DELEGATE_TwoParams(FBleCharacteristicDelegate, FString, FString);
DECLARE_DELEGATE_ThreeParams(FBleCharacteristicDataDelegate, FString, FString, TArray<uint8>&);

UINTERFACE(BlueprintType)
class BLEGOODIES_API UBleDeviceInterface : public UInterface
{
    GENERATED_BODY()
};

class BLEGOODIES_API IBleDeviceInterface
{
    GENERATED_BODY()

public:
    virtual FString GetDeviceName() { return FString(); }
    virtual FString GetDeviceId() { return FString(); }
    virtual void Connect(FBleDelegate Success, FBleErrorDelegate Error) {}
    virtual void Disconnect(FBleDelegate Success, FBleErrorDelegate Error) {}
    virtual void WriteCharacteristic(const FString& ServiceUUID, const FString& CharacteristicUUID, const TArray<uint8>& Data, FBleDelegate Success, FBleErrorDelegate Error) {}
    virtual void SubscribeToCharacteristic(const FString& ServiceUUID, const FString& CharacteristicUUID, bool bWithResponse, FBleDelegate Success, FBleErrorDelegate Error) {}
    virtual void ReadCharacteristic(const FString& ServiceUUID, const FString& CharacteristicUUID, FBleDelegate Success, FBleErrorDelegate Error) {}
    virtual void BindToCharacteristicWriteEvent(FBleCharacteristicDelegate Delegate) {}
    virtual void BindToCharacteristicNotificationEvent(FBleCharacteristicDataDelegate Delegate) {}
};
