// Fill out your copyright notice in the Description page of Project Settings.


#include "FollowerSpawner.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "FollowingActor.h"
#include "Engine/World.h"


#define COLLISION_PICKUP        ECC_GameTraceChannel2

// Sets default values
AFollowerSpawner::AFollowerSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	this->SetActorHiddenInGame(true);

	this->bReplicates = true;
	NetUpdateFrequency = 10.f;
	MinNetUpdateFrequency = 5.f;

	NetPriority = 5;

	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> Mesh;
		FConstructorStatics() : Mesh(TEXT("StaticMesh'/Game/Challenges/DuckChallenge/SM_Editor_SphereSpawner.SM_Editor_SphereSpawner'")) {}
	}ConstructorStatics;

	EditorSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Editor"));
	EditorSphere->SetStaticMesh(ConstructorStatics.Mesh.Get());
	EditorSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);


	EditorSphere->SetCollisionObjectType(COLLISION_PICKUP);
	EditorSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	EditorSphere->SetCollisionResponseToChannel(COLLISION_PICKUP, ECollisionResponse::ECR_Overlap);
	RootComponent = EditorSphere;

	SetActorTickInterval(1.f);
}

void AFollowerSpawner::SpawnSphere()
{
	if (GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		FActorSpawnParameters SpawnParams = FActorSpawnParameters();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
		SpawnParams.Owner = this;
		AFollowingActor* actor = GetWorld()->SpawnActor<AFollowingActor>(AFollowingActor::StaticClass(), this->GetActorTransform(), SpawnParams);
		if (actor)
		{
			//UE_LOG(LogTemp, Log, TEXT("Spawner with index: %d"), index);
			bIsOccupied = true;

			actor->Value = this->SphereValue;
			actor->OwningSpawner = this;
			actor->Multi_SetTickEnabled(true);
			ForceNetUpdate();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Did not spawn Sphere"));
		}
	}
}

void AFollowerSpawner::BeginPlay()
{
	Super::BeginPlay();

}

void AFollowerSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TArray<AActor*> OverlappingActors;

	if (GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		if (bIsOccupied)
		{
			EditorSphere->GetOverlappingActors(OverlappingActors, AFollowingActor::StaticClass());


			if (OverlappingActors.Num() == 0)
			{
				UE_LOG(LogTemp, Error, TEXT("Follower Spawner should be occupied (no overlap) with index: %d"), this->index);
				bIsOccupied = false;
			}
			else
			{
				bool bHasRightOverlappedActor = false;
				for (size_t i = 0; i < OverlappingActors.Num(); i++)
				{
					AFollowingActor* overlappedFollower = Cast<AFollowingActor>(OverlappingActors[i]);
					if (overlappedFollower)
					{
						if (overlappedFollower->OwningSpawner == this)
						{
							bHasRightOverlappedActor = true;
						}
					}
				}
				if (!bHasRightOverlappedActor)
				{
					UE_LOG(LogTemp, Error, TEXT("Follower Spawner should be occupied (with overlap) with index: %d"), this->index);
					bIsOccupied = false;
				}
			}

		}
	}

}

