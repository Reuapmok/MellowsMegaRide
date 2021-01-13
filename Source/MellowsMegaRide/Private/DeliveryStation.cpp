// Fill out your copyright notice in the Description page of Project Settings.


#include "DeliveryStation.h"
#include "Net/UnrealNetwork.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "MellowsPawn.h"
#include "FollowingActor.h"
#include "MellowsGameState.h"
#include "MellowsPlayerState.h"

#define COLLISION_PICKUP        ECC_GameTraceChannel2

// Sets default values
ADeliveryStation::ADeliveryStation()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	DeliveryStationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeliveryStationMesh"));
	bool rootSetupSuccessful = SetRootComponent(DeliveryStationMesh);
	check(rootSetupSuccessful && "Root component could not be set in delivery station")
	DeliveryZone = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Zone"));
	DeliveryZone->SetupAttachment(RootComponent);
	DeliveryZone->SetCollisionObjectType(COLLISION_PICKUP);
	DeliveryZone->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	DeliveryZone->SetCollisionResponseToChannel(COLLISION_PICKUP, ECollisionResponse::ECR_Overlap);

	DeliveryZone->OnComponentBeginOverlap.AddDynamic(this, &ADeliveryStation::OnDeliveryZoneOverlap);
}


void ADeliveryStation::BeginPlay()
{
	Super::BeginPlay();
}

void ADeliveryStation::OnDeliveryZoneOverlap(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (GetNetMode() == NM_DedicatedServer)
	{

		AMellowsPawn* MellowsPawn = Cast<AMellowsPawn>(OtherActor);

		if (MellowsPawn && OtherComp->ComponentHasTag("PickupSphere"))
		{
			uint32 accumulatedScore = 0;
			AFollowingActor* currentFollower = MellowsPawn->FirstFollowingActor;
			if (!currentFollower)
			{
				UE_LOG(LogTemp, Error, TEXT("Follower is null"));
			}

			while (currentFollower)
			{
				accumulatedScore += currentFollower->Value;
				// possible because follower is destroyed at the end of tick
				currentFollower->Destroy();
				currentFollower = currentFollower->next;
			}

			accumulatedScore = accumulatedScore + accumulatedScore / 1.4;

			//AMellowsGameState* GameState = Cast<AMellowsGameState>(GetWorld()->GetGameState());
			//if (GameState)
			//{
			//	GameState->Multicast_UpdatePlayerboardScores();
			//}
			
			MellowsPawn->FirstFollowingActor = nullptr;
			AMellowsPlayerState* playerState;
			playerState = Cast<AMellowsPlayerState>(MellowsPawn->GetPlayerState());
			check(playerState != nullptr);

			if (accumulatedScore > 0)
			{
				MellowsPawn->Client_DeliveredOrbsMessage(accumulatedScore);
				MellowsPawn->Client_PlaySound(MellowsPawn->DeliverSound);
			}

			// RPC to client that he has been delivered the orbs
			playerState->Multicast_AddScore(accumulatedScore);
			MellowsPawn->UpdateFollowingActorsCount();
			
			Multicast_Scored(accumulatedScore);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Something that is not a MellowsPawn hit the delivery station?"));
		}
	}
}

void ADeliveryStation::Multicast_Scored_Implementation(uint32 Points)
{
	if (GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Log, TEXT("Scored event called"));
		OnScored(Points);
	}
}


void ADeliveryStation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

