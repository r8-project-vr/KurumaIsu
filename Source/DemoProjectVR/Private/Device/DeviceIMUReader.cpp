#include "Device/DeviceIMUReader.h"

#include "Engine/Engine.h"
#include "Internationalization/Regex.h"

#if PLATFORM_WINDOWS
#include "WindowsSerial/WindowsSerial.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <setupapi.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogDeviceIMUReader, Log, All);

ADeviceIMUReader::ADeviceIMUReader() { PrimaryActorTick.bCanEverTick = true; }

void ADeviceIMUReader::BeginPlay()
{
	Super::BeginPlay();
	if (bConnectOnBeginPlay) { ConnectDevice(); }
}

void ADeviceIMUReader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bConnectOnBeginPlay && !IsDeviceConnected())
	{
		ReconnectElapsed += DeltaTime;
		if (ReconnectElapsed >= ReconnectInterval)
		{
			ReconnectElapsed = 0.0f;
			ConnectDevice();
		}
	}
	ReadAvailableSerialData();
	UpdateFilteredDebugValues(DeltaTime);
	DrawDebugStatus();
}

void ADeviceIMUReader::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DisconnectDevice();
	Super::EndPlay(EndPlayReason);
}

bool ADeviceIMUReader::ConnectDevice()
{
#if PLATFORM_WINDOWS
	DisconnectDevice();
	if (bAutoDetectComPort)
	{
		const int32 DetectedPort = FindXiaoComPort();
		if (DetectedPort <= 0)
		{
			UE_LOG(LogDeviceIMUReader, Verbose, TEXT("Seeed XIAO USB serial device is not connected."));
			return false;
		}
		ComPort = DetectedPort;
	}
	SerialPort = new WindowsSerial(BaudRate);
	const int32 Result = SerialPort->OpenPort(ComPort);
	if (Result != 0)
	{
		UE_LOG(LogDeviceIMUReader, Error, TEXT("Failed to open COM%d at %d baud (error %d)."), ComPort, BaudRate, Result);
		delete SerialPort;
		SerialPort = nullptr;
		return false;
	}
	if (!SerialPort->SetControlSignals(true, true))
	{
		UE_LOG(LogDeviceIMUReader, Warning, TEXT("COM%d opened, but enabling DTR/RTS failed."), ComPort);
	}
	SerialPort->clear();
	ReceiveBuffer.Reset();
	bHasDeviceOrientation = false;
	bHasDeviceQuaternion = false;
	UE_LOG(LogDeviceIMUReader, Display, TEXT("Connected to IMU device on COM%d at %d baud."), ComPort, BaudRate);
	return true;
#else
	UE_LOG(LogDeviceIMUReader, Error, TEXT("DeviceIMUReader currently supports Windows only."));
	return false;
#endif
}

void ADeviceIMUReader::DisconnectDevice()
{
#if PLATFORM_WINDOWS
	if (SerialPort != nullptr)
	{
		if (SerialPort->GetState()) { SerialPort->ClosePort(); }
		delete SerialPort;
		SerialPort = nullptr;
		UE_LOG(LogDeviceIMUReader, Display, TEXT("IMU device disconnected."));
	}
#endif
	ReceiveBuffer.Reset();
}

int32 ADeviceIMUReader::FindXiaoComPort() const
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
			BYTE HardwareIdBuffer[2048] = {};
			DWORD PropertyType = 0;
			DWORD RequiredSize = 0;
			if (!SetupDiGetDeviceRegistryPropertyW(DeviceInfoSet, &DeviceInfo, SPDRP_HARDWAREID,
				&PropertyType, HardwareIdBuffer, sizeof(HardwareIdBuffer), &RequiredSize))
			{
				continue;
			}

			const FString HardwareId(reinterpret_cast<const wchar_t*>(HardwareIdBuffer));
			if (!HardwareId.Contains(TEXT("VID_2886"), ESearchCase::IgnoreCase)) { continue; }

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

bool ADeviceIMUReader::IsDeviceConnected() const
{
#if PLATFORM_WINDOWS
	return SerialPort != nullptr && SerialPort->GetState();
#else
	return false;
#endif
}

void ADeviceIMUReader::ReadAvailableSerialData()
{
#if PLATFORM_WINDOWS
	if (!IsDeviceConnected()) { return; }
	constexpr int32 MaxBytesPerTick = 4096;
	int32 BytesRead = 0;
	while (SerialPort->available() > 0 && BytesRead < MaxBytesPerTick)
	{
		const int32 Value = SerialPort->read();
		if (Value < 0) { break; }
		++BytesRead;
		const TCHAR Character = static_cast<TCHAR>(static_cast<uint8>(Value));
		if (Character == TEXT('\n'))
		{
			FString CompleteLine = MoveTemp(ReceiveBuffer);
			ReceiveBuffer.Reset();
			CompleteLine.TrimStartAndEndInline();
			if (!CompleteLine.IsEmpty())
			{
				LastReceivedLine = CompleteLine;
				if (bLogReceivedLines) { UE_LOG(LogDeviceIMUReader, Display, TEXT("Serial: %s"), *CompleteLine); }
				ParseIMULine(CompleteLine);
			}
		}
		else if (Character != TEXT('\r'))
		{
			ReceiveBuffer.AppendChar(Character);
			if (ReceiveBuffer.Len() > 1024)
			{
				UE_LOG(LogDeviceIMUReader, Warning, TEXT("Discarding an overlong serial line."));
				ReceiveBuffer.Reset();
			}
		}
	}
#endif
}

