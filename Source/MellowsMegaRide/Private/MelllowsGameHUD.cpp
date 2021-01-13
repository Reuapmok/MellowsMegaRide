// Fill out your copyright notice in the Description page of Project Settings.


#include "MelllowsGameHUD.h"

#include "Blueprint/UserWidget.h"



AMelllowsGameHUD::AMelllowsGameHUD()
{

}

void AMelllowsGameHUD::DrawHUD()
{
	Super::DrawHUD();
};

void AMelllowsGameHUD::BeginPlay()
{
	Super::BeginPlay();

	//if (MellowsGameOverUIWidgetClass)
	//{
	//	MellowsGameOverUIWidget = CreateWidget<UMellowsGameOverUIWidget>(GetWorld(), MellowsGameOverUIWidgetClass);
	//}

	// was widget created
	if (MellowsUIWidgetClass)
	{
		MellowsUIWidget = CreateWidget<UMellowsUIWidget>(GetWorld(), MellowsUIWidgetClass);

		// make sure Widget was created
		if (MellowsUIWidget)
		{
			// Add it to the Viewport
			MellowsUIWidget->AddToViewport();
			UE_LOG(LogTemp, Log, TEXT("Your message"));
		}
	}
};

void AMelllowsGameHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}


void AMelllowsGameHUD::createGameEndWidget()
{
	if (MellowsGameOverUIWidgetClass)
	{
		MellowsGameOverUIWidget = CreateWidget<UMellowsGameOverUIWidget>(GetWorld(), MellowsGameOverUIWidgetClass);

		AMellowsGameState* GameState = Cast<AMellowsGameState>(GetWorld()->GetGameState());

		// add the playerstate references to our array
		 // get list of player from gamestate
		if (GameState)
		{


			for (auto& Player : GameState->PlayerArray)
			{
				if (MellowsGameOverPlayerEntryUIWidgetClass)
				{
					// ADD PLAYER STATE TO LIST
					AMellowsPlayerState* PlayerState = Cast<AMellowsPlayerState>(Player);
					MellowsGameOverUIWidget->sortedPlayerList.AddUnique(PlayerState);
					// i++;

					// create the widget
					UMellowsGameOverScoreUIWidget* PlayerEntry = CreateWidget<UMellowsGameOverScoreUIWidget>(GetWorld(), MellowsGameOverPlayerEntryUIWidgetClass);
					// ADD WIDGET TO VERTICAL BOX
					MellowsGameOverUIWidget->PlayerVerdictBoard->AddChild(PlayerEntry);
					// ADD WIDGET REFEREMCE TO ARRAY
					MellowsGameOverUIWidget->playerVerdictBoardEntries.AddUnique(PlayerEntry);
					// CREATE WIDGET FOR EACH PLAYERSTATE
				}
			}
		}
	}


	MellowsUIWidget->RemoveFromViewport();
	MellowsGameOverUIWidget->AddToViewport();


}
void AMelllowsGameHUD::addMellowsUIWidget()
{
	//MellowsUIWidget = CreateWidget<UMellowsUIWidget>(GetWorld(), MellowsUIWidgetClass);
	MellowsUIWidget->AddToViewport();
}
void AMelllowsGameHUD::addPlayerEntryUIWidget(AMellowsPlayerState* newPlayerEntry)
{
	if (MellowsPlayerEntryUIWidgetClass)
	{
		// create the widget
		UMellowsScoreboardEntryUIWidget* PlayerEntry = CreateWidget<UMellowsScoreboardEntryUIWidget>(GetWorld(), MellowsPlayerEntryUIWidgetClass);
		MellowsUIWidget->Playerboard->AddChild(PlayerEntry);
		MellowsUIWidget->playerBoardEntries.AddUnique(PlayerEntry);
		MellowsUIWidget->sortedPlayerList.AddUnique(newPlayerEntry);
	}
}
void AMelllowsGameHUD::addInGamePlayerInfoUIWidget(AMellowsPlayerState * newPlayerEntry)
{
	AMellowsPlayerController* ownPlayerController = Cast<AMellowsPlayerController>(GetOwningPlayerController());
	if (ownPlayerController)
	{
		AMellowsPlayerState* ownPlayerState = Cast<AMellowsPlayerState>(ownPlayerController->PlayerState);

		// ADD WIDGET ONLY IF ITS THE OTHER CLIENTS PLAYERINFO
		if (MellowsInGamePlayerInfoUIWidgetClass && ownPlayerState != newPlayerEntry)
		{
			// create the widget
			UMellowsInGamePlayerInfoUIWidget* PlayerInfo = CreateWidget<UMellowsInGamePlayerInfoUIWidget>(GetWorld(), MellowsInGamePlayerInfoUIWidgetClass);
			//MellowsUIWidget->get(PlayerInfo);

			//PlayerInfo->SetDesiredSizeInViewport(FVector2D(365.f, 40.f));

			UPanelSlot* Slot = MellowsUIWidget->CanvasPanel_358->AddChild(PlayerInfo);//PlayerInfo->AddToViewport();
			if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
			{

				//MellowsUIWidget->CanvasPanel_358->AddChild(PlayerInfo);
				//MellowsUIWidget->Playerboard->AddChild(PlayerInfo);
				CanvasSlot->SetAutoSize(true);

				MellowsUIWidget->playerNamesInGame.AddUnique(PlayerInfo);
				//MellowsUIWidget->sortedPlayerList.AddUnique(newPlayerEntry);
			}
		}
	}

}
void AMelllowsGameHUD::updatePlayerEntryUIWidgets()
{
	// DELETE ALL CHILDREN FROM Playerboard
	MellowsUIWidget->Playerboard->ClearChildren();

	// FOR EACH JOINEDPLAYERSSTATE 
	// ADD A WIDGET ENTRY
	AMellowsGameState* GameState = Cast<AMellowsGameState>(GetWorld()->GetGameState());


	//for (auto& PlayerSorted : GameState->JoinedPlayersStates)//sortedPlayerList)
	//{
	//	if (MellowsPlayerEntryUIWidgetClass)
	//	{
	//		// create the widget
	//		UMellowsScoreboardEntryUIWidget* PlayerEntry = CreateWidget<UMellowsScoreboardEntryUIWidget>(GetWorld(), MellowsPlayerEntryUIWidgetClass);
	//		MellowsUIWidget->Playerboard->AddChild(PlayerEntry);
	//		MellowsUIWidget->playerBoardEntries.AddUnique(PlayerEntry);
	//		//MellowsUIWidget->sortedPlayerList.AddUnique(newPlayerEntry);
	//	}
	//}

}

