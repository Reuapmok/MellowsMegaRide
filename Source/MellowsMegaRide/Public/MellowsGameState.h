// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MellowsPlayerState.h"
#include "MellowsGameState.generated.h"

/**
 * 
 */

class AFollowerSpawner;
class AMellowsPlayerController;

UCLASS()
class AMellowsGameState : public AGameState
{
	GENERATED_BODY()
	

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = Timing)
	float GetRemainingTime();

	void PlayerIsReady(AMellowsPlayerController* PlayerController);


	//void sortJoinedPlayersStates();


	// Add replicated properties here
	//void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	TArray<AMellowsPlayerController*> JoinedPlayers;

		FVector ServerVelocityTarget;


	//UFUNCTION()
	//	virtual void OnRep_PlayerBoardChanged();

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)//ReplicatedUsing = OnRep_PlayerBoardChanged)
	//TArray<AMellowsPlayerState*> JoinedPlayersStates;

		UFUNCTION(NetMulticast, reliable)
		void Multicast_AddPlayerboardEntry();
		void Multicast_AddPlayerboardEntry_Implementation();

	//UFUNCTION(NetMulticast, reliable)
	//void Multicast_UpdatePlayerboardScores();
	//void Multicast_UpdatePlayerboardScores_Implementation();



protected:


	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Timing)
	float MatchTime = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Timing)
	float TimeTillRestart = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Timing)
		float RemainingTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Timing)
	float TimeToJoin = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Timing)
	int CountDown = 3;

	// because one second is really short choose a suitable time step for counting down
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Timing)
	float CountDownTimeStep = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Timing)
	bool bInfiniteGame = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spawner, meta=(ClampMin="0"))
	int StartingSpheresNum = 1;

	UPROPERTY(EditAnywhere, Category = Spawner)
	int TimeTillNextRespawn = 20;

	UPROPERTY(EditAnywhere, Category = Spawner)
	int RespawningSpheresNum = 1;

	FTimerHandle SphereSpawnerHandle;
	FTimerHandle ServerMatchTimerHandle;
	FTimerHandle ClientMatchTimerHandle;
	FTimerHandle TimeToJoinTimerHandle;
	FTimerHandle CountDownTimerHandle;
	FTimerHandle RestartGameTimerHandle;

	UFUNCTION(NetMulticast, reliable)
	void Multicast_StartCountGametime();
	void Multicast_StartCountGametime_Implementation();

	UFUNCTION(NetMulticast, reliable)
	void Multicast_StartCountDown(uint32 Remaining);
	void Multicast_StartCountDown_Implementation(uint32 Remaining);

	UFUNCTION(NetMulticast, reliable)
	void Multicast_StartCountGameTimeLateJoiner(uint32 PlayerId, int Remaining);
	void Multicast_StartCountGameTimeLateJoiner_Implementation(uint32 PlayerId, int Remaining);

	//UFUNCTION(NetMulticast, reliable)
	//void Multicast_UpdatePlayerboardEntries();
	//void Multicast_UpdatePlayerboardEntries_Implementation();



	void OnGameOver();
	void FindWinner();


	virtual void HandleMatchHasStarted() override;

private:
	// server only
	void GetAllSphereSpawners();
	void SelectStartingSpawners();

	bool bGameStarted = false;

	UFUNCTION()
	void OnRespawnSpheres();

	UFUNCTION()
	void CallCountDown();

	UFUNCTION()
	void RestartGame();

	TArray<AFollowerSpawner*> FollowerSpawnerList;
};
