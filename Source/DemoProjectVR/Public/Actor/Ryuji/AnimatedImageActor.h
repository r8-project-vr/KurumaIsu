#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AnimatedImageActor.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMeshComponent;
class UTexture2D;

/**
 * Displays an editable array of textures one after another.
 * Uses a timer instead of Tick so inactive frames have no per-frame CPU cost.
 */
UCLASS(Blueprintable)
class DEMOPROJECTVR_API AAnimatedImageActor : public AActor
{
	GENERATED_BODY()

public:
	AAnimatedImageActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Textures are displayed in array order. Null entries are skipped. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animated Image|Frames")
	TArray<TObjectPtr<UTexture2D>> Images;

	/** Seconds between frames. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animated Image|Playback", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float FrameInterval = 1.0f;

	/** Return to the first valid image after reaching the end. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animated Image|Playback")
	bool bLoop = true;

	/** Start playback automatically when play begins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animated Image|Playback")
	bool bAutoPlay = true;

	/** Initial array index. Values outside the array are clamped. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animated Image|Playback", meta = (ClampMin = "0"))
	int32 StartIndex = 0;

	/** Material containing a Texture2D parameter named FrameTexture by default. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animated Image|Rendering")
	TObjectPtr<UMaterialInterface> DisplayMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animated Image|Rendering")
	FName TextureParameterName = TEXT("FrameTexture");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animated Image|Rendering")
	TObjectPtr<UStaticMeshComponent> ImagePlane;

	UFUNCTION(BlueprintCallable, Category = "Animated Image")
	void PlayAnimation();

	UFUNCTION(BlueprintCallable, Category = "Animated Image")
	void StopAnimation();

	UFUNCTION(BlueprintCallable, Category = "Animated Image")
	void RestartAnimation();

	UFUNCTION(BlueprintCallable, Category = "Animated Image")
	void SetFrame(int32 FrameIndex);

	UFUNCTION(BlueprintPure, Category = "Animated Image")
	int32 GetCurrentFrame() const { return CurrentFrameIndex; }

	UFUNCTION(BlueprintPure, Category = "Animated Image")
	bool IsAnimationPlaying() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	FTimerHandle FrameTimerHandle;
	int32 CurrentFrameIndex = INDEX_NONE;

	void AdvanceFrame();
	void EnsureDynamicMaterial();
	bool ApplyFrame(int32 FrameIndex);
	int32 FindFirstValidFrame(int32 PreferredIndex) const;
};
