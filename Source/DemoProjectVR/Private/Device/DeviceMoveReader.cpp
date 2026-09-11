// Fill out your copyright notice in the Description page of Project Settings.


#include "Device/DeviceMoveReader.h"
#include "Device/MoveInput.h"

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

	if (!bDeviceConnected)
	{
		return;
	}

	PollingTimer += DeltaTime;

	// まだ応答待ちなら読み取り処理
	if (bWaitingForResponse)
	{
		ReadDataProcess();
		return;
	}

	// ポーリング周期前なら何もしない
	if (PollingTimer < PollingInterval)
	{
		return;
	}

	PollingTimer = 0.0f;

	// まず更新フラグを要求
	RequestUpdateFlag();
}

bool ADeviceMoveReader::ConnectDevice()
{
#if PLATFORM_WINDOWS

	// 既存接続がある場合だけ切断
	if (SerialController != nullptr)
	{
		DisconnectDevice();
	}

	// COMポート自動検索
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

	// コントローラー生成
	SerialController = NewObject<UASerialLibControllerWin>(this);

	if (SerialController == nullptr)
	{
		DEBUG_PRINT("Failed to create SerialController");
		return false;
	}

	DEBUG_PRINT("SerialController created.");

	SerialController->Initialize(TargetDeviceID, 0x01, 0x01);

	DEBUG_PRINT("SerialController initialized.");

	WindowsSerial* serial = new WindowsSerial();

	if (serial == nullptr)
	{
		DEBUG_PRINT("Failed to create WindowsSerial");
		return false;
	}

	// コントローラーへ設定
	SerialController->SetInterfacePt(serial);

	// COMポートに接続
	int result = SerialController->ConnectDevice(ComPort);

	if (result == -1)
	{
		DEBUG_PRINT("Failed to connect Move device on COM%d", ComPort);
		return false;
	}

	DEBUG_PRINT("Connected to Move device on COM%d", ComPort);

	bDeviceConnected = true;

	return true;
#else
	DEBUG_PRINT("DeviceMoveReader currently supports Windows only.");
	return false;
#endif
}

void ADeviceMoveReader::DisconnectDevice()
{
#if PLATFORM_WINDOWS
	// Controller経由で切断
	if (IsValid(SerialController))
	{
		SerialController->DisConnectDevice();
	}

	SerialController = nullptr;

	// WindowsSerialを削除
	if (SerialInterface != nullptr)
	{
		delete SerialInterface;
		SerialInterface = nullptr;
	}

	DEBUG_PRINT("Move device disconnected.");
#endif
	ReceiveBuffer.Reset();
}

bool ADeviceMoveReader::IsDeviceConnected() const
{
#if PLATFORM_WINDOWS
	if (SerialController == nullptr)
	{
		DEBUG_PRINT("SerialController Nullptr");
		return false;
	}

	return SerialController->GetConnectionState();
#else
	return false;
#endif
}

void ADeviceMoveReader::SetMoveInput(AMoveInput* moveinput)
{
	MoveInput = moveinput;
}

void ADeviceMoveReader::SendDeviceValue()
{
	MoveInput->SetValue(CurrentRPS);
}

void ADeviceMoveReader::ReadDataProcess()
{
	if (SerialController == nullptr)
	{
		return;
	}

	ASerialDataStruct::ASerialData resp;

	const int result = SerialController->ReadData(&resp);

	if (result != 0)
	{
		return;
	}

	//DEBUG_PRINT("Response received. data_num = %d", resp.data_num);

	bWaitingForResponse = false;

	// 更新フラグの応答
	if (CurrentRequest == EDeviceRequest::UpdateFlag)
	{
		if (resp.data_num < 1)
		{
			DEBUG_PRINT("UpdateFlag response is invalid");
			CurrentRequest = EDeviceRequest::None;
			return;
		}

		uint8 UpdateFlag = resp.data[0];

		//DEBUG_PRINT("Update Flag = %d", UpdateFlag);
		
		CurrentRequest = EDeviceRequest::None;

		if (UpdateFlag == 1)
		{
			RequestRPS();
		}

		return;
	}

	// RPSの応答
	if (CurrentRequest == EDeviceRequest::RPS)
	{
		if (resp.data_num < 4)
		{
			DEBUG_PRINT("RPS response is invalid");
			return;
		}

		// ビッグエンディアンでint32に変換
		int32 rawRPS = (static_cast<int32>(resp.data[0]) << 24) | (static_cast<int32>(resp.data[1]) << 16) | (static_cast<int32>(resp.data[2]) << 8) | static_cast<int32>(resp.data[3]);

		// 100倍されているので戻す
		float RPS = static_cast<float>(rawRPS) / 100.0f;

		// 正負の取得
		SerialController->WriteData(0x23);
		const int resultTemp = SerialController->ReadData(&resp);
		
		if (resultTemp != 0)
		{
			DEBUG_PRINT("Fail to Read 0x23");
			return;
		}

		bool isPositive = resp.data[0] == 0;
		if (isPositive)
		{
			rawRPS *= -1;
			RPS *= -1.0f;
		}
		if (bInputInversion)
		{
			rawRPS *= -1;
			RPS *= -1.0f;
		}


		//DEBUG_PRINT("Raw RPS = %d", rawRPS);
		//DEBUG_PRINT("RPS = %lf", RPS);

		CurrentRPS = RPS;

		SendDeviceValue();

		CurrentRequest = EDeviceRequest::None;

		return;
	}
}

void ADeviceMoveReader::RequestUpdateFlag()
{
	if (SerialController == nullptr)
	{
		return;
	}

	SerialController->WriteData(0x20);

	bWaitingForResponse = true;

	CurrentRequest = EDeviceRequest::UpdateFlag;

	//DEBUG_PRINT("Request Update Flag");
}

void ADeviceMoveReader::RequestRPS()
{
	if (SerialController == nullptr)
	{
		return;
	}

	SerialController->WriteData(0x22);

	CurrentRequest = EDeviceRequest::RPS;

	bWaitingForResponse = true;

	//DEBUG_PRINT("Request RPS");
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

