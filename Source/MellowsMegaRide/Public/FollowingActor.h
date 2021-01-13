// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h" 	

#include "TimerManager.h"
#include "UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "FollowingActor.generated.h"

UCLASS()
class MELLOWSMEGARIDE_API AFollowingActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFollowingActor();

	void FollowTarget(AActor* newNext);
	void SetNextOnPrevoiusElement(AActor* newNext);
	AFollowingActor* GetLast(uint32 maxIteration);

	void DropFromHere();

	uint32 FollowerCount();

	AFollowingActor* GetAt(uint32 followerIndex);

	virtual void Tick(float DeltaTime) override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	bool ContainsFollowingActor(AFollowingActor* followingActor);

	bool IsChainTargeted();

	// score one player gets from this follower when delivered
	uint32 Value = 1;

	FTimerHandle CheckTargetableTimer;
	bool bIsTargeted = false;

	void CheckTargeted();

	UFUNCTION()
		virtual void OnRep_LocationChanged();

	void UpdateTarget(FVector target);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	UPROPERTY(Replicated)
	AActor* previous = nullptr;
	UPROPERTY(Replicated)
	AFollowingActor* next = nullptr;


	UPROPERTY(EditAnywhere)
	UMaterialInterface* VisibleMaterial = nullptr;
	UPROPERTY(EditAnywhere)
	UMaterialInterface* InvisibleMaterial = nullptr;

	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* StaticMesh;

	UPROPERTY(Category = Particles, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UParticleSystemComponent* Lightning;
	UPROPERTY(Category = Particles, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UParticleSystemComponent* Pillar;

	UPROPERTY(ReplicatedUsing = OnRep_LocationChanged)
		FVector ServerLocationTarget;

	UFUNCTION(NetMulticast, reliable)
		void Multi_SetTickEnabled(bool tickEnabled);
	void Multi_SetTickEnabled_Implementation(bool tickEnabled);




	void SetVisibilityLocal(bool bIsVisible);


	UFUNCTION(NetMulticast, reliable)
		void Multi_UpdateReferences(AActor* serverPrev, AFollowingActor* serverNext);
	void Multi_UpdateReferences_Implementation(AActor* serverPrev, AFollowingActor* serverNext);
	UFUNCTION(NetMulticast, reliable)
		void Multi_UpdateTargetedState(bool isTargeted);
	void Multi_UpdateTargetedState_Implementation(bool isTargeted);

		void StartTargetForParent();

		void EndTargetForParent();

		void StartTargetForFollowing();

		void EndTargetForFollowing();

	FVector LocationTarget;

	class AFollowerSpawner* OwningSpawner = nullptr;
};
