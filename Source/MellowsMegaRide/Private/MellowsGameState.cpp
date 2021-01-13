// Fill out your copyright notice in the Description page of Project Settings.


#include "MellowsGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"
#include "MellowsGameMode.h"
#include "Algo/Sort.h" 
#include "MellowsPlayerState.h"
#include "MellowsPlayerController.h"
#include "FollowerSpawner.h"
#include "MellowsPawn.h"
#include "MelllowsGameHUD.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"

void AMellowsGameState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetNetMode() == NM_DedicatedServer)
		RemainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(ServerMatchTimerHandle);
	else
		RemainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(ClientMatchTimerHandle);
}

void AMellowsGameState::PlayerIsReady(AMellowsPlayerController* PlayerController)
{	
	if (!JoinedPlayers.Contains(PlayerController))
	{
		JoinedPlayers.Push(PlayerController);

		//AMellowsPlayerState* newPlayerEntry = Cast<AMellowsPlayerState>(PlayerController->PlayerState);

		// ADD TO JOINEDPLAYER SSTATES
		//if (newPlayerEntry)
		//{
			//JoinedPlayersStates.Push(newPlayerEntry);

			Multicast_AddPlayerboardEntry();

			// UPDATE ALL CLIENTS PLAYER 
			
		//}
	}

	UE_LOG(LogTemp, Warning, TEXT("Player %s is ready. What a cool dude/dudess"), *PlayerController->PlayerState->GetPlayerName());
	if (JoinedPlayers.Num() == PlayerArray.Num() && !bGameStarted)
	{
		GetWorld()->GetTimerManager().SetTimer(SphereSpawnerHandle, this, &AMellowsGameState::CallCountDown, CountDownTimeStep, false);
		bGameStarted = true;
	}
	// if game already started
	else if (bGameStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player %s joined late. Such an uncool dude/dudess"), *PlayerController->PlayerState->GetPlayerName());

		AMellowsPawn* MellowsPawn = Cast<AMellowsPawn>(PlayerController->GetPawn());
		if (MellowsPawn)
		{
			MellowsPawn->SetStopped(false);
			Multicast_StartCountGameTimeLateJoiner(PlayerController->PlayerState->PlayerId, RemainingTime);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Wtf: Game started is %d"), bGameStarted);
	}
}

//void AMellowsGameState::sortJoinedPlayersStates()
//{
//	JoinedPlayersStates.StableSort([](AMellowsPlayerState& A, AMellowsPlayerState& B) {
//		return A.GetScore() > B.GetScore();
//	});
//}

//void AMellowsGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//	DOREPLIFETIME(AMellowsGameState, JoinedPlayersStates);
//
//}

//void AMellowsGameState::OnRep_PlayerBoardChanged()
//{
//	//sortJoinedPlayersStates();
//	JoinedPlayersStates.StableSort([](AMellowsPlayerState& A, AMellowsPlayerState& B) {
//		return A.GetScore() > B.GetScore();
//	});
//}

void AMellowsGameState::Multicast_AddPlayerboardEntry_Implementation()
{

	APlayerController* Player = GetWorld()->GetFirstPlayerController();
	if (Player)
	{
		AMelllowsGameHUD* PlayerHud = Cast<AMelllowsGameHUD>(Player->GetHUD());
		if (PlayerHud)
		{


			//PlayerHud->addPlayerEntryUIWidget(newPlayerEntry);
			//PlayerHud->addInGamePlayerInfoUIWidget(newPlayerEntry);
			
			// RECREATE SCOREBOARD FOR EACH PLAYER
			PlayerHud->reCreateScoreBoard();
			PlayerHud->updateScoreBoard();
		}
	}
}


void AMellowsGameState::Multicast_StartCountGametime_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("In Start counting!"));

	if (!bInfiniteGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("Start counting!"));
		if (GetNetMode() == NM_DedicatedServer)
		{
			GetWorld()->GetTimerManager().SetTimer(ServerMatchTimerHandle, this, &AMellowsGameState::OnGameOver, MatchTime, false);
			GetWorld()->GetTimerManager().SetTimer(SphereSpawnerHandle, this, &AMellowsGameState::OnRespawnSpheres, TimeTillNextRespawn, true);
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(ClientMatchTimerHandle, nullptr, MatchTime, false);

			// HIDE COUNTDOWN 
			APlayerController* Player = GetWorld()->GetFirstPlayerController();
			if (Player)
			{
				AMelllowsGameHUD* PlayerHud = Cast<AMelllowsGameHUD>(Player->GetHUD());
				if (PlayerHud)
				{
					//PlayerHud->MellowsUIWidget->GameEndText->SetVisibility(ESlateVisibility::Hidden);
					//PlayerHud->MellowsUIWidget->JoinGameText->SetText(FText::FromString();
					PlayerHud->MellowsUIWidget->JoinGameText->SetVisibility(ESlateVisibility::Hidden);
					PlayerHud->MellowsUIWidget->RemainingTimeText->SetVisibility(ESlateVisibility::Visible);


				}
			}
		}

		for (FConstPlayerControllerIterator itr = GetWorld()->GetPlayerControllerIterator(); itr; itr++)
		{
			AMellowsPawn* pawn = Cast<AMellowsPawn>(itr->Get()->GetPawn());
			if (pawn)
			{
				pawn->SetStopped(false);
			}
		}
	}
}


