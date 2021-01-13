// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MellowsPlayerState.generated.h"

UENUM(BlueprintType)
enum class EVictoryStatusEnum : uint8
{
	VE_Undecided,
	VE_Winner,
	VE_Loser
};

UENUM(BlueprintType)
enum class EColorSchemeEnum : uint8
{
	CSE_Brown,
	CSE_Red,
	CSE_Green,
	CSE_Purple,
	CSE_Blue,
	CSE_Pink
};

/**
 * 
 */
UCLASS()
class AMellowsPlayerState : public APlayerState
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = PlayerState)
	int Points = 0;

public:

	UPROPERTY(replicated, BlueprintReadOnly, Category = PlayerState)
	int FollowingActorsCount = 0;

	UPROPERTY(replicated, BlueprintReadOnly, Category = PlayerState)
	EColorSchemeEnum ColorScheme = EColorSchemeEnum::CSE_Brown;

	virtual void Destroyed() override;
	/**
	* Adds Points to accumulated score, server only
	* @param rewardedPoints amount of points that should be added
	*/
	UFUNCTION(NetMulticast, reliable, Category = "Score")
	void Multicast_AddScore(int rewardedPoints);
	void Multicast_AddScore_Implementation(int rewardedPoints);

	UFUNCTION(BlueprintCallable, Category = "Score")
	int GetScore();

	UFUNCTION(BlueprintCallable, Category = "FollowingActors")
	void SetFollowingActorsCount(int followingActors);

	UFUNCTION(NetMulticast, reliable)
	void Multicast_SetFollowingActorsCount(int followingActors);
	void Multicast_SetFollowingActorsCount_Implementation(int followingActors);

	UFUNCTION(BlueprintCallable, Category = "FollowingActors")
	void AddFollowingActorsCount(int addedfollowingActors);

	UFUNCTION(Client, reliable)
	void Client_MatchResults(EVictoryStatusEnum MatchResult);
	void Client_MatchResults_Implementation(EVictoryStatusEnum MatchResult);
};
