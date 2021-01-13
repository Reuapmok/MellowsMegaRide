// Fill out your copyright notice in the Description page of Project Settings.

#include "FollowingActor.h"
#include "MellowsPawn.h"


#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "FollowerSpawner.h"

#define COLLISION_PICKUP        ECC_GameTraceChannel2

// Sets default values
AFollowingActor::AFollowingActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bReplicateMovement = false;
	bAlwaysRelevant = true;
	NetUpdateFrequency = 10.f;
	MinNetUpdateFrequency = 5.f;
	SetActorTickInterval(0.f);
	SetActorTickEnabled(true);

	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> Mesh;
		FConstructorStatics() : Mesh(TEXT("StaticMesh'/Game/Challenges/DuckChallenge/SM_Energy_Sphere.SM_Energy_Sphere'")) {}
	}ConstructorStatics;

	struct FConstructorStaticsMat1
	{
		ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> Material;
		FConstructorStaticsMat1() : Material(TEXT("Material'/Game/Challenges/DuckChallenge/M_EnergySphere.M_EnergySphere'")) {}
	}ConstructorStaticsMat1;
	struct FConstructorStaticsMat2
	{
		ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> Material;
		FConstructorStaticsMat2() : Material(TEXT("MaterialInstanceConstant'/Game/FadeObjects/Materials/M_FadeMaterial_Inst.M_FadeMaterial_Inst'")) {}
	}ConstructorStaticsMat2;

	VisibleMaterial = ConstructorStaticsMat1.Material.Get();
	InvisibleMaterial = ConstructorStaticsMat2.Material.Get();

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh0"));
	StaticMesh->SetStaticMesh(ConstructorStatics.Mesh.Get());
	StaticMesh->SetIsReplicated(false);
	StaticMesh->SetRenderCustomDepth(true);
	StaticMesh->SetCustomDepthStencilValue(252);

	NetUpdateFrequency = 100.f;
	MinNetUpdateFrequency = 20.f;

	/// Only overlap with pickup spheres 
	StaticMesh->SetCollisionObjectType(COLLISION_PICKUP);
	StaticMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	StaticMesh->SetCollisionResponseToChannel(COLLISION_PICKUP, ECollisionResponse::ECR_Overlap);
	StaticMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Overlap);
	SetRootComponent(StaticMesh);
	LocationTarget = GetActorLocation();

	static ConstructorHelpers::FObjectFinder<UParticleSystem> LightningPS(TEXT("ParticleSystem'/Game/Effects/Attaching/PS_SphereLightning.PS_SphereLightning'"));

	Lightning = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("LightningParticles0"));
	Lightning->SetTemplate(LightningPS.Object);
	Lightning->bAutoActivate = false;
	Lightning->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UParticleSystem> PillarPS(TEXT("ParticleSystem'/Game/Effects/Pillar/PS_PillarOrb.PS_PillarOrb'"));

	Pillar = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("PillarParticles0"));
	Pillar->SetTemplate(PillarPS.Object);
	Pillar->bAutoActivate = true;
	Pillar->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AFollowingActor::BeginPlay()
{
	Super::BeginPlay();
	LocationTarget = GetActorLocation();
	if (GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		GetWorld()->GetTimerManager().SetTimer(CheckTargetableTimer, this, &AFollowingActor::CheckTargeted,
			0.5f, true);
		GetWorld()->GetTimerManager().PauseTimer(CheckTargetableTimer);

		//OwningSpawner = Cast<AFollowerSpawner>(GetOwner());
		//UE_LOG(LogTemp, Warning, TEXT("Came to life in index: %d"), OwningSpawner->index);
		//if (!OwningSpawner)
		//{
		//	UE_LOG(LogTemp, Error, TEXT("OwningSpawner is nullptr"));
		//}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Sphere Spawned on client"));
	}

}


