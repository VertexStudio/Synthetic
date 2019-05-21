#include "DynamicsCommon.h"

float UDynamicsCommon::MeanOfFloatArray(const TArray<float> &Samples)
{
    float T = 0.0f;
    for (auto I = 0; I < Samples.Num(); ++I)
    {
        T += Samples[I];
    }
    return T / Samples.Num();
}

float UDynamicsCommon::VarianceOfFloatArray(const TArray<float> &Samples)
{
    float M = MeanOfFloatArray(Samples);
    float R = 0.0f;
    for (auto I = 0; I < Samples.Num(); ++I)
    {
        R += FMath::Pow(Samples[I] - M, 2.0f);
    }
    return R / Samples.Num();
}

float UDynamicsCommon::MedianOfFloatArray(const TArray<float> &Samples)
{
    TArray<float> S;
    S.Append(Samples);
    S.Sort();

    int Length = S.Num();
    if (Length == 0)
    {
        return 0;
    }
    else if (Length == 1)
    {
        return S[0];
    }
    else if (Length % 2)
    {
        int Index = Length / 2;
        return S[Index];
    }
    else
    {
        int IndexA = Length / 2 - 1;
        int IndexB = Length / 2;
        return (S[IndexA] + S[IndexB]) / 2.0f;
    }
}

float UDynamicsCommon::StandardDeviationOfFloatArray(const TArray<float> &Samples)
{
    float V = VarianceOfFloatArray(Samples);
    return FMath::Sqrt(V);
}

FQuat UDynamicsCommon::RotatorToQuat(FRotator Rotator)
{
    return Rotator.Quaternion();
}

FVector UDynamicsCommon::CubicBezier(float Time, FVector P0, FVector P1, FVector P2, FVector P3)
{
    // p = (1-t)^3*p0 + 3*t*(1-t)^2*p1 + 3*t^2*(1-t)*p2 + t^3*p3
    return P0 * FMath::Pow(1.f - Time, 3.f) + P1 * 3.f * Time * FMath::Pow(1.f - Time, 2.f) +
           P2 * 3.f * FMath::Pow(Time, 2.f) * (1.f - Time) + P3 * FMath::Pow(Time, 3.f);
}

