// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Deathzone.generated.h"

UCLASS()
class MELLOWSMEGARIDE_API ADeathzone : public AActor
{
	GENERATED_BODY()
	
public:	
	ADeathzone();

	UPROPERTY(Category = Plane, VisibleDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* DeathZoneMesh;

	UPROPERTY(Category = Box, VisibleDefaultsOnly, BlueprintReadOnly)
	class UBoxComponent* KillBox;

	UFUNCTION()
	void OnKillBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
