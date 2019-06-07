#include "DynamicsCommon.h"
#include "DetectableActor.h"
#include "XmlParser/Public/XmlFile.h"

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

bool UDynamicsCommon::SaveLabelingFormat(USceneCaptureComponent2D *RenderComponent, EExportFormat Format, FString FilePath, FString FileName)
{
    uint32 imgWidth = RenderComponent->TextureTarget->SizeX;
    uint32 imgHeight = RenderComponent->TextureTarget->SizeY;
    TArray<TTuple<FBox2D, float>> Objects;
    FString yoloOutput;

    const FString XmlPath = FPaths::Combine(FilePath, FileName + ".xml");
    const FString ImagePath = FPaths::Combine(FilePath, FileName + ".png");

    FXmlFile *Xml = new FXmlFile("<annotation></annotation>", EConstructMethod::ConstructFromBuffer);
    FXmlNode *XmlRoot = Xml->GetRootNode();

    XmlRoot->AppendChildNode(TEXT("folder"), TEXT("Unspecified"));
    XmlRoot->AppendChildNode(TEXT("filename"), FileName);
    XmlRoot->AppendChildNode(TEXT("path"), FilePath);
    XmlRoot->AppendChildNode(TEXT("source"), TEXT(""));
    FXmlNode *XmlSource = XmlRoot->FindChildNode(TEXT("source"));
    XmlSource->AppendChildNode(TEXT("database"), TEXT("Unknown"));
    XmlRoot->AppendChildNode(TEXT("size"), TEXT(""));
    FXmlNode *XmlSize = XmlRoot->FindChildNode(TEXT("size"));
    XmlSize->AppendChildNode(TEXT("width"), FString::FromInt(imgWidth));
    XmlSize->AppendChildNode(TEXT("height"), FString::FromInt(imgHeight));
    XmlSize->AppendChildNode(TEXT("depth"), TEXT("3"));
    XmlRoot->AppendChildNode(TEXT("segmented"), TEXT("0"));

    FXmlNode *XmlNext = XmlRoot->FindChildNode(TEXT("segmented"));

    for (TActorIterator<AActor> ActorItr(RenderComponent->GetWorld()); ActorItr; ++ActorItr)
    {
       UDetectableActor *Actor = ActorItr->FindComponentByClass<UDetectableActor>();

       if (Actor)
       {   
           FBox2D BoxOut;
           bool IsOccluded;
           bool IsTruncated;
           bool IsValid;
           bool IsInCameraView = CalcMinimumBoundingBox(*ActorItr, RenderComponent, BoxOut, IsTruncated, IsValid, IsOccluded);
           
           Actor->Occluded = static_cast<int>(IsOccluded);
           
           if (IsValid && IsInCameraView)
           {
               Objects.Add(MakeTuple(BoxOut, 5.0f));

               float cx = ((BoxOut.Min.X + BoxOut.Max.X) / 2) / imgWidth;
               float cy = ((BoxOut.Min.Y + BoxOut.Max.Y) / 2) / imgHeight;
               float w = (BoxOut.Max.X - BoxOut.Min.X) / imgWidth;
               float h = (BoxOut.Max.Y - BoxOut.Min.Y) / imgHeight;
               
               if (Format == EExportFormat::VE_YOLO)
               {
                   // YOLOv3 Format: {CLASS} {CX} {CY} {W} {H}, normalized [0,1]
                   yoloOutput += Actor->ClassName + " " +  FString::SanitizeFloat(cx) + " " + FString::SanitizeFloat(cy) + " "
                                + FString::SanitizeFloat(w) + " " + FString::SanitizeFloat(h) + "\n";
               } else if(Format == EExportFormat::VE_PASCAL_VOC) {
                   XmlRoot->AppendChildNode(TEXT("object"), "");
                   XmlNext = (FXmlNode *)XmlNext->GetNextNode();
                   XmlNext->AppendChildNode(TEXT("name"), ActorItr->GetName());
                   XmlNext->AppendChildNode(TEXT("pose"), TEXT("Unknown"));
                   XmlNext->AppendChildNode(TEXT("truncated"), IsTruncated ? TEXT("1") : TEXT("0"));
                   XmlNext->AppendChildNode(TEXT("difficult"), TEXT("0"));
                   XmlNext->AppendChildNode(TEXT("occluded"), FString::FromInt(Actor->Occluded));
                   XmlNext->AppendChildNode(TEXT("bndbox"), TEXT(""));
                   FXmlNode *XmlBox = XmlNext->FindChildNode(TEXT("bndbox"));
                   XmlBox->AppendChildNode(TEXT("xmin"), FString::Printf(TEXT("%f"), BoxOut.Min.X));
                   XmlBox->AppendChildNode(TEXT("xmax"), FString::Printf(TEXT("%f"), BoxOut.Max.X));
                   XmlBox->AppendChildNode(TEXT("ymin"), FString::Printf(TEXT("%f"), BoxOut.Min.Y));
                   XmlBox->AppendChildNode(TEXT("ymax"), FString::Printf(TEXT("%f"), BoxOut.Max.Y));
               }
           }
       }
    }

    RenderComponent->CaptureScene();
    UKismetRenderingLibrary::ExportRenderTarget(RenderComponent->GetWorld(), RenderComponent->TextureTarget, FilePath, FileName + ".png");
    
    if (Format == EExportFormat::VE_YOLO)
    {
        return WriteTxt(yoloOutput, FilePath, FileName + ".txt");
    } else
    {
        if (!Xml->Save(XmlPath))
        {
            UE_LOG(LogTemp, Error, TEXT("%s couldn't be saved at %s"), *FileName, *FilePath );
            return false;
        }
        delete Xml;
        return true;
    }

    UE_LOG(LogTemp, Error, TEXT("%s couldn't be saved at %s"), *FileName, *FilePath );
    return false;
}

