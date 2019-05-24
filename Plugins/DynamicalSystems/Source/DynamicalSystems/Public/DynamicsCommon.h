#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "SceneView.h"
#include "Misc/FileHelper.h"
#include "Components/SkinnedMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "SkeletalRenderPublic.h"
#include "DrawDebugHelpers.h"
#include "DynamicsCommon.generated.h"

/**
 * 
 */
UCLASS()
class DYNAMICALSYSTEMS_API UDynamicsCommon : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

  public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math|Float")
    static float MeanOfFloatArray(const TArray<float> &Samples);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math|Float")
    static float VarianceOfFloatArray(const TArray<float> &Samples);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math|Float")
    static float MedianOfFloatArray(const TArray<float> &Samples);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math|Float")
    static float StandardDeviationOfFloatArray(const TArray<float> &Samples);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math|Rotator")
    static FQuat RotatorToQuat(FRotator Rotator);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math|Vector")
    static FVector CubicBezier(float Time, FVector P0, FVector P1, FVector P2, FVector P3);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math|Vector")
    static bool CalcMinimumBoundingBox(const AActor* Actor, USceneCaptureComponent2D *RenderComponent, FBox2D &BoxOut, bool &Truncated);

    UFUNCTION(BlueprintPure, Category = "File", meta = (Keywords = "ReadTxt"))
    static bool ReadTxt(FString FilePath, FString FileName, FString &OutputTxt);

    UFUNCTION(BlueprintCallable, Category = "File", meta = (Keywords = "WriteTxt"))
    static bool WriteTxt(FString inText, FString FilePath, FString FileName);
};