// Called every frame
void AFollowingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetActorLocation(FMath::VInterpTo(GetActorLocation(), LocationTarget, DeltaTime, 15.f));
	if (GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		if (previous)
		{
			AMellowsPawn* pawn = Cast<AMellowsPawn>(previous);
			if (pawn)
			{
				if (pawn->FirstFollowingActor != this)
				{
					UE_LOG(LogTemp, Error, TEXT("chain incomplete: pawn->FirstFollowingActor != this"));
				}
			}
			else
			{
				AFollowingActor* previousFollower = Cast<AFollowingActor>(previous);
				if (previousFollower)
				{
					if (previousFollower->next != this)
					{
						UE_LOG(LogTemp, Error, TEXT("chain incomplete: previousFollower->next != this"));
					}
				}
			}

			ServerLocationTarget = GetActorLocation();
		}

		if (next)
		{
			if (next->previous != this)
			{
				UE_LOG(LogTemp, Error, TEXT("chain incomplete: next->previous != this"));
			}
		}

	}

	if (next)
	{
		next->UpdateTarget(GetActorLocation());
	}

	if (previous)
	{
		if (!Lightning->IsActive())
		{
			Lightning->Activate(true);
		}

		Lightning->SetBeamTargetPoint(0, previous->GetActorLocation(), 0);
		Lightning->SetBeamSourcePoint(0, GetActorLocation(), 0);
	}
	else
	{
		if (Lightning->IsActive())
		{
			Lightning->Deactivate();
		}
	}

}

void AFollowingActor::Multi_SetTickEnabled_Implementation(bool tickEnabled)
{
	SetActorTickEnabled(tickEnabled);
}

void AFollowingActor::SetVisibilityLocal(bool bIsVisible)
{
	if (bIsVisible)
	{
		if (VisibleMaterial)
			StaticMesh->SetMaterial(0, VisibleMaterial);
	}
	else
	{
		if (InvisibleMaterial)
			StaticMesh->SetMaterial(0, InvisibleMaterial);
	}

	if (next)
	{
		next->SetVisibilityLocal(bIsVisible);
	}
}

void AFollowingActor::Multi_UpdateReferences_Implementation(AActor* serverPrev, AFollowingActor* serverNext)
{
	bIsTargeted = false;
	previous = serverPrev;
	next = serverNext;
}

void AFollowingActor::Multi_UpdateTargetedState_Implementation(bool isTargeted)
{
	bIsTargeted = isTargeted;
}

void AFollowingActor::StartTargetForParent()
{
	StaticMesh->SetScalarParameterValueOnMaterials("EmissiveStrength", 500.f);
	StaticMesh->SetVectorParameterValueOnMaterials("Color", FVector(0.f, 1.f, 0.f));
}

void AFollowingActor::EndTargetForParent()
{
	StaticMesh->SetScalarParameterValueOnMaterials("EmissiveStrength", 10.f);
	StaticMesh->SetVectorParameterValueOnMaterials("Color", FVector(1.f, 1.f, 1.f));

}

void AFollowingActor::StartTargetForFollowing()
{
	StartTargetForParent();
	if (next)
	{
		next->StartTargetForFollowing();
	}
}

void AFollowingActor::EndTargetForFollowing()
{
	EndTargetForParent();
	if (next)
	{
		next->EndTargetForFollowing();
	}
}

void AFollowingActor::FollowTarget(AActor* Target)
{
	if (GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		if (Target != nullptr)
		{
			GetWorld()->GetTimerManager().UnPauseTimer(CheckTargetableTimer);
			/// inform spawner that sphere was collected
			if (this->OwningSpawner)
			{
				UE_LOG(LogTemp, Warning, TEXT("Collected from Owning spawner with index: %d"), OwningSpawner->index);
				this->OwningSpawner->bIsOccupied = false;
				this->OwningSpawner = nullptr;
			}

			// Clear old reference to this 
			SetNextOnPrevoiusElement(nullptr);
			// Set reference to the target this should follow
			previous = Target;
			// update target to let this follow
			SetNextOnPrevoiusElement(this);

			/// update replication Replication
			ServerLocationTarget = GetActorLocation();
			Multi_UpdateReferences(previous, next);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("tried following nullptr"));
		}

	}
}

void AFollowingActor::SetNextOnPrevoiusElement(AActor* newNext)
{
	AFollowingActor* asFollower = Cast<AFollowingActor>(previous);
	if (asFollower)
	{
		asFollower->next = Cast<AFollowingActor>(newNext);
		asFollower->Multi_UpdateReferences(asFollower->previous, asFollower->next);
	}
	else
	{
		AMellowsPawn* asPawn = Cast<AMellowsPawn>(previous);
		if (asPawn)
		{
			asPawn->FirstFollowingActor = Cast<AFollowingActor>(newNext);
			asPawn->Multi_UpdateFollowerReference(asPawn->FirstFollowingActor);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Previous is not a follower or pawn when setting next... setting previous to nullptr"));
			previous = nullptr;
		}
	}
}

