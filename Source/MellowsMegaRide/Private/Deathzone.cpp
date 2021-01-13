// Fill out your copyright notice in the Description page of Project Settings.


#include "Deathzone.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "MellowsPawn.h"

// Sets default values
ADeathzone::ADeathzone()
{
	PrimaryActorTick.bCanEverTick = true;

	DeathZoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeathZoneMesh"));
	bool rootSetupSuccessful = SetRootComponent(DeathZoneMesh);
	check(rootSetupSuccessful && "Root Component could not be set in Deathzone")
	KillBox = CreateDefaultSubobject<UBoxComponent>(TEXT("KillBox"));
	KillBox->SetupAttachment(RootComponent);

	KillBox->OnComponentBeginOverlap.AddDynamic(this, &ADeathzone::OnKillBoxOverlap);
}

void ADeathzone::OnKillBoxOverlap(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		AMellowsPawn* pawn = Cast<AMellowsPawn>(OtherActor);
		check(pawn != nullptr && "Pawn is nullptr... in Deathzone");
		pawn->Destroy();
	}
}

// Called when the game starts or when spawned
void ADeathzone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADeathzone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