bool UDynamicsCommon::CalcMinimumBoundingBox(USceneCaptureComponent2D *RenderComponent, const FQuat& Rotation, FVector Origin, FVector Extent, FBox2D &BoxOut, bool &Truncated)
{
    bool isCompletelyInView = true;
    UTextureRenderTarget2D *RenderTexture = RenderComponent->TextureTarget;
    TArray<FVector> Points; 
    TArray<FVector2D> Points2D;
    FTransform const Transform(Rotation);
    FMinimalViewInfo Info;

    Info.Location = RenderComponent->GetComponentTransform().GetLocation();
    Info.Rotation = RenderComponent->GetComponentTransform().GetRotation().Rotator();
    Info.FOV = RenderComponent->FOVAngle;
    Info.ProjectionMode = RenderComponent->ProjectionType;
    Info.AspectRatio = float(RenderTexture->SizeX) / float(RenderTexture->SizeY);
    Info.OrthoNearClipPlane = 1;
    Info.OrthoFarClipPlane = 1000;
    Info.bConstrainAspectRatio = true;

    Points.Add(Origin + Transform.TransformPosition(FVector(Extent.X, Extent.Y, Extent.Z)));
    Points.Add(Origin + Transform.TransformPosition(FVector(-Extent.X, Extent.Y, Extent.Z)));
    Points.Add(Origin + Transform.TransformPosition(FVector(Extent.X, -Extent.Y, Extent.Z)));
    Points.Add(Origin + Transform.TransformPosition(FVector(-Extent.X, -Extent.Y, Extent.Z)));
    Points.Add(Origin + Transform.TransformPosition(FVector(Extent.X, Extent.Y, -Extent.Z)));
    Points.Add(Origin + Transform.TransformPosition(FVector(-Extent.X, Extent.Y, -Extent.Z)));
    Points.Add(Origin + Transform.TransformPosition(FVector(Extent.X, -Extent.Y, -Extent.Z)));
    Points.Add(Origin + Transform.TransformPosition(FVector(-Extent.X, -Extent.Y, -Extent.Z)));

    FVector2D MinPixel(RenderTexture->SizeX, RenderTexture->SizeY);
    FVector2D MaxPixel(0, 0);
    FIntRect ScreenRect(0, 0, RenderTexture->SizeX, RenderTexture->SizeY);

    FSceneViewProjectionData ProjectionData;
    ProjectionData.ViewOrigin = Info.Location;

    ProjectionData.ViewRotationMatrix = FInverseRotationMatrix(Info.Rotation) * 
                                                                                FMatrix(
                                                                                        FPlane(0, 0, 1, 0),
                                                                                        FPlane(1, 0, 0, 0),
                                                                                        FPlane(0, 1, 0, 0),
                                                                                        FPlane(0, 0, 0, 1));

    ProjectionData.ProjectionMatrix = Info.CalculateProjectionMatrix();
    ProjectionData.SetConstrainedViewRectangle(ScreenRect);

    for (FVector &Point : Points)
    {
        FVector2D Pixel(0, 0);
        FSceneView::ProjectWorldToScreen((Point), ScreenRect, ProjectionData.ComputeViewProjectionMatrix(), Pixel);
        Points2D.Add(Pixel);
        MaxPixel.X = FMath::Max(Pixel.X, MaxPixel.X);
        MaxPixel.Y = FMath::Max(Pixel.Y, MaxPixel.Y);
        MinPixel.X = FMath::Min(Pixel.X, MinPixel.X);
        MinPixel.Y = FMath::Min(Pixel.Y, MinPixel.Y);
    }

    BoxOut = FBox2D(MinPixel, MaxPixel);
    if (BoxOut.Min.X < 0)
    {
        BoxOut.Min.X = 0;
        isCompletelyInView = false;
    }
    else if (BoxOut.Min.X > RenderTexture->SizeX)
    {
        BoxOut.Min.X = RenderTexture->SizeX;
        isCompletelyInView = false;
    }
    if (BoxOut.Min.Y < 0)
    {
        BoxOut.Min.Y = 0;
        isCompletelyInView = false;
    }
    else if (BoxOut.Min.Y > RenderTexture->SizeY)
    {
        BoxOut.Min.Y = RenderTexture->SizeY;
        isCompletelyInView = false;
    }
    if (BoxOut.Max.X > RenderTexture->SizeX)
    {
        BoxOut.Max.X = RenderTexture->SizeX;
        isCompletelyInView = false;
    }
    else if (BoxOut.Max.X < 0)
    {
        BoxOut.Max.X = 0;
        isCompletelyInView = false;
    }
    if (BoxOut.Max.Y > RenderTexture->SizeY)
    {
        BoxOut.Max.Y = RenderTexture->SizeY;
        isCompletelyInView = false;
    }
    else if (BoxOut.Max.Y < 0)
    {
        BoxOut.Max.Y = 0;
        isCompletelyInView = false;
    }

    Truncated = !isCompletelyInView;

    if (BoxOut.GetSize().X < 5 || BoxOut.GetSize().Y < 5)
    {
        return false;
    }

    return true;
}

bool UDynamicsCommon::ReadTxt(FString FilePath, FString FileName, FString &OutputTxt)
{
    FPaths::NormalizeDirectoryName(FilePath);
    FString full_path =  FilePath + "/" + FileName;
    FPaths::RemoveDuplicateSlashes(full_path);
    return FFileHelper::LoadFileToString(OutputTxt, *full_path);
}

bool UDynamicsCommon::WriteTxt(FString inText, FString FilePath, FString FileName)
{
    FPaths::NormalizeDirectoryName(FilePath);
    FString full_path =  FilePath + "/" + FileName;
    FPaths::RemoveDuplicateSlashes(full_path);
    return FFileHelper::SaveStringToFile(inText, *full_path);
}