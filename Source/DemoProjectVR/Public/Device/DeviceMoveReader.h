// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DebugHelper.h"

#if PLATFORM_WINDOWS
#include "WindowsSerial/WindowsSerial.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <setupapi.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#include "DeviceMoveReader.generated.h"

UCLASS()
class DEMOPROJECTVR_API ADeviceMoveReader : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADeviceMoveReader();
	virtual void Tick(float DeltaTime) override;
	//virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Device|IMU")
	bool ConnectDevice();
	UFUNCTION(BlueprintCallable, Category = "Device|IMU")
	void DisconnectDevice();
	//UFUNCTION(BlueprintPure, Category = "Device|IMU")
	//bool IsDeviceConnected() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Serial", meta = (ClampMin = "1"))
	int32 ComPort = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Serial", meta = (ClampMin = "1200"))
	int32 BaudRate = 115200;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Serial")
	bool bConnectOnBeginPlay = true;
	/** Automatically finds a connected Seeed XIAO (USB VID 2886), so COM numbers may differ between PCs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Serial")
	bool bAutoDetectComPort = true;
	///** Retry delay when the device is connected after the game has already started. */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Serial", meta = (ClampMin = "0.5"))
	//float ReconnectInterval = 2.0f;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Debug")
	//bool bShowOnScreenDebug = true;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Debug")
	//bool bLogReceivedLines = false;

	///** Higher values react faster. The default intentionally produces a calm debug signal. */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Debug", meta = (ClampMin = "0.1", ClampMax = "30.0"))
	//float DebugSmoothingSpeed = 2.0f;

	///** Gyroscope changes below this magnitude are treated as sensor noise. */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Debug", meta = (ClampMin = "0.0"))
	//float GyroscopeDeadZone = 3.0f;

	///** Per-axis acceleration changes below this amount are hidden from the filtered display. */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Debug", meta = (ClampMin = "0.0"))
	//float AccelerationDeadZone = 0.03f;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device|IMU")
	//FVector Gyroscope = FVector::ZeroVector;
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device|IMU")
	//FVector Acceleration = FVector::ZeroVector;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device|IMU")
	//FVector FilteredGyroscope = FVector::ZeroVector;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device|IMU")
	//FVector FilteredAcceleration = FVector::ZeroVector;

	///** Sensor-fused physical orientation received from the XIAO firmware. */
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device|IMU")
	//FRotator DeviceOrientation = FRotator::ZeroRotator;

	///** Raw fused orientation from a QUAT,W,X,Y,Z packet. */
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device|IMU")
	//FQuat DeviceOrientationQuaternion = FQuat::Identity;

	
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device|IMU")
	//FString LastReceivedLine;
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device|IMU")
	//int32 ParsedSampleCount = 0;

	// 車輪のデバイスのシリアル番号（確認方法：デバイスマネージャーからデバイスID確認）
	// デバイス[3]：15bf3a9
	// デバイス[4]：5c03106
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device|SerialNum")
	FString DeviceSirialNumber = "5c03106";

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	//void ReadAvailableSerialData();
	//bool ParseIMULine(const FString& Line);
	//void UpdateFilteredDebugValues(float DeltaTime);
	//void DrawDebugStatus() const;
	int32 FindXiaoComPort() const;

	WindowsSerial* SerialPort = nullptr;
	FString ReceiveBuffer;
	//float ReconnectElapsed = 0.0f;
public:	

};