void AMellowsGameState::Multicast_StartCountDown_Implementation(uint32 Remaining)
{
	//if (GEngine)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Countdown: %d"), Remaining));
	//}
	if (GetNetMode() == NM_DedicatedServer)
	{
		if (Remaining > 0)
		{
			GetWorld()->GetTimerManager().SetTimer(SphereSpawnerHandle, this, &AMellowsGameState::CallCountDown, CountDownTimeStep, false);
			--CountDown;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Countdown is over"));
			Multicast_StartCountGametime();
		}
	}
	else {
		// UPDATE UI
		// TODO:	MAKE AND USE NEW UI TEXTBLOCK ELEMENT
		APlayerController* Player = GetWorld()->GetFirstPlayerController();
		if (Player)
		{
			AMelllowsGameHUD* PlayerHud = Cast<AMelllowsGameHUD>(Player->GetHUD());
			if (PlayerHud)
			{
				//PlayerHud->MellowsUIWidget->GameEndText->SetText(FText::AsNumber(Remaining));
				//PlayerHud->MellowsUIWidget->GameEndText->SetVisibility(ESlateVisibility::Visible);
				PlayerHud->MellowsUIWidget->JoinGameText->SetText(FText::AsNumber(Remaining));
				//PlayerHud->MellowsUIWidget->JoinGameText->SetVisibility(ESlateVisibility::Visible);

			}
		}
		// 
	}
}


void AMellowsGameState::Multicast_StartCountGameTimeLateJoiner_Implementation(uint32 PlayerId, int Remaining)
{
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("You are late.")));

	if (GetNetMode() == NM_Client)
	{
		APlayerController* PController = GetWorld()->GetFirstPlayerController();
		if (PController)
		{
			APlayerState* state = PController->PlayerState;
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("There is a player controller")));
			if (state->IsValidLowLevel() && (state->PlayerId == PlayerId))
			{
				//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Timer is set")));
				GetWorld()->GetTimerManager().SetTimer(ClientMatchTimerHandle, nullptr, Remaining, false);
				AMelllowsGameHUD* PlayerHud = Cast<AMelllowsGameHUD>(PController->GetHUD());
				if (PlayerHud)
				{
					//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Found hud")));
					PlayerHud->MellowsUIWidget->JoinGameText->SetVisibility(ESlateVisibility::Hidden);
					PlayerHud->MellowsUIWidget->RemainingTimeText->SetVisibility(ESlateVisibility::Visible);
				}
			}
		}
	}
}

//void AMellowsGameState::Multicast_UpdatePlayerboardEntries_Implementation()
//{
//	// FOR EACH CLIENT
//	APlayerController* Player = GetWorld()->GetFirstPlayerController();
//
//	// GET HUD
//	AMelllowsGameHUD* PlayerHud = Cast<AMelllowsGameHUD>(Player->GetHUD());
//
//	if (PlayerHud)
//	{
//		// GET PLAYERSTATES
//		//PlayerHud->addPlayerEntryUIWidget(newPlayerEntry);
//		//PlayerHud->addInGamePlayerInfoUIWidget(newPlayerEntry);
//		PlayerHud->updatePlayerEntryUIWidgets();
//	}
//}

//void AMellowsGameState::Multicast_UpdatePlayerboardScores_Implementation()
//{
//	sortJoinedPlayersStates();
//}


void AMellowsGameState::OnGameOver()
{
	UE_LOG(LogTemp, Warning, TEXT("Game is Over!"));

	FindWinner();
	GetWorld()->GetTimerManager().SetTimer(RestartGameTimerHandle, this, &AMellowsGameState::RestartGame, TimeTillRestart, false);
}

