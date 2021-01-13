// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MellowsPlayerState.h"
#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MellowsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class AMellowsGameMode : public AGameMode
{
	GENERATED_BODY()
	
protected:

	bool bWaitingTimeOver = false;

	// game will start when MaxNumPlayers joined game
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameMode)
	int MaxNumPlayers = 4;

	// each time a player joins, the game will wait another specified seconds before starting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GameMode)
	float WaitingTime = 10;

	FTimerHandle WaitingTimeHandle;

	void OnWaitingTimeOver();
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual bool ReadyToStartMatch_Implementation() override;

	virtual FString InitNewPlayer(
		APlayerController * NewPlayerController,
		const FUniqueNetIdRepl & UniqueId,
		const FString & Options,
		const FString & Portal
	);

	bool ParseCommandlineParam(const TCHAR* Stream, const TCHAR* Match, uint32& Value);

private:
	unsigned int JoinedPlayersCounter = 0;

	virtual void Logout(AController * Exiting) override;

	TArray<EColorSchemeEnum> AvailableColors = 
	{ 
		EColorSchemeEnum::CSE_Brown, 
		EColorSchemeEnum::CSE_Green, 
		EColorSchemeEnum::CSE_Purple, 
		EColorSchemeEnum::CSE_Red,
		EColorSchemeEnum::CSE_Pink,
		EColorSchemeEnum::CSE_Blue
	};

};
