#include "GameFeelImpactSparks.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

#include "UTween.h"

AGameFeelImpactSparks::AGameFeelImpactSparks()
{
    PrimaryActorTick.bCanEverTick = true;

    RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
    SetRootComponent(RootComp);
}

void AGameFeelImpactSparks::TriggerSparks(const FVector& HitDirection)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    static UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    static UMaterialInterface* SparkMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/GameFeel/Materials/MI_GameFeel_Sparks.MI_GameFeel_Sparks"));
    if (!SparkMat)
    {
        SparkMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/GameFeel/Materials/MI_GameFeel_Sword.MI_GameFeel_Sword"));
    }

    bIsActive = true;
    ElapsedLifetime = 0.f;

    for (int32 i = 0; i < SparkCount; ++i)
    {
        UStaticMeshComponent* SparkMesh = NewObject<UStaticMeshComponent>(this);
        if (!SparkMesh) continue;

        SparkMesh->RegisterComponent();
        SparkMesh->AttachToComponent(RootComp, FAttachmentTransformRules::KeepRelativeTransform);
        SparkMesh->SetRelativeLocation(FVector::ZeroVector);
        SparkMesh->SetRelativeScale3D(SparkScale);
        SparkMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        if (CubeMesh) SparkMesh->SetStaticMesh(CubeMesh);
        if (SparkMat) SparkMesh->SetMaterial(0, SparkMat);

        FVector ConeDir = (HitDirection + FMath::VRand() * 0.55f).GetSafeNormal();
        ConeDir.Z = FMath::Abs(ConeDir.Z) * 0.4f + 0.2f;

        const float Speed = FMath::RandRange(MinSpeed, MaxSpeed);

        FSparkParticle Particle;
        Particle.MeshComp = SparkMesh;
        Particle.Velocity = ConeDir * Speed;
        Particles.Add(Particle);

        UTween::ComponentRelativeScale(SparkMesh, FVector::ZeroVector, SparkLifetime)
            .SetEase(ETweenEase::InQuad);

        SparkMeshes.Add(SparkMesh);
    }

    OnSparksTriggered(HitDirection);
}

void AGameFeelImpactSparks::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsActive)
    {
        return;
    }

    ElapsedLifetime += DeltaTime;

    for (FSparkParticle& Particle : Particles)
    {
        if (Particle.MeshComp.IsValid())
        {
            FVector CurrentLoc = Particle.MeshComp->GetRelativeLocation();
            CurrentLoc += Particle.Velocity * DeltaTime;
            Particle.Velocity += FVector(0.f, 0.f, -980.f) * DeltaTime;
            Particle.MeshComp->SetRelativeLocation(CurrentLoc);
        }
    }

    if (ElapsedLifetime >= SparkLifetime)
    {
        Destroy();
    }
}