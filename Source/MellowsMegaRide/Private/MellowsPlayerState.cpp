// Fill out your copyright notice in the Description page of Project Settings.


#include "MellowsPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "MelllowsGameHUD.h"

void AMellowsPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMellowsPlayerState, ColorScheme);
	DOREPLIFETIME(AMellowsPlayerState, FollowingActorsCount);
}

void AMellowsPlayerState::Destroyed()
{
	Super::Destroyed();

	AMellowsGameState* GameState = Cast<AMellowsGameState>(GetWorld()->GetGameState());
	if (GameState)
	{
		GameState->Multicast_AddPlayerboardEntry();
		UE_LOG(LogTemp, Warning, TEXT("WE SHOULD BE UPDATING THE PLAYERLIST"));

	}
}

void AMellowsPlayerState::Multicast_AddScore_Implementation(int rewardedPoints)
{
		Points += rewardedPoints;
		UE_LOG(LogTemp, Warning, TEXT("Added points to ID: %d now has %d Points."), PlayerId, Points);

		// UPDATE OWN HUD

		// GET PLAYER CONTROLLER
		APlayerController* Player = GetWorld()->GetFirstPlayerController();

		if (Player)
		{
			// GET THE HUD
			AMelllowsGameHUD* PlayerHud = Cast<AMelllowsGameHUD>(Player->GetHUD());

			if (PlayerHud)
			{
				PlayerHud->reCreateScoreBoard();
				PlayerHud->updateScoreBoard();
			}

		}

}

int AMellowsPlayerState::GetScore()
{
	return Points;
}

void AMellowsPlayerState::SetFollowingActorsCount(int followingActors) 
{
	FollowingActorsCount = followingActors;
	Multicast_SetFollowingActorsCount(followingActors);
	UE_LOG(LogTemp, Warning, TEXT("Set FollowingActors to ID: %d now has %d followingActors."), PlayerId, FollowingActorsCount);
}

void AMellowsPlayerState::Multicast_SetFollowingActorsCount_Implementation(int followingActors)
{
	FollowingActorsCount = followingActors;
}


void AMellowsPlayerState::AddFollowingActorsCount(int addedfollowingActors)
{
	FollowingActorsCount += addedfollowingActors;
	UE_LOG(LogTemp, Warning, TEXT("Added FollowingActors to ID: %d added %d followingActors."), PlayerId, addedfollowingActors);

}

void AMellowsPlayerState::Client_MatchResults_Implementation(EVictoryStatusEnum MatchResult)
{
	APlayerController* Player = GetWorld()->GetFirstPlayerController();
	
	int clientPlayerId = Player->PlayerState->PlayerId;

	UE_LOG(LogTemp, Warning, TEXT("Hiho Player ID: %d"), Player->PlayerState->PlayerId);

	if (this->PlayerId == clientPlayerId)
	{
		if (MatchResult == EVictoryStatusEnum::VE_Winner)
		{
			if (GEngine)
			{
				AMelllowsGameHUD* PlayerHud = Cast<AMelllowsGameHUD>(Player->GetHUD());
				if (PlayerHud)
				{
					//PlayerHud->MellowsUIWidget->GameEndText->SetText(FText::FromString(FString("You won! Points: ") + FString::FromInt(this->Points)));
					//PlayerHud->MellowsUIWidget->GameEndText->SetVisibility(ESlateVisibility::Visible);

					//PlayerHud->MellowsUIWidget->RemoveFromParent();
					PlayerHud->createGameEndWidget();
					PlayerHud->MellowsGameOverUIWidget->setScoreBoard(true);

					//PlayerHud->MellowsGameOverUIWidget->AddToViewport();
					//PlayerHud->MellowsGameOverUIWidget->GameEndText->SetText(FText::FromString(FString("You won!")));// Points: ")) +FString::FromInt(this->Points)));
					//PlayerHud->MellowsGameOverUIWidget->GameEndText->SetVisibility(ESlateVisibility::Visible);
					//PlayerHud->MellowsGameOverUIWidget->setScoreBoard();


				}
			}
		}
		else if (MatchResult == EVictoryStatusEnum::VE_Loser)
		{
			if (GEngine)
			{
				AMelllowsGameHUD* PlayerHud = Cast<AMelllowsGameHUD>(Player->GetHUD());
				if (PlayerHud)
				{
					//PlayerHud->MellowsUIWidget->GameEndText->SetText(FText::FromString(FString("You lost... Points: ") + FString::FromInt(this->Points)));
					//PlayerHud->MellowsUIWidget->GameEndText->SetVisibility(ESlateVisibility::Visible);


					PlayerHud->createGameEndWidget();
					PlayerHud->MellowsGameOverUIWidget->setScoreBoard(false);
					//PlayerHud->MellowsUIWidget->RemoveFromParent();

					// ADD GAME OVER WIDGET TO SCREEN
					//PlayerHud->MellowsGameOverUIWidget->AddToViewport();
					//PlayerHud->MellowsGameOverUIWidget->GameEndText->SetText(FText::FromString(FString("You lost...")));// Points: ") + FString::FromInt(this->Points)));
					//PlayerHud->MellowsGameOverUIWidget->GameEndText->SetVisibility(ESlateVisibility::Visible);
					//PlayerHud->MellowsGameOverUIWidget->setScoreBoard();


				}
				//PlayerHud->MellowsGameOverUIWidget->AddToViewport();
				//PlayerHud->MellowsGameOverUIWidget->setScoreBoard();


			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Victory status is neither winner nor looser and that is a problem."));
		}
	}
}
