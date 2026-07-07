#pragma once

#if !UE_BUILD_SHIPPING

#define DEBUG_PRINT(Format, ...) \
{ \
    FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
    if (GEngine) \
    { \
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Orange, Msg); \
    } \
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg); \
}

#else

#define DEBUG_PRINT(Format, ...)

#endif