void AMelllowsGameHUD::updateScoreBoard()
{
	// GET THE PLAYERSTATE LIST
	AMellowsGameState* GameState = Cast<AMellowsGameState>(GetWorld()->GetGameState());
	AMellowsPawn* ownPlayerPawn = Cast<AMellowsPawn>(GetOwningPawn());

	if (MellowsUIWidget)
	{
		if (GameState && ownPlayerPawn)
		{
			int32 i = 0;
			int32 j = 0;

			// GET TEMPORARY COPY OF PLAYERSTATE LIST
			TArray<AMellowsPlayerState*> TempPlayerStates;


			// COPY PLAYERSTATES TO TEMPORARY ARRAY
			for (auto& PlayerSorted : GameState->PlayerArray)
			{
				AMellowsPlayerState* currentPlayerState = Cast<AMellowsPlayerState>(PlayerSorted);
				TempPlayerStates.Push(currentPlayerState);
			}


			// SORT TEMPORARY COPY
			TempPlayerStates.StableSort([](AMellowsPlayerState& A, AMellowsPlayerState& B)
			{
				return A.GetScore() > B.GetScore();
			});

			// FOR EACH PLAYERSTATE
			for (auto& PlayerSorted : TempPlayerStates)//sortedPlayerList)
			{
				//AMellowsPlayerState* currentPlayerState = Cast<AMellowsPlayerState>(PlayerSorted);

				if (PlayerSorted->IsValidLowLevel())
				{
					AMellowsPawn* currentPawn = Cast<AMellowsPawn>(PlayerSorted->GetPawn());
					if (currentPawn)
					{
						//PlayerScores += PlayerSorted->GetPlayerName() + " : " + FString::FromInt(PlayerSorted->GetScore()) + "\n";
												 // THE SCOREBOARD ENTRIES
												 //
						if (i < MellowsUIWidget->playerBoardEntries.Num())
						{
							MellowsUIWidget->playerBoardEntries[i]->Playername->SetText(FText::FromString(PlayerSorted->GetPlayerName()));
							MellowsUIWidget->playerBoardEntries[i]->Playerscore->SetText(FText::AsNumber(PlayerSorted->GetScore()));
							if (ownPlayerPawn == currentPawn)
							{
								//playerList[i]->SetText(FText::FromString(PlayerState->GetPlayerName() + " : " + FString::FromInt(PlayerState->GetScore())));
								MellowsUIWidget->playerBoardEntries[i]->Playername->SetColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.3f));
								MellowsUIWidget->playerBoardEntries[i]->Playericon->SetVisibility(ESlateVisibility::Visible);
							}
							else
							{
								MellowsUIWidget->playerBoardEntries[i]->Playername->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f));
								MellowsUIWidget->playerBoardEntries[i]->Playericon->SetVisibility(ESlateVisibility::Hidden);

							}

							switch (PlayerSorted->ColorScheme)
							{
							case EColorSchemeEnum::CSE_Brown:
								//
								MellowsUIWidget->playerBoardEntries[i]->Playercolor->SetBrushTintColor(FLinearColor(1.f, 0.8f, 0.21f));
								break;
							case EColorSchemeEnum::CSE_Green:
								MellowsUIWidget->playerBoardEntries[i]->Playercolor->SetBrushTintColor(FLinearColor(0.5f, 1.f, 0.35f));
								break;
							case EColorSchemeEnum::CSE_Purple:
								MellowsUIWidget->playerBoardEntries[i]->Playercolor->SetBrushTintColor(FLinearColor(0.5f, 0.28f, 1.0f));
								break;
							case EColorSchemeEnum::CSE_Red:
								MellowsUIWidget->playerBoardEntries[i]->Playercolor->SetBrushTintColor(FLinearColor(1.0f, 0.21f, 0.21f));
								break;
							case EColorSchemeEnum::CSE_Pink:
								MellowsUIWidget->playerBoardEntries[i]->Playercolor->SetBrushTintColor(FLinearColor(1.0f, 0.5f, 1.0f));
								break;
							case EColorSchemeEnum::CSE_Blue:
								MellowsUIWidget->playerBoardEntries[i]->Playercolor->SetBrushTintColor(FLinearColor(0.1f, 0.5f, 1.0f));
								break;

							}

						}

						// ITERATE OVER INGAME PLAYERINFO
						//

					}
				}
				i++;

			}
		}
	}

}
void AMelllowsGameHUD::reCreateScoreBoard()
{

	// DELETES ALL CHILDS OF SCOREBOARD AND CREATES NEW ONES FOR EVERY PLAYERSTATE
	AMellowsGameState* GameState = Cast<AMellowsGameState>(GetWorld()->GetGameState());

	if (MellowsUIWidget)
	{
		MellowsUIWidget->Playerboard->ClearChildren();
		MellowsUIWidget->playerBoardEntries.Empty();

		for (auto& PlayerSorted : GameState->PlayerArray)
		{
			if (MellowsPlayerEntryUIWidgetClass)
			{
				UMellowsScoreboardEntryUIWidget* PlayerEntry = CreateWidget<UMellowsScoreboardEntryUIWidget>(GetWorld(), MellowsPlayerEntryUIWidgetClass);
				MellowsUIWidget->Playerboard->AddChild(PlayerEntry);
				MellowsUIWidget->playerBoardEntries.AddUnique(PlayerEntry);
			}
		}


		// DELETE ALL INGAMEINFO WIDGETS FROM CANVAS
		for (auto& InGameInfoWidget : MellowsUIWidget->playerNamesInGame)
		{
			MellowsUIWidget->CanvasPanel_358->RemoveChild(InGameInfoWidget);
		}

		// DELETE THE INGAMEINFOWIDGET ARRAY REFERENCES
		MellowsUIWidget->playerNamesInGame.Empty();
	}





	AMellowsPlayerController* ownPlayerController = Cast<AMellowsPlayerController>(GetOwningPlayerController());

	// FOR EACH PLAYERSTATE THAT IS NOT OUR OWN ADD A INGAMEINFOWIDGET
	if (ownPlayerController)
	{
		AMellowsPlayerState* ownPlayerState = Cast<AMellowsPlayerState>(ownPlayerController->PlayerState);
		if (GameState)
		{
			for (auto& PlayerSorted : GameState->PlayerArray)
			{
				AMellowsPlayerState* currentPlayerState = Cast<AMellowsPlayerState>(PlayerSorted);

				if (MellowsInGamePlayerInfoUIWidgetClass && ownPlayerState != currentPlayerState)
				{
					// create the widget
					UMellowsInGamePlayerInfoUIWidget* PlayerInfo = CreateWidget<UMellowsInGamePlayerInfoUIWidget>(GetWorld(), MellowsInGamePlayerInfoUIWidgetClass);


					UPanelSlot* Slot = MellowsUIWidget->CanvasPanel_358->AddChild(PlayerInfo);
					if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
					{
						CanvasSlot->SetAutoSize(true);
						MellowsUIWidget->playerNamesInGame.AddUnique(PlayerInfo);
					}
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("GameState is null"));
		}

	}

}
;