bool ADeviceIMUReader::ParseIMULine(const FString& Line)
{
	// Supported examples: "1.0,2.0,3.0" and "gx:1 gy:2 gz:3 ax:4 ay:5 az:6".
	const FRegexPattern NumberPattern(TEXT("[-+]?(?:[0-9]*\\.)?[0-9]+(?:[eE][-+]?[0-9]+)?"));
	FRegexMatcher Matcher(NumberPattern, Line);
	TArray<double> Values;
	while (Matcher.FindNext()) { Values.Add(FCString::Atod(*Matcher.GetCaptureGroup(0))); }
	if (Line.StartsWith(TEXT("QUAT,"), ESearchCase::IgnoreCase) && Values.Num() == 7)
	{
		// Firmware order is W, X, Y, Z, Gyro X, Gyro Y, Gyro Z.
		FQuat ParsedOrientation(Values[1], Values[2], Values[3], Values[0]);
		if (ParsedOrientation.ContainsNaN() || ParsedOrientation.SizeSquared() <= UE_SMALL_NUMBER)
		{
			return false;
		}

		ParsedOrientation.Normalize();
		DeviceOrientationQuaternion = ParsedOrientation;
		DeviceOrientation = ParsedOrientation.Rotator().GetNormalized();
		Gyroscope = FVector(Values[4], Values[5], Values[6]);
		bHasDeviceQuaternion = true;
		bHasDeviceOrientation = true;
		++ParsedSampleCount;
		return true;
	}
	if (Line.StartsWith(TEXT("ORI,"), ESearchCase::IgnoreCase) && Values.Num() == 6)
	{
		// Firmware order is Roll, Pitch, Yaw, Gyro X, Gyro Y, Gyro Z.
		DeviceOrientation = FRotator(Values[1], Values[2], Values[0]);
		Gyroscope = FVector(Values[3], Values[4], Values[5]);
		bHasDeviceOrientation = true;
		++ParsedSampleCount;
		return true;
	}
	if (Values.Num() != 3 && Values.Num() != 6) { return false; }

	Gyroscope = FVector(Values[0], Values[1], Values[2]);
	if (Values.Num() == 6) { Acceleration = FVector(Values[3], Values[4], Values[5]); }
	++ParsedSampleCount;
	return true;
}

void ADeviceIMUReader::UpdateFilteredDebugValues(float DeltaTime)
{
	auto ApplyAxisDeadZone = [](const FVector& Value, float DeadZone)
	{
		return FVector(
			FMath::Abs(Value.X) >= DeadZone ? Value.X : 0.0,
			FMath::Abs(Value.Y) >= DeadZone ? Value.Y : 0.0,
			FMath::Abs(Value.Z) >= DeadZone ? Value.Z : 0.0);
	};

	const FVector TargetGyroscope = ApplyAxisDeadZone(Gyroscope, GyroscopeDeadZone);
	const FVector TargetAcceleration = ApplyAxisDeadZone(Acceleration, AccelerationDeadZone);
	FilteredGyroscope = FMath::VInterpTo(FilteredGyroscope, TargetGyroscope, DeltaTime, DebugSmoothingSpeed);
	FilteredAcceleration = FMath::VInterpTo(FilteredAcceleration, TargetAcceleration, DeltaTime, DebugSmoothingSpeed);
}

void ADeviceIMUReader::DrawDebugStatus() const
{
	if (!bShowOnScreenDebug || GEngine == nullptr) { return; }
	const FString Status = IsDeviceConnected()
		? FString::Printf(
			TEXT("Device IMU [COM%d]  Samples: %d\nOrientation: Pitch %.1f  Yaw %.1f  Roll %.1f\nRaw: %s"),
			ComPort, ParsedSampleCount, DeviceOrientation.Pitch, DeviceOrientation.Yaw, DeviceOrientation.Roll,
			LastReceivedLine.IsEmpty() ? TEXT("(waiting for data)") : *LastReceivedLine)
		: FString::Printf(TEXT("Device IMU: COM%d disconnected"), ComPort);
	GEngine->AddOnScreenDebugMessage(static_cast<uint64>(reinterpret_cast<UPTRINT>(this)), 0.0f,
		IsDeviceConnected() ? FColor::Cyan : FColor::Red, Status);
}
