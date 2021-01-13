// Fill out your copyright notice in the Description page of Project Settings.


#include "ChargePickup.h"
#include "MellowsPawn.h"
#include "UObject/ConstructorHelpers.h"

#define COLLISION_PICKUP        ECC_GameTraceChannel2

// Sets default values
AChargePickup::AChargePickup()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> Mesh;
		FConstructorStatics() : Mesh(TEXT("StaticMesh'/Game/Art/Pickups/S_FuelCell.S_FuelCell'")) {}
	}ConstructorStatics;
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh0"));
	PickupMesh->SetStaticMesh(ConstructorStatics.Mesh.Get());

	if (!SetRootComponent(PickupMesh)) { UE_LOG(LogTemp, Error, TEXT("Failed to set RootComponent")); }
	PickupMesh->SetSimulatePhysics(false);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupMesh->SetIsReplicated(true);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere0"));
	CollisionSphere->SetSphereRadius(200.f);
	CollisionSphere->SetupAttachment(RootComponent);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AChargePickup::OnCollisionSphereBeginOverlap);

	/// Only overlap with pickup spheres 
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(COLLISION_PICKUP);
	CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(COLLISION_PICKUP, ECollisionResponse::ECR_Overlap);

	if (GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		//CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		SetActorTickEnabled(false);
	}



	SetActorTickInterval(0.1f);
}

// Called when the game starts or when spawned
void AChargePickup::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AChargePickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		if (!PickupMesh->bVisible)
		{
			TimeToRespawn -= DeltaTime;
			if (TimeToRespawn < 0.f)
			{
				Multicast_SetVisibility(true);
				TimeToRespawn = RespawnTime;
			}
		}
	}

}

void AChargePickup::OnCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (GetNetMode() == ENetMode::NM_DedicatedServer)
	{


		AMellowsPawn* overlappedActor = Cast<AMellowsPawn>(OtherActor);
		if (overlappedActor)
		{
			if (OtherComp->ComponentHasTag("PickupSphere"))
			{
				Multicast_SetVisibility(false);
				overlappedActor->MovementComponent->AddImpulse(overlappedActor->MovementComponent->BaseMaxCharge);
				overlappedActor->MovementComponent->AddCharge(overlappedActor->MovementComponent->BaseMaxCharge);
				overlappedActor->Client_PickedUpImpulseMessage();
				overlappedActor->Client_PlaySound(overlappedActor->MovementComponent->ImpulseReadySound);

				UE_LOG(LogTemp, Warning, TEXT("Pickup detected"));
			}

		}
	}
}

void AChargePickup::Multicast_SetVisibility_Implementation(bool visibility)
{
	PickupMesh->SetVisibility(visibility);
}
