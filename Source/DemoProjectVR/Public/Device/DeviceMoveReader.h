// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DebugHelper.h"
#include "../../../../Plugins/ASerialCom/Source/ASerialCom/Public/ASerialLibControllerWin.h"

#if PLATFORM_WINDOWS
#include "WindowsSerial/WindowsSerial.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <setupapi.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#include "DeviceMoveReader.generated.h"

class AMoveInput;

enum class EDeviceRequest
{
	None,
	UpdateFlag,
	RPS
};

UCLASS()
class DEMOPROJECTVR_API ADeviceMoveReader : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADeviceMoveReader();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	//virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Device|IMU")
	bool ConnectDevice();
	UFUNCTION(BlueprintCallable, Category = "Device|IMU")
	void DisconnectDevice();
	UFUNCTION(BlueprintPure, Category = "Device|IMU")
	bool IsDeviceConnected() const;

	void SetMoveInput(AMoveInput* moveinput);

	void SendDeviceValue();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Serial", meta = (ClampMin = "1"))
	int TargetDeviceID = 0x03;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Serial", meta = (ClampMin = "1"))
	int DeviceVersion = 0x02;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Serial", meta = (ClampMin = "1"))
	int32 ComPort = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Serial", meta = (ClampMin = "1200"))
	int32 BaudRate = 115200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Serial")
	bool bConnectOnBeginPlay = true;
	

	/** Automatically finds a connected Seeed XIAO (USB VID 2886), so COM numbers may differ between PCs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Serial")
	bool bAutoDetectComPort = true;

	/** Retry delay when the device is connected after the game has already started. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Serial", meta = (ClampMin = "0.5"))
	float ReconnectInterval = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Debug")
	bool bShowOnScreenDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Debug")
	bool bLogReceivedLines = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device|IMU")
	FString LastReceivedLine;

	// 車輪のデバイスのシリアル番号（確認方法：デバイスマネージャーからデバイスID確認）
	// デバイス[3]：15bf3a9
	// デバイス[4]：5c03106
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Device|SerialNum")
	FString DeviceSirialNumber = "5c03106";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Device|SerialNum")
	bool bInputInversion = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void ReadDataProcess();
	void RequestUpdateFlag();
	void RequestRPS();
	int32 FindXiaoComPort() const;

	UASerialLibControllerWin* SerialController = nullptr;
	WindowsSerial* SerialInterface = nullptr;
	FString ReceiveBuffer;
	bool bDeviceConnected = false;
	u_int Command = 0x21;

	float PollingTimer = 0.0f;
	float PollingInterval = 0.01f;
	bool bWaitingForResponse = false;
	EDeviceRequest CurrentRequest = EDeviceRequest::None;
	float CurrentRPS = 0.0f;
	AMoveInput* MoveInput = nullptr;
};
