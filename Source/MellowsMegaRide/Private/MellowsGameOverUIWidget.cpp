// Fill out your copyright notice in the Description page of Project Settings.


#include "MellowsGameOverUIWidget.h"
#include "Components/TextBlock.h"
#include "MellowsPlayerState.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h" 
#include "MellowsScoreboardEntryUIWidget.h"
#include "MellowsGameOverScoreUIWidget.h"
#include "MellowsGameState.h"
#include "Algo/Sort.h" 

UMellowsGameOverUIWidget::UMellowsGameOverUIWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void UMellowsGameOverUIWidget::NativeConstruct()
{
	// Call the Blueprint "Event Construct" node
	Super::NativeConstruct();


	// ItemTitle can be nullptr if we haven't created it in the
	// Blueprint subclass

	//if (ItemTitle)
	//{
	//	ItemTitle->SetText(FText::FromString("Hello world!"));
	//
	//}
	//if (ChargeBar)
	//{
	//	ChargeBar->SetPercent(1.0f);
	//}

	
}

void UMellowsGameOverUIWidget::NativeTick(const FGeometry & MyGeometry, float DeltaSeconds)
{
	Super::NativeTick(MyGeometry, DeltaSeconds);
}

void UMellowsGameOverUIWidget::setScoreBoard(bool verdict) {
	// get gamestate
	AMellowsGameState* GameState = Cast<AMellowsGameState>(GetWorld()->GetGameState());

	// add the playerstate references to our array
	 // get list of player from gamestate
	if (GameState)
	{


		// SET PLAYER MESSAGE
		if (verdict)
		{
			GameEndText->SetText(FText::FromString("YOU WON"));
		}
		else {
			GameEndText->SetText(FText::FromString("YOU LOST"));

		}


		int32 i = 0;


		//for (auto& Player : GameState->PlayerArray)
		//{
		//	AMellowsPlayerState* PlayerState = Cast<AMellowsPlayerState>(Player);
		//	sortedPlayerList.AddUnique(PlayerState);
			// i++;
		//}

		// sort the playerState array by score
		sortedPlayerList.StableSort([](AMellowsPlayerState& A, AMellowsPlayerState& B) {
			return A.GetScore() > B.GetScore();
		});

		AMellowsPlayerState* myPlayerstate = Cast<AMellowsPlayerState>(GetOwningPlayerState());
		//
		for (auto& PlayerSorted : sortedPlayerList)
		{
			//AMellowsPlayerState* PlayerState = Cast<AMellowsPlayerState>(PlayerSorted);

			//AMellowsPawn* currentPawn = Cast<AMellowsPawn>(PlayerSorted->GetPawn());
			if (PlayerSorted)
			{
				//PlayerScores += PlayerSorted->GetPlayerName() + " : " + FString::FromInt(PlayerSorted->GetScore()) + "\n";

				// SET PLAYER NAME
				playerVerdictBoardEntries[i]->Playername->SetText(FText::FromString(PlayerSorted->GetPlayerName()));

				// SET PLAYER SCORE
				playerVerdictBoardEntries[i]->Playerscore->SetText(FText::FromString(FString::FromInt(PlayerSorted->GetScore())));

				playerVerdictBoardEntries[i]->PlayerrankText->SetText(FText::FromString(FString::FromInt(i+1)));
				// SET PLAYER IMAGE
				switch (i)
				{
				case 0:
					playerVerdictBoardEntries[i]->PlayerrankIcon->SetBrushTintColor(FLinearColor(1.f, 0.8f, 0.15f));
					break;
				case 1:
					playerVerdictBoardEntries[i]->PlayerrankIcon->SetBrushTintColor(FLinearColor(0.3f, 0.6f, 0.5f));
					break;
				case 2:
					playerVerdictBoardEntries[i]->PlayerrankIcon->SetBrushTintColor(FLinearColor(1.f, 0.7f, 0.5f));
					break;
				default:
					playerVerdictBoardEntries[i]->PlayerrankIcon->SetVisibility(ESlateVisibility::Hidden);

				}

				if (myPlayerstate == PlayerSorted)
				{
					playerVerdictBoardEntries[i]->Playername->SetColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.3f));
				}
				else
				{
					playerVerdictBoardEntries[i]->Playername->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f));

				}

				// SET PLAYERNAMES COLORS
				switch (PlayerSorted->ColorScheme)
				{
				case EColorSchemeEnum::CSE_Brown:
					//
					playerVerdictBoardEntries[i]->PlayerColorIcon->SetBrushTintColor(FLinearColor(1.f, 0.8f, 0.21f));
					break;
				case EColorSchemeEnum::CSE_Green:
					playerVerdictBoardEntries[i]->PlayerColorIcon->SetBrushTintColor(FLinearColor(0.5f, 1.f, 0.35f));
					break;
				case EColorSchemeEnum::CSE_Purple:
					playerVerdictBoardEntries[i]->PlayerColorIcon->SetBrushTintColor(FLinearColor(0.5f, 0.28f, 1.0f));
					break;
				case EColorSchemeEnum::CSE_Red:
					playerVerdictBoardEntries[i]->PlayerColorIcon->SetBrushTintColor(FLinearColor(1.0f, 0.21f, 0.21f));
					break;
				case EColorSchemeEnum::CSE_Pink:
					playerVerdictBoardEntries[i]->PlayerColorIcon->SetBrushTintColor(FLinearColor(1.0f, 0.5f, 1.0f));
					break;
				case EColorSchemeEnum::CSE_Blue:
					playerVerdictBoardEntries[i]->PlayerColorIcon->SetBrushTintColor(FLinearColor(0.1f, 0.5f, 1.0f));
					break;
				}


				//playerList[i]->SetText(FText::FromString(PlayerSorted->GetPlayerName() + " : " + FString::FromInt(PlayerSorted->GetScore())));
			
				//if (myPlayerstate == PlayerSorted)
				//{
				//	playerList[i]->SetText(FText::FromString(PlayerSorted->GetPlayerName() + " : " + FString::FromInt(PlayerSorted->GetScore())));

				//}
				/*
				switch (PlayerSorted->ColorScheme)
				{
				case EColorSchemeEnum::CSE_Brown:
					//
					playerList[i]->SetColorAndOpacity(FLinearColor(1.f, 0.8f, 0.21f));
					break;
				case EColorSchemeEnum::CSE_Green:
					playerList[i]->SetColorAndOpacity(FLinearColor(0.5f, 1.f, 0.35f));
					break;
				case EColorSchemeEnum::CSE_Purple:
					playerList[i]->SetColorAndOpacity(FLinearColor(0.5f, 0.28f, 1.0f));
					break;
				case EColorSchemeEnum::CSE_Red:
					playerList[i]->SetColorAndOpacity(FLinearColor(1.0f, 0.21f, 0.21f));
				case EColorSchemeEnum::CSE_Pink:
					playerList[i]->SetColorAndOpacity(FLinearColor(1.0f, 0.5f, 1.0f));
					break;
				case EColorSchemeEnum::CSE_Blue:
					playerList[i]->SetColorAndOpacity(FLinearColor(0.1f, 0.5f, 1.0f));
					break;
				}
				*/
			}
			i++;


		}
	}
}
