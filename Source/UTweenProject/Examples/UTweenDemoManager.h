#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UTweenDemoManager.generated.h"

class AStaticMeshActor;
class UStaticMesh;
class UMaterialInterface;
class UWidgetComponent;

/**
 * Spawns a line of primitive shapes at BeginPlay and attaches a distinct
 * UTween animation to each one. Shows off single tweens, sequences, parallel
 * groups, nested sequences, and parametric value tweens.
 */
UCLASS(Blueprintable, BlueprintType)
class UTWEENPROJECT_API AUTweenDemoManager : public AActor
{
    GENERATED_BODY()

public:
    AUTweenDemoManager();

    virtual void Tick(float DeltaTime) override;

    /** Number of shapes to spawn. Demos loop if this exceeds the number of presets. */
    UPROPERTY(EditAnywhere, Category = "UTween|Demo", meta = (ClampMin = "1", ClampMax = "64"))
    int32 NumShapes = 10;

    /** Spacing between shapes along LineDirection, in Unreal units. */
    UPROPERTY(EditAnywhere, Category = "UTween|Demo")
    float Spacing = 240.f;

    /** Direction along which shapes are arranged. Normalized at runtime. */
    UPROPERTY(EditAnywhere, Category = "UTween|Demo")
    FVector LineDirection = FVector(0.f, 1.f, 0.f);

    /** Mesh used for every spawned shape. Defaults to SM_TweenDemo_Cube. */
    UPROPERTY(EditAnywhere, Category = "UTween|Demo")
    TObjectPtr<UStaticMesh> ShapeMesh;

    /** Material applied to every spawned shape. Defaults to MI_TweenDemo_Cube. */
    UPROPERTY(EditAnywhere, Category = "UTween|Demo")
    TObjectPtr<UMaterialInterface> ShapeMaterial;

    UPROPERTY(VisibleAnywhere, Category = "UTween|Demo")
    TArray<TObjectPtr<AStaticMeshActor>> SpawnedActors;

    TArray<TObjectPtr<UWidgetComponent>> SpawnedLabels;

protected:
    virtual void BeginPlay() override;

    AStaticMeshActor* SpawnShape(int32 Index);
    UWidgetComponent* SpawnLabelForShape(int32 Index, AStaticMeshActor* Shape);
    void GetDemoText(int32 Index, FString& OutTitle, FString& OutSubtitle) const;
    FLinearColor GetDemoColor(int32 Index) const;
    void StartAnimationFor(int32 Index, AStaticMeshActor* Shape);

    void Demo0_VerticalBounce(AStaticMeshActor* Shape, const FVector& Base);
    void Demo1_ContinuousSpin(AStaticMeshActor* Shape);
    void Demo2_ScalePulse(AStaticMeshActor* Shape);
    void Demo3_SlideSequence(AStaticMeshActor* Shape, const FVector& Base);
    void Demo4_ElasticPop(AStaticMeshActor* Shape);
    void Demo5_BarrelRoll(AStaticMeshActor* Shape);
    void Demo6_Orbit(AStaticMeshActor* Shape, const FVector& Base);
    void Demo7_JumpFlip(AStaticMeshActor* Shape, const FVector& Base);
    void Demo8_ParallelCombo(AStaticMeshActor* Shape, const FVector& Base);
    void Demo9_NestedShake(AStaticMeshActor* Shape, const FVector& Base);
};
