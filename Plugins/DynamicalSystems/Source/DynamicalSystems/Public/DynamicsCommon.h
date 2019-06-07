#pragma once

#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "SceneView.h"
#include "Misc/FileHelper.h"
#include "Components/SkinnedMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "SkeletalRenderPublic.h"
#include "DynamicsCommon.generated.h"

UENUM(BlueprintType)
enum class EExportFormat: uint8
{
  VE_YOLO         UMETA(DisplayName="YOLOv3"),
  VE_PASCAL_VOC   UMETA(DisplayName="Pascal VOC")
};

UCLASS()
class DYNAMICALSYSTEMS_API UDynamicsCommon : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    static bool ProjectWorldToScreen(const FVector& WorldPosition, const FIntRect& ViewRect, const FMatrix& ViewProjectionMatrix, FVector& out_ScreenPos);

  public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Enum)
	  EExportFormat FormatEnum;

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

    UFUNCTION(BlueprintCallable, Category = "Math|Vector")
    static bool SaveLabelingFormat(USceneCaptureComponent2D *RenderComponent, EExportFormat Format, FString FilePath = "/tmp/synthetic_data", FString FileName = "render");

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math|Vector")
    static bool CalcMinimumBoundingBox(const AActor* Actor, USceneCaptureComponent2D *RenderComponent, FBox2D &BoxOut, float &DistanceFromCameraView, bool &Truncated, bool &Valid, bool &IsOccluded);

    UFUNCTION(BlueprintPure, Category = "File", meta = (Keywords = "ReadTxt"))
    static bool ReadTxt(FString FilePath, FString FileName, FString &OutputTxt);

    UFUNCTION(BlueprintCallable, Category = "File", meta = (Keywords = "WriteTxt"))
    static bool WriteTxt(FString inText, FString FilePath, FString FileName);
};
