// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeliveryStation.generated.h"

UCLASS()
class MELLOWSMEGARIDE_API ADeliveryStation : public AActor
{
	GENERATED_BODY()
	
public:	
	ADeliveryStation();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnDeliveryZoneOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, reliable)
	void Multicast_Scored(uint32 Points);
	void Multicast_Scored_Implementation(uint32 Points);

	UFUNCTION(Category = "Points", BlueprintImplementableEvent, BlueprintCallable)
	void OnScored(int Points);

	UPROPERTY(Category = Delivery, VisibleDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* DeliveryStationMesh;

	UPROPERTY(Category = Delivery, VisibleDefaultsOnly, BlueprintReadOnly)
	class USphereComponent* DeliveryZone;

public:	
	virtual void Tick(float DeltaTime) override;

};
