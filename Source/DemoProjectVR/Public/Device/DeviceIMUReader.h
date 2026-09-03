#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeviceIMUReader.generated.h"

class WindowsSerial;

/** Reads newline-delimited gyroscope, or gyroscope and acceleration, values over USB serial. */
UCLASS(Blueprintable)
class DEMOPROJECTVR_API ADeviceIMUReader : public AActor
{
	GENERATED_BODY()

public:
	ADeviceIMUReader();
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Device|IMU")
	bool ConnectDevice();
	UFUNCTION(BlueprintCallable, Category = "Device|IMU")
	void DisconnectDevice();
	UFUNCTION(BlueprintPure, Category = "Device|IMU")
	bool IsDeviceConnected() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Serial", meta = (ClampMin = "1"))
	int32 ComPort = 3;
	/** Automatically finds a connected Seeed XIAO (USB VID 2886), so COM numbers may differ between PCs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Serial")
	bool bAutoDetectComPort = true;
	/** Retry delay when the device is connected after the game has already started. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Serial", meta = (ClampMin = "0.5"))
	float ReconnectInterval = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Serial", meta = (ClampMin = "1200"))
	int32 BaudRate = 115200;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Serial")
	bool bConnectOnBeginPlay = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Debug")
	bool bShowOnScreenDebug = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Debug")
	bool bLogReceivedLines = false;

	/** Higher values react faster. The default intentionally produces a calm debug signal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Debug", meta = (ClampMin = "0.1", ClampMax = "30.0"))
	float DebugSmoothingSpeed = 2.0f;

	/** Gyroscope changes below this magnitude are treated as sensor noise. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Debug", meta = (ClampMin = "0.0"))
	float GyroscopeDeadZone = 3.0f;

	/** Per-axis acceleration changes below this amount are hidden from the filtered display. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device|Debug", meta = (ClampMin = "0.0"))
	float AccelerationDeadZone = 0.03f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device|IMU")
	FVector Gyroscope = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device|IMU")
	FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device|IMU")
	FVector FilteredGyroscope = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device|IMU")
	FVector FilteredAcceleration = FVector::ZeroVector;

	/** Sensor-fused physical orientation received from the XIAO firmware. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device|IMU")
	FRotator DeviceOrientation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device|IMU")
	bool bHasDeviceOrientation = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device|IMU")
	FString LastReceivedLine;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device|IMU")
	int32 ParsedSampleCount = 0;

protected:
	virtual void BeginPlay() override;

private:
	void ReadAvailableSerialData();
	bool ParseIMULine(const FString& Line);
	void UpdateFilteredDebugValues(float DeltaTime);
	void DrawDebugStatus() const;
	int32 FindXiaoComPort() const;

	WindowsSerial* SerialPort = nullptr;
	FString ReceiveBuffer;
	float ReconnectElapsed = 0.0f;
};
