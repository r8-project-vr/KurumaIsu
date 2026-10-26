// Fill out your copyright notice in the Description page of Project Settings.


#include "Device/DeviceMoveReader.h"
#include "Device/MoveInput.h"

int CalcStateIndex(bool bDeviceA, bool bDeviceB)
{
	// 受け取った2進数をデバイスの情報に変換
	// 
	// 読み取り順序
	//  A | B | V
	// ---|---|---
	//  0 | 0 | 0
	//  1 | 0 | 1
	//  1 | 1 | 3
	//  0 | 1 | 2
	if (bDeviceA)
	{
		if (bDeviceB)
		{
			return 3;
		}
		return 1;
	}
	if (bDeviceB)
	{
		return 2;
	}
	return 0;
}


// Sets default values
ADeviceMoveReader::ADeviceMoveReader()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ADeviceMoveReader::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	DisconnectDevice();
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

	// 接続が無かったら再接続処理
	if (!bDeviceConnected)
	{
		bool canReConnect = ReConnectTimer <= 0.0f;
		if (canReConnect)
		{
			ConnectDevice();
			ReConnectTimer = ReConnectInterval;
		}
		else
		{
			ReConnectTimer -= DeltaTime;
		}
	}

	float fCorrectedAngularDelta = -fDeviceCurrentAngle * (1.0f - exp(-DeltaTime * fDeviceCorrectionRate));
	fDeviceCurrentAngle += fCorrectedAngularDelta;
	if (bInputInversion)
	{
		fCorrectedAngularDelta *= -1;
	}
		MoveInput->SetValue(fCorrectedAngularDelta * 300);

	PollingTimer += DeltaTime;

	//DEBUG_PRINT("=====================================");
	//DEBUG_PRINT("PollingTimer: % lf", PollingTimer);

	// まだ応答待ちなら読み取り処理
	if (bWaitingForResponse)
	{
		const float PollingTimeout = 5.0f;
		if (PollingTimer >= PollingTimeout)
		{
			//DEBUG_PRINT("Polling Timeout");

			DisconnectDevice();
			PollingTimer = 0.0f;
			return;
		}

		//DEBUG_PRINT("Try Read Data");
		ReadDataProcess();
		return;
	}

	// デバイス側の更新レートに合わせて変更する（現在データは送られてきた全てのデータに応答する）
	/*
	// ポーリング周期前なら何もしない
	if (PollingTimer < PollingInterval)
	{
		//DEBUG_PRINT("Waiting Polling");
		return;
	}
	*/

	PollingTimer = 0.0f;

	//DEBUG_PRINT("Request Updated");

	//// まず更新フラグを要求
	RequestUpdate();
	//RequestRPS();
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

	DEBUG_PRINT("%s | SerialController created.", *GetName());

	SerialController->Initialize(TargetDeviceID, 0x01, 0x01);

	DEBUG_PRINT("%s | SerialController initialized.", *GetName());

	WindowsSerial* serial = new WindowsSerial();

	if (serial == nullptr)
	{
		DEBUG_PRINT("Failed to create WindowsSerial");
		return false;
	}

	SerialInterface = serial;

	// コントローラーへ設定
	SerialController->SetInterfacePt(serial);

	// COMポートに接続
	int result = SerialController->ConnectDevice(ComPort);

	if (result == -1)
	{
		DEBUG_PRINT("Failed to connect Move device on COM%d", ComPort);
		return false;
	}

	DEBUG_PRINT("%s | Connected to Move device on COM%d", *GetName(), ComPort);

	bDeviceConnected = true;
	PollingTimer = 0.0f;
	ReConnectTimer = 0.0f;
	bWaitingForResponse = false;
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
	bDeviceConnected = false;

	DEBUG_PRINT("%s | Move device disconnected.", *GetName());
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