AFollowingActor* AFollowingActor::GetLast(uint32 maxIteration)
{
	if (next == nullptr)
	{
		return this;
	}
	else
	{
		if (maxIteration == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Infinite loop detection for GetLast()"));
			return nullptr;
		}

		return next->GetLast(--maxIteration);
	}
}

void AFollowingActor::DropFromHere()
{
	if (GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		AFollowingActor* overlappedPreviousFollowing = Cast<AFollowingActor>(previous);
		ServerLocationTarget = GetActorLocation();
		if (overlappedPreviousFollowing)
		{
			overlappedPreviousFollowing->next = (nullptr);
		}
		else
		{
			AMellowsPawn* overlappedPreviousPawn = Cast<AMellowsPawn>(previous);
			if (overlappedPreviousPawn)
			{
				overlappedPreviousPawn->FirstFollowingActor = (nullptr);
			}
		}
		LocationTarget = GetActorLocation();
		if (next)
		{
			next->DropFromHere();
			next = (nullptr);
		}
	}
}

uint32 AFollowingActor::FollowerCount()
{
     	return next ? next->FollowerCount() + 1 : 1;
}

AFollowingActor* AFollowingActor::GetAt(uint32 followerIndex)
{
	if (next && followerIndex > 0)
	{
		return  next->GetAt(followerIndex - 1);
	}
	else if (followerIndex == 0)
	{
		return this;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Too large index in FollowingActor::GetAt"));
		return nullptr;
	}


}



void AFollowingActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFollowingActor, ServerLocationTarget);
	DOREPLIFETIME(AFollowingActor, previous);
	DOREPLIFETIME(AFollowingActor, next);
}

bool AFollowingActor::ContainsFollowingActor(AFollowingActor* followingActor)
{
	if (this == followingActor)
	{
		return true;
	}
	else if (next == nullptr)
	{
		return false;
	}
	else
	{
		return next->ContainsFollowingActor(followingActor);
	}
}

bool AFollowingActor::IsChainTargeted()
{
	if (bIsTargeted)
		return true;
	if (next)
		return next->IsChainTargeted();
	else
		return false;
}

// Checks all MellowsPawns if they overlap with this sphere
void AFollowingActor::CheckTargeted()
{
	TSubclassOf<AMellowsPawn> classToFind;
	classToFind = AMellowsPawn::StaticClass();
	TArray<AActor*> FoundPawns;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), classToFind, FoundPawns);

	bool bIsTargetedByAny = false;
	for (size_t i = 0; i < FoundPawns.Num(); i++)
	{
		AMellowsPawn* mellowsPawn = Cast<AMellowsPawn>(FoundPawns[i]);
		const bool bIsOwningPawn = mellowsPawn->FirstFollowingActor && mellowsPawn->FirstFollowingActor->ContainsFollowingActor(this);
		if (!bIsOwningPawn && StaticMesh->IsOverlappingComponent(Cast<AMellowsPawn>(FoundPawns[i])->TargetingCone))
		{
			bIsTargetedByAny = true;
		}
	}
	bIsTargeted = bIsTargetedByAny;

}

void AFollowingActor::OnRep_LocationChanged()
{
	const FVector UpdatedPos = FVector::Distance(GetActorLocation(), ServerLocationTarget) < 5000.f ? FMath::VInterpTo(GetActorLocation(), ServerLocationTarget, GetWorld()->GetDeltaSeconds(), 8.f) : ServerLocationTarget;
	SetActorLocation(UpdatedPos, false, nullptr, ETeleportType::TeleportPhysics);
}

void AFollowingActor::UpdateTarget(FVector target)
{
	const FVector currentLocation = GetActorLocation();
	FVector DistanceVector = currentLocation - target;
	DistanceVector /= DistanceVector.Size();
	DistanceVector *= 275.f;
	LocationTarget = target + DistanceVector;
}
