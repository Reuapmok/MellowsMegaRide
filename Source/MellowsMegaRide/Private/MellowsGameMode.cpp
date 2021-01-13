// Fill out your copyright notice in the Description page of Project Settings.


#include "MellowsGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "CommandLine.h"
#include "MellowsPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "MellowsPlayerState.h"
#include "Misc/Char.h"
#include "MellowsGameState.h"

#include "GameFramework/Controller.h"

void AMellowsGameMode::OnWaitingTimeOver()
{
	bWaitingTimeOver = true;
}

void AMellowsGameMode::BeginPlay()
{
	Super::BeginPlay();

	uint32 maxplayers;

	ParseCommandlineParam(FCommandLine::Get(), TEXT("maxplayers"), maxplayers);
	if (maxplayers == 0)
		maxplayers = 6;
	MaxNumPlayers = maxplayers;
}

void AMellowsGameMode::PostLogin(APlayerController * NewPlayer)
{
	Super::PostLogin(NewPlayer);


	//AMellowsPlayerController* MellowsController = Cast<AMellowsPlayerController>(NewPlayer);

	//if(MellowsController)
		//MellowsController->Client_AddPlayerboardEntry();
	//AMellowsPlayerController* MellowsController = Cast<AMellowsPlayerController>(NewPlayer);
	//if(MellowsController)
	//	MellowsController->Multicast_AddPlayerboardEntry();

	//AMellowsGameState* MellowsState = Cast<AMellowsGameState>(GetWorld()->GetGameState());

	//if (MellowsState)
	//{
	//	MellowsState->Multicast_AddPlayerboardEntry();
	//}

	APlayerState* PlayerState = NewPlayer->PlayerState;

	// if playername is empty, player gets a generic name consisiting of <"Player"> + <playercount>
	if(PlayerState->GetPlayerName().IsEmpty())
		PlayerState->SetPlayerName(FString("Player").Append(FString::FromInt(JoinedPlayersCounter)));

	UE_LOG(LogTemp, Warning, TEXT("Name is: %s"), *PlayerState->GetPlayerName());

	AMellowsPlayerState* MellowsPlayerState = Cast<AMellowsPlayerState>(PlayerState);


	if (MellowsPlayerState && AvailableColors.Num() > 0)
	{
		MellowsPlayerState->ColorScheme = AvailableColors.Pop();
	}

	++JoinedPlayersCounter;

	GetWorld()->GetTimerManager().SetTimer(WaitingTimeHandle, this, &AMellowsGameMode::OnWaitingTimeOver, WaitingTime, false);
}

bool AMellowsGameMode::ReadyToStartMatch_Implementation()
{
	 //If bDelayed Start is set, wait for a manual match start

	if (bDelayedStart)
	{
		return false;
	}

	// Start when MaxNumPlayers is reached
	if (GetMatchState() == MatchState::WaitingToStart)
	{
		if (NumPlayers + NumBots == MaxNumPlayers || bWaitingTimeOver)
		{
			return true;
		}
	}
	return false;
}

FString AMellowsGameMode::InitNewPlayer(APlayerController * NewPlayerController, const FUniqueNetIdRepl & UniqueId, const FString & Options, const FString & Portal)
{
	FString result = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	UE_LOG(LogTemp, Warning, TEXT("Optionsstring: %s"), *Options);

	FString name = UGameplayStatics::ParseOption(Options, TEXT("playername"));
	FString body = UGameplayStatics::ParseOption(Options, TEXT("BodyColor"));
	FString wing = UGameplayStatics::ParseOption(Options, TEXT("WingColor"));
	FString jet = UGameplayStatics::ParseOption(Options, TEXT("JetColor"));
	FString inner = UGameplayStatics::ParseOption(Options, TEXT("InnerPartColor"));
	FString interior = UGameplayStatics::ParseOption(Options, TEXT("InteriorColor"));
	FString hinge = UGameplayStatics::ParseOption(Options, TEXT("HingeColor"));
	FString plate = UGameplayStatics::ParseOption(Options, TEXT("PlateColor"));
	
	int wingColor;
	int bodyColor;
	int jetColor;
	int innerPartColor;
	int interiorColor;
	int hingeColor;
	int plateColor;

	if (!body.IsEmpty())
		bodyColor = FCString::Atoi(*body);
	else
		bodyColor = 0;

	if (!wing.IsEmpty())
		wingColor = FCString::Atoi(*wing);
	else
		wingColor = 4;

	if (!jet.IsEmpty())
		jetColor = FCString::Atoi(*jet);
	else
		jetColor = 6;

	if (!inner.IsEmpty())
		innerPartColor = FCString::Atoi(*inner);
	else
		innerPartColor = 7;

	if (!interior.IsEmpty())
		interiorColor = FCString::Atoi(*interior);
	else
		interiorColor = 8;

	if (!hinge.IsEmpty())
		hingeColor = FCString::Atoi(*hinge);
	else
		hingeColor = 9;

	if (!plate.IsEmpty())
		plateColor = FCString::Atoi(*plate);
	else
		plateColor = 10;
		
	UE_LOG(LogTemp, Warning, TEXT("New Name is!!!: %s Body: %d, Wings: %d, Jets: %d, Inner: %d, Interior: %d, Hinge: %d, Plate: %d"), 
		*name, bodyColor, wingColor, jetColor, innerPartColor, interiorColor, hingeColor, plateColor);

	NewPlayerController->PlayerState->SetPlayerName(UGameplayStatics::ParseOption(Options, TEXT("playername")));
	AMellowsPlayerController* MellowsPlayerController = Cast<AMellowsPlayerController>(NewPlayerController);
	if (MellowsPlayerController)
	{
		FColorGliderParts colorglider{
			bodyColor,
			wingColor,
			jetColor,
			innerPartColor,
			interiorColor,
			hingeColor,
			plateColor
		};
		MellowsPlayerController->SetGliderColor(colorglider);
	}

	return result;
}

bool AMellowsGameMode::ParseCommandlineParam(const TCHAR * Stream, const TCHAR * Match, uint32 & Value)
{
	const TCHAR* Temp = FCString::Strifind(Stream, Match);
	TCHAR* End;
	if (Temp == NULL)
		return false;
	Value = FCString::Strtoi(Temp + 1 + FCString::Strlen(Match), &End, 10);
	UE_LOG(LogTemp, Warning, TEXT("Maxplayers: %d"), Value);

	return true;
}

void AMellowsGameMode::Logout(AController * Exiting)
{
	Super::Logout(Exiting);

	UE_LOG(LogTemp, Warning, TEXT("Player %s left the game"), *Exiting->PlayerState->GetPlayerName());

	AMellowsGameState* GameState = Cast<AMellowsGameState>(GetWorld()->GetGameState());
	if (GameState)
	{
		//GameState->RemovePlayerState(Exiting->PlayerState);
		int i = 0;
		for (auto& PlayerState : GameState->PlayerArray)
		{
			
			UE_LOG(LogTemp, Warning, TEXT("WE ENTERED THE GAMESTATE %s"), (PlayerState->IsPendingKill() ? TEXT("True") : TEXT("False")));
			i++;
		}
		GameState->Multicast_AddPlayerboardEntry();
	}
}