void ADeviceMoveReader::ChangeDeviceNum(EDeviceNumber changedNum)
{
	DisconnectDevice();

	DeviceNumber = changedNum;
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
	DEBUG_PRINT("%s | %d", *GetName(), resp.data[0]);

	bWaitingForResponse = false;


	// 受け取った2進数をデバイスの情報に変換
	// 
	// 読み取り順序
	//  A | B | V
	// ---|---|---
	//  0 | 0 | 0
	//  1 | 0 | 1
	//  1 | 1 | 3
	//  0 | 1 | 2

	bool bDeviceNewA = false;
	bool bDeviceNewB = false;
	switch (resp.data[0])
	{
	case 0:
		bDeviceNewA = false;
		bDeviceNewB = false;
		break;
	case 1:
		bDeviceNewA = true;
		bDeviceNewB = false;
		break;
	case 2:
		bDeviceNewA = false;
		bDeviceNewB = true;
		break;
	case 3:
		bDeviceNewA = true;
		bDeviceNewB = true;
		break;
	}
	
	const float fAngleToTransitionTable[4] =
	{
		fMarkRatio / 2.0f, 
		(1.0f - fMarkRatio) / 2.0f,
		(1.0f - fMarkRatio) / 2.0f, 
		fMarkRatio / 2.0f,
	};
	const int XX = iRecentStateRotation * 2;
	int iStateRotationTable[4][4] =
	{
		// New | Old
		/*        00, 01, 10, 11,  */
		/* 00 */{ +0, +1, -1, XX, }, 
		/* 01 */{ -1, +0, XX, +1, }, 
		/* 10 */{ +1, XX, +0, -1, }, 
		/* 11 */{ XX, -1, +1, +0, }, 
	};
	int iOldState = CalcStateIndex(bDeviceA, bDeviceB);
	int iNewState = CalcStateIndex(bDeviceNewA, bDeviceNewB);
	int iStateRotation = iStateRotationTable[iNewState][iOldState];
	if (iStateRotation > 0)
	{
		for (int i = 0; i < iStateRotation; i++)
		{
			fDeviceCurrentAngle += fAngleToTransitionTable[(i + iOldState + 4) % 4];
		}
	}
	else
	{
		for (int i = -1; i >= iStateRotation; i--)
		{
			fDeviceCurrentAngle -= fAngleToTransitionTable[(i + iOldState + 4) % 4];
		}
	}
	if (iStateRotation != XX && iStateRotation != 0)
	{
		iRecentStateRotation = iStateRotation;
	}
	// DEBUG_PRINT("%s | Rotation: %d, Recent: %d", *GetName(), iStateRotation, iRecentStateRotation)
	// DEBUG_PRINT("%s | A: %d, B: %d, NewA: %d, NewB: %d", *GetName(), bDeviceA ? 1 : 0, bDeviceB ? 1 : 0, bDeviceNewA ? 1 : 0, bDeviceNewB ? 1 : 0);
	// DEBUG_PRINT("%s | State: %d, Current: %lf", *GetName(), iNewState, fDeviceCurrentAngle);
	bDeviceA = bDeviceNewA;
	bDeviceB = bDeviceNewB;

	if (resp.data_num != 1)
	{
		DEBUG_PRINT("RPS response is invalid -> data num : %d", resp.data_num);
		return;
	}

	SendDeviceValue();

	CurrentRequest = EDeviceRequest::None;

	return;

	// デバイスの仕様変更に伴い変更
	// 更新フラグの応答
	/*
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
	*/

	// デバイスの仕様変更に伴い変更
	/*

	// RPSの応答
	if (CurrentRequest == EDeviceRequest::RPS)
	{


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
			DisconnectDevice();
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
	}
	*/
}

// デバイスの仕様変更に伴い変更
bool ADeviceMoveReader::RequestUpdate()
{
	if (SerialController == nullptr)
	{
		// DEBUG_PRINT("RequestUpdate failed : SerialController is nullptr");
		return false;
	}

	SerialController->WriteData(0x20);

	ASerialDataStruct::ASerialData resp;

	const int result = SerialController->ReadData(&resp);

	DEBUG_PRINT("%s | Request Update Result => %d", *GetName(), result);

	bWaitingForResponse = true;

	if (result != 0)
	{
		return false;
	}

	return true;
}

// デバイスの仕様変更に伴い利用停止
void ADeviceMoveReader::RequestRPS()
{
	// 利用停止

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
			if (!InstanceId.Contains(SerialNumberTable[(int)DeviceNumber], ESearchCase::IgnoreCase))
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