void AMellowsGameState::FindWinner()
{
	TArray<AMellowsPlayerState*> Winners;
	Winners.Push(Cast<AMellowsPlayerState>(PlayerArray[0]));

	for (int i = 1; i < PlayerArray.Num(); ++i)
	{
		AMellowsPlayerState* CurrentPlayerState = Cast<AMellowsPlayerState>(PlayerArray[i]);

		UE_LOG(LogTemp, Warning, TEXT("Current Player ID: %d Points: %d"), CurrentPlayerState->PlayerId, CurrentPlayerState->GetScore());
		UE_LOG(LogTemp, Warning, TEXT("Current winner Points: %d"), Winners[0]->GetScore());

		if (CurrentPlayerState->GetScore() > Winners[0]->GetScore())
		{
			for (auto& PlayerState : Winners)
			{
				PlayerState->Client_MatchResults(EVictoryStatusEnum::VE_Loser);
			}
			Winners = TArray<AMellowsPlayerState*>();
			UE_LOG(LogTemp, Warning, TEXT("Should be 0: "), Winners.Num());
			Winners.Push(CurrentPlayerState);
		}
		else if (CurrentPlayerState->GetScore() == Winners[0]->GetScore())
		{
			UE_LOG(LogTemp, Warning, TEXT("Equals"));
			Winners.Push(CurrentPlayerState);
		}
		else
		{
			CurrentPlayerState->Client_MatchResults(EVictoryStatusEnum::VE_Loser);
			UE_LOG(LogTemp, Warning, TEXT("Looser"));
		}
	}

	for (auto& PlayerState : Winners)
	{
		PlayerState->Client_MatchResults(EVictoryStatusEnum::VE_Winner);
	}
}

void AMellowsGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	if (GetNetMode() == NM_DedicatedServer)
	{
		GetAllSphereSpawners();
		SelectStartingSpawners();
	}
}

void AMellowsGameState::GetAllSphereSpawners()
{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFollowerSpawner::StaticClass(), FoundActors);
		AFollowerSpawner* Spawner = nullptr;
		for (int32 i = 0; i < FoundActors.Num(); ++i)
		{
			Spawner = Cast<AFollowerSpawner>(FoundActors[i]);
			check(Spawner != nullptr && "One of the spawners is somehow a null pointer");
			Spawner->index = i;
			FollowerSpawnerList.Add(Spawner);
		}

		for (int i = 0; i < FollowerSpawnerList.Num(); i++)
		{
			UE_LOG(LogTemp, Warning, TEXT("Test Spawner with index %d: "), FollowerSpawnerList[i]->index);
		}

		if (FollowerSpawnerList.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("No Spawners placed in Level!!!"));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("%d Spawners placed in Level."), FollowerSpawnerList.Num());
		}
}

void AMellowsGameState::SelectStartingSpawners()
{
	int TotalSpawners = FollowerSpawnerList.Num();

	if (StartingSpheresNum > TotalSpawners)
	{
		UE_LOG(LogTemp, Warning, TEXT("Too much StartingSpheres set. StartingSpheres = %d Spawners in level = %d"), StartingSpheresNum, TotalSpawners);
		StartingSpheresNum = TotalSpawners;
	}

	TArray<int> RandomNumbers;
	for (int i = 0; i < StartingSpheresNum; ++i)
	{
		int randInt = FMath::RandRange(0, TotalSpawners - 1);
		while (RandomNumbers.Contains(randInt))
		{
			randInt = FMath::RandRange(0, TotalSpawners - 1);
		}
		RandomNumbers.Add(randInt);
	}

	for (auto& i : RandomNumbers)
	{
		FollowerSpawnerList[i]->SpawnSphere();
	}
}

void AMellowsGameState::OnRespawnSpheres()
{
	TArray<int> AvailableSpawnersIndices;
	for (int i = 0; i < FollowerSpawnerList.Num(); i++)
	{
		if (!FollowerSpawnerList[i]->bIsOccupied)
		{
			AvailableSpawnersIndices.Add(i);
		}
	}

	int PossibleSpawingSpheres = RespawningSpheresNum;

	if (RespawningSpheresNum > AvailableSpawnersIndices.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Too much Spheres requested. Requested = %d Available Spawners in level = %d. Total num spawner: %d"), RespawningSpheresNum, AvailableSpawnersIndices.Num(), FollowerSpawnerList.Num());
		PossibleSpawingSpheres = AvailableSpawnersIndices.Num();
	}

	TArray<int> RandomNumbers;
	for (int i = 0; i < PossibleSpawingSpheres; i++)
	{
		int randInt = FMath::RandRange(0, AvailableSpawnersIndices.Num() - 1);
		while (RandomNumbers.Contains(randInt))
		{
			randInt = FMath::RandRange(0, AvailableSpawnersIndices.Num() - 1);
		}

		RandomNumbers.Add(randInt);
		UE_LOG(LogTemp, Log, TEXT("Spawn with index: %d"), AvailableSpawnersIndices[randInt]);
		FollowerSpawnerList[AvailableSpawnersIndices[randInt]]->SpawnSphere();
	}
}

void AMellowsGameState::CallCountDown()
{
	Multicast_StartCountDown(CountDown);
}

void AMellowsGameState::RestartGame()
{
	AMellowsGameMode* GameMode = Cast<AMellowsGameMode>(GetWorld()->GetAuthGameMode());
	GameMode->RestartGame();
}

float AMellowsGameState::GetRemainingTime()
{
	return RemainingTime;
}