bool UDynamicsCommon::CalcMinimumBoundingBox(const AActor* Actor, USceneCaptureComponent2D *RenderComponent, FBox2D &BoxOut, bool &Truncated, bool &Valid, bool &Occluded)
{
    bool isCompletelyInView = true;
    Valid = true;
    UTextureRenderTarget2D *RenderTexture = RenderComponent->TextureTarget;
    TArray<FVector> Points;
    TArray<FVector2D> Points2D;
    FMinimalViewInfo Info;

    Info.Location = RenderComponent->GetComponentTransform().GetLocation();
    Info.Rotation = RenderComponent->GetComponentTransform().GetRotation().Rotator();
    Info.FOV = RenderComponent->FOVAngle;
    Info.ProjectionMode = RenderComponent->ProjectionType;
    Info.AspectRatio = float(RenderTexture->SizeX) / float(RenderTexture->SizeY);
    Info.OrthoNearClipPlane = 1;
    Info.OrthoFarClipPlane = 1000;
    Info.bConstrainAspectRatio = true;
    
    USkinnedMeshComponent *Mesh = Actor->FindComponentByClass<USkinnedMeshComponent>();

    // Skinned Mesh
    if (Mesh)
    {
        Occluded = !(Mesh->GetWorld()->GetTimeSeconds() - Mesh->LastRenderTimeOnScreen <= 0.2f);
        
        TArray<FFinalSkinVertex> OutVertices;
        Mesh->GetCPUSkinnedVertices(OutVertices, 0);

        FTransform const MeshWorldTransform = Mesh->GetComponentTransform();

        for (FFinalSkinVertex &Vertex : OutVertices)
        {
            Points.Add(MeshWorldTransform.TransformPosition(Vertex.Position));
        }
    } else {
        Valid = false;

        UStaticMeshComponent *StaticMeshComponent = Actor->FindComponentByClass<UStaticMeshComponent>();

        // Static Mesh
        if (StaticMeshComponent)
        {
            if (!StaticMeshComponent) { Valid = false; };
            if (!StaticMeshComponent->GetStaticMesh()) { Valid = false; };
            if (!StaticMeshComponent->GetStaticMesh()->RenderData) { Valid = false; };
            if (StaticMeshComponent->GetStaticMesh()->RenderData->LODResources.Num() > 0)
            {
                FPositionVertexBuffer* VertexBuffer = &StaticMeshComponent->GetStaticMesh()->RenderData->LODResources[0].VertexBuffers.PositionVertexBuffer;
                if (VertexBuffer)
                {
                    Occluded = !(StaticMeshComponent->GetWorld()->GetTimeSeconds() - StaticMeshComponent->LastRenderTimeOnScreen <= 0.1f);
                    
                    FTransform const MeshWorldTransform = StaticMeshComponent->GetComponentTransform();

                    const int32 VertexCount = VertexBuffer->GetNumVertices();
                    
                    if (VertexCount > 0)
                    {
                        Valid = true;
                    }
                    
                    for (int32 Index = 0; Index < VertexCount; Index++)
                    {
                        Points.Add(MeshWorldTransform.TransformPosition(VertexBuffer->VertexPosition(Index)));
                    }
                } else
                {
                    Valid = false;
                }     
            }
        } else
        {
            Valid = false;
        }
    }

    if (!Valid)
    {
        return false;
    }

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
        if (Pixel.X >= (RenderTexture->SizeX * -0.01) && Pixel.X <= (RenderTexture->SizeX + RenderTexture->SizeX * 0.01) && Pixel.Y >= (RenderTexture->SizeY * -0.01) && Pixel.Y <= (RenderTexture->SizeY + RenderTexture->SizeY * 0.01))
        {
            Points2D.Add(Pixel);
            MaxPixel.X = FMath::Max(Pixel.X, MaxPixel.X);
            MaxPixel.Y = FMath::Max(Pixel.Y, MaxPixel.Y);
            MinPixel.X = FMath::Min(Pixel.X, MinPixel.X);
            MinPixel.Y = FMath::Min(Pixel.Y, MinPixel.Y);
        }
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