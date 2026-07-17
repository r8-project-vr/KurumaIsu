#pragma once

#if !UE_BUILD_SHIPPING

// 文字列を画面とログに表示するマクロ（文字列はprintfフォーマットで指定）
#define DEBUG_PRINT(Format, ...) \
{ \
    FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
    if (GEngine) \
    { \
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Orange, Msg); \
    } \
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg); \
}

// 任意の列挙型のデータ名を文字列で返すマクロ
#define ENUM_STR(Value) \
    *StaticEnum<std::decay_t<decltype(Value)>>()->GetNameStringByValue((int64)Value)

#else

#define DEBUG_PRINT(Format, ...)

#endif