// Fill out your copyright notice in the Description page of Project Settings.


#include "Device/DeviceMoveReader.h"

// Sets default values
ADeviceMoveReader::ADeviceMoveReader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADeviceMoveReader::BeginPlay()
{
	Super::BeginPlay();
	if (bConnectOnBeginPlay) { ConnectDevice(); }
}

// Called every frame
void ADeviceMoveReader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ADeviceMoveReader::ConnectDevice()
{
#if PLATFORM_WINDOWS
	DisconnectDevice();
	if (bAutoDetectComPort)
	{
		const int32 DetectedPort = FindXiaoComPort();
		if (DetectedPort <= 0)
		{
			DEBUG_PRINT("Seeed XIAO USB serial device is not connected.");
			return false;
		}
		ComPort = DetectedPort;
	}
	SerialPort = new WindowsSerial(BaudRate);
	const int32 Result = SerialPort->OpenPort(ComPort);
	if (Result != 0)
	{
		DEBUG_PRINT("Failed to open COM%d at %d baud (error %d).", ComPort, BaudRate, Result);
		delete SerialPort;
		SerialPort = nullptr;
		return false;
	}
	if (!SerialPort->SetControlSignals(true, true))
	{
		DEBUG_PRINT("COM%d opened, but enabling DTR/RTS failed.", ComPort);
	}
	SerialPort->clear();
	ReceiveBuffer.Reset();
	DEBUG_PRINT("Connected to IMU device on COM%d at %d baud.", ComPort, BaudRate);
	return true;
#else
	DEBUG_PRINT("DeviceMoveReader currently supports Windows only.");
	return false;
#endif
}

void ADeviceMoveReader::DisconnectDevice()
{
#if PLATFORM_WINDOWS
	if (SerialPort != nullptr)
	{
		if (SerialPort->GetState()) { SerialPort->ClosePort(); }
		delete SerialPort;
		SerialPort = nullptr;
		DEBUG_PRINT("Move device disconnected.");
	}
#endif
	ReceiveBuffer.Reset();
}

int32 ADeviceMoveReader::FindXiaoComPort() const
{
#if PLATFORM_WINDOWS
	GUID PortClassGuids[8];
	DWORD RequiredGuids = 0;
	if (!SetupDiClassGuidsFromNameW(L"Ports", PortClassGuids, UE_ARRAY_COUNT(PortClassGuids), &RequiredGuids))
	{
		return INDEX_NONE;
	}

	for (DWORD GuidIndex = 0; GuidIndex < FMath::Min<DWORD>(RequiredGuids, UE_ARRAY_COUNT(PortClassGuids)); ++GuidIndex)
	{
		HDEVINFO DeviceInfoSet = SetupDiGetClassDevsW(&PortClassGuids[GuidIndex], nullptr, nullptr, DIGCF_PRESENT);
		if (DeviceInfoSet == INVALID_HANDLE_VALUE) { continue; }

		SP_DEVINFO_DATA DeviceInfo = {};
		DeviceInfo.cbSize = sizeof(DeviceInfo);
		for (DWORD DeviceIndex = 0; SetupDiEnumDeviceInfo(DeviceInfoSet, DeviceIndex, &DeviceInfo); ++DeviceIndex)
		{
			WCHAR InstanceIdBuffer[512] = {};

			if (!SetupDiGetDeviceInstanceIdW(
				DeviceInfoSet,
				&DeviceInfo,
				InstanceIdBuffer,
				UE_ARRAY_COUNT(InstanceIdBuffer),
				nullptr))
			{
				continue;
			}

			const FString InstanceId(InstanceIdBuffer);

			// 確認用
			DEBUG_PRINT("Device Instance ID: %s", *InstanceId);

			// 指定した個体だけ通す
			if (!InstanceId.Contains(DeviceSirialNumber, ESearchCase::IgnoreCase))
			{
				continue;
			}

			HKEY DeviceKey = SetupDiOpenDevRegKey(DeviceInfoSet, &DeviceInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
			if (DeviceKey == INVALID_HANDLE_VALUE) { continue; }
			wchar_t PortName[64] = {};
			DWORD PortNameSize = sizeof(PortName);
			const LONG QueryResult = RegQueryValueExW(DeviceKey, L"PortName", nullptr, nullptr,
				reinterpret_cast<LPBYTE>(PortName), &PortNameSize);
			RegCloseKey(DeviceKey);
			if (QueryResult != ERROR_SUCCESS) { continue; }

			const FString PortString(PortName);
			if (PortString.StartsWith(TEXT("COM"), ESearchCase::IgnoreCase))
			{
				const int32 Result = FCString::Atoi(*PortString.Mid(3));
				SetupDiDestroyDeviceInfoList(DeviceInfoSet);
				return Result;
			}
		}
		SetupDiDestroyDeviceInfoList(DeviceInfoSet);
	}
#endif
	return INDEX_NONE;
}

