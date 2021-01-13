// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FollowerSpawner.generated.h"

UCLASS()
class MELLOWSMEGARIDE_API AFollowerSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFollowerSpawner();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spawner)
	int SphereValue = 1;

	int index;

	bool bIsOccupied = false;

	void SpawnSphere();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UStaticMeshComponent* EditorSphere;

};
