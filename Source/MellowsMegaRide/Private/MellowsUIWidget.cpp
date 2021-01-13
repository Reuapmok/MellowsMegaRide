// Fill out your copyright notice in the Description page of Project Settings.


#include "MellowsUIWidget.h"

#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "MellowsPawn.h"
#include "MellowsPlayerState.h"
#include "MellowsGameState.h"
#include "MellowsPlayerController.h"
#include "Components/CanvasPanelSlot.h" 
#include "Components/Border.h" 
#include "Algo/Sort.h" 
#include "MellowsGliderDisplayUIWidget.h"
#include "Materials/MaterialInstanceDynamic.h" 
#include "MellowsScoreboardEntryUIWidget.h"
#include "MellowsInGamePlayerInfoUIWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h" 

#include "Components/CanvasPanel.h" 
#include "Components/VerticalBox.h" 

UMellowsUIWidget::UMellowsUIWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void UMellowsUIWidget::NativeConstruct()
{
	// Call the Blueprint "Event Construct" node
	Super::NativeConstruct();

	StoreWidgetAnimations();

	GotOrbsAnimation = GetAnimationByName(TEXT("GotOrbsAnim"));
	ImpulseReadyAnimation = GetAnimationByName(TEXT("ImpulseReadyAnim"));
	BeeingRobbedAnimation = GetAnimationByName(TEXT("RobbedAnim"));
}

void UMellowsUIWidget::NativeTick(const FGeometry & MyGeometry, float DeltaSeconds)
{
	Super::NativeTick(MyGeometry, DeltaSeconds);


	// should not be done every tick....
	AMellowsPawn* Mellow = Cast<AMellowsPawn>(GetOwningPlayerPawn());

	// ALWAYS check your pointer 
	if (Mellow)
	{

		// get Player State
		AMellowsPlayerState* MellowState = Cast<AMellowsPlayerState>(Mellow->GetPlayerState());

		if (MellowState)
		{
		}


		// get Remaining Time
		AMellowsGameState* GameState = Cast<AMellowsGameState>(GetWorld()->GetGameState());

		if (GameState)
		{
			// SET THE REMAINING TIME
			// should be done every tick i guess

			if ((FMath::FloorToInt(GameState->GetRemainingTime()) % 60) < 10)
				RemainingTimeText->SetText(FText::FromString(FString::FromInt(GameState->GetRemainingTime() / 60.f) + " : 0" + FString::FromInt(FMath::FloorToInt(GameState->GetRemainingTime()) % 60)));
			else
				RemainingTimeText->SetText(FText::FromString(FString::FromInt(GameState->GetRemainingTime() / 60.f) + " : " + FString::FromInt(FMath::FloorToInt(GameState->GetRemainingTime()) % 60)));

			// every tick
			if (GameState->GetRemainingTime() < 30.f)
			{
				float colorChanger = (GameState->GetRemainingTime() * 1.0f / 30.f);
				RemainingTimeText->SetColorAndOpacity(FLinearColor(1.0f, colorChanger, 0.0f));
			}
			else
			{
				// ONCE
				RemainingTimeText->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.0f));
			}
		}

		// Update Player SpeedText
		SpeedText->SetText(FText::AsNumber(FMath::FloorToInt(Mellow->MovementComponent->ForwardSpeed*0.036f)));


		// UPDATE BOOST BAR 
		// CAN STAY HERE AS IT CHANGES EVERY TICK
		// if boost is less than 100%


		//ChargeMeter_1->GetDynamicMaterial()->SetScalarParameterValue("Progress", (Mellow->MovementComponent->Charge + Mellow->MovementComponent->UsedCharge)*0.01f);


		// Update Impulse Bar
		//ImpulseBar->SetPercent((Mellow->MovementComponent->Impulse)*0.01f);

		// MAKE IMPULS ICON VISIBLE
		// TODO:
		// should be done when impulse is ready



		// SCOREBOARD


		// get list of player from gamestate

		// UPDATE SCOREBOARD ENTRIES
		// TODO: 
		// should be done when:
		// client joins game
		// player delivers orbs

		//UE_LOG(LogTemp, Error, TEXT("NUMBER PLAYERS %d"), GameState->JoinedPlayersStates.Num());

		if (GameState)
		{
			int32 i = 0;
			int32 j = 0;


			// TODO: Use https://answers.unrealengine.com/questions/606320/index.html instead

			sortedPlayerList.StableSort([](AMellowsPlayerState& A, AMellowsPlayerState& B) {
				return A.GetScore() > B.GetScore();
			});


			AMellowsPlayerState* myPlayerstate = Cast<AMellowsPlayerState>(GetOwningPlayerState());


			// SHOULD NOT SORT IT EVERY TICK BUT HEY....
			//GameState->JoinedPlayersStates.StableSort([](AMellowsPlayerState& A, AMellowsPlayerState& B) {
			//	return A.GetScore() > B.GetScore();
			//});


			// IF WE HAVE LESS PLAYERSTATES IN THE PLAYERARRAY THAN IN THE JOINEDPLAYERSTATES
			// UPDATE EVERYTHING

			// PLAYER SCOREBOARD ENTRIES
			//

			// IF PLAYERSORTED LIST IS BIGGER THAN JOINEDPLAYER

			
			for (auto& PlayerSorted : GameState->PlayerArray)//JoinedPlayersStates)//sortedPlayerList)
			{
				AMellowsPlayerState* PlayerState = Cast<AMellowsPlayerState>(PlayerSorted);

				if (PlayerState->IsValidLowLevel())
				{
					AMellowsPawn* currentPawn = Cast<AMellowsPawn>(PlayerState->GetPawn());
					if (currentPawn)
					{
						//PlayerScores += PlayerSorted->GetPlayerName() + " : " + FString::FromInt(PlayerSorted->GetScore()) + "\n";
												 // THE SCOREBOARD ENTRIES
												 //

						/*
						if (i < playerBoardEntries.Num())
						{
							playerBoardEntries[i]->Playername->SetText(FText::FromString(PlayerSorted->GetPlayerName()));
							playerBoardEntries[i]->Playerscore->SetText(FText::AsNumber(PlayerSorted->GetScore()));
							if (Mellow == currentPawn)
							{
								//playerList[i]->SetText(FText::FromString(PlayerState->GetPlayerName() + " : " + FString::FromInt(PlayerState->GetScore())));
								playerBoardEntries[i]->Playername->SetColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.3f));
								playerBoardEntries[i]->Playericon->SetVisibility(ESlateVisibility::Visible);
							}
							else {
								playerBoardEntries[i]->Playername->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f));
								playerBoardEntries[i]->Playericon->SetVisibility(ESlateVisibility::Hidden);

							}

							switch (PlayerSorted->ColorScheme)
							{
							case EColorSchemeEnum::CSE_Brown:
								//
								playerBoardEntries[i]->Playercolor->SetBrushTintColor(FLinearColor(1.f, 0.8f, 0.21f));
								break;
							case EColorSchemeEnum::CSE_Green:
								playerBoardEntries[i]->Playercolor->SetBrushTintColor(FLinearColor(0.5f, 1.f, 0.35f));
								break;
							case EColorSchemeEnum::CSE_Purple:
								playerBoardEntries[i]->Playercolor->SetBrushTintColor(FLinearColor(0.5f, 0.28f, 1.0f));
								break;
							case EColorSchemeEnum::CSE_Red:
								playerBoardEntries[i]->Playercolor->SetBrushTintColor(FLinearColor(1.0f, 0.21f, 0.21f));
								break;
							case EColorSchemeEnum::CSE_Pink:
								playerBoardEntries[i]->Playercolor->SetBrushTintColor(FLinearColor(1.0f, 0.5f, 1.0f));
								break;
							case EColorSchemeEnum::CSE_Blue:
								playerBoardEntries[i]->Playercolor->SetBrushTintColor(FLinearColor(0.1f, 0.5f, 1.0f));
								break;

							}

						}
						*/
						// ITERATE OVER INGAME PLAYERINFO
						//
						if (j < playerNamesInGame.Num())
						{
							// IF ITS NOT OUR PAWN
							if (Mellow != currentPawn)
							{

								playerNamesInGame[j]->Playername->SetText(FText::FromString(PlayerState->GetPlayerName()));
								playerNamesInGame[j]->Playerorbs->SetText(FText::AsNumber(PlayerState->FollowingActorsCount));

								switch (PlayerState->ColorScheme)
								{
								case EColorSchemeEnum::CSE_Brown:
									//
									playerNamesInGame[j]->Playercolor->SetBrushTintColor(FLinearColor(1.f, 0.8f, 0.21f));
									break;
								case EColorSchemeEnum::CSE_Green:
									playerNamesInGame[j]->Playercolor->SetBrushTintColor(FLinearColor(0.5f, 1.f, 0.35f));
									break;
								case EColorSchemeEnum::CSE_Purple:
									playerNamesInGame[j]->Playercolor->SetBrushTintColor(FLinearColor(0.5f, 0.28f, 1.0f));
									break;
								case EColorSchemeEnum::CSE_Red:
									playerNamesInGame[j]->Playercolor->SetBrushTintColor(FLinearColor(1.0f, 0.21f, 0.21f));
									break;
								case EColorSchemeEnum::CSE_Pink:
									playerNamesInGame[j]->Playercolor->SetBrushTintColor(FLinearColor(1.0f, 0.5f, 1.0f));
									break;
								case EColorSchemeEnum::CSE_Blue:
									playerNamesInGame[j]->Playercolor->SetBrushTintColor(FLinearColor(0.1f, 0.5f, 1.0f));
									break;

								}

								// SET THE LOCATION
								// should be updated every tick
								APlayerController* ownController = GetOwningPlayer();
								if (ownController)
								{
									//CanvasPanel_358->child
								   // getslot
								   // playerNamesInGame[j]->
									FVector2D screenPosition = FVector2D::FVector2D(0.f, 0.f);

									//}
									UCanvasPanelSlot * widgetCanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(playerNamesInGame[j]);

									if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(ownController, currentPawn->GetActorLocation(), screenPosition))
									{
										screenPosition.X = screenPosition.X - (widgetCanvasSlot->GetSize().X);
										screenPosition.Y = screenPosition.Y + 30.f;

										widgetCanvasSlot->SetPosition(screenPosition);
										playerNamesInGame[j]->SetVisibility(ESlateVisibility::Visible);
									}
									else
									{
										playerNamesInGame[j]->SetVisibility(ESlateVisibility::Hidden);
									}

								}
								j++;
							}

						}
					}

				}
				i++;
			}
			
		}
	}



}

void UMellowsUIWidget::StoreWidgetAnimations()
{
	AnimationsMap.Empty();

	UProperty* Prop = GetClass()->PropertyLink;

	while (Prop)
	{
		// only deal with object properties
		if (Prop->GetClass() == UObjectProperty::StaticClass())
		{
			UObjectProperty* ObjProp = Cast<UObjectProperty>(Prop);

			// only get properties that are widget animations
			if (ObjProp->PropertyClass == UWidgetAnimation::StaticClass())
			{
				UObject* Obj = ObjProp->GetOptionalPropertyValue_InContainer(this);

				UWidgetAnimation* WidgetAnimation = Cast<UWidgetAnimation>(Obj);

				if (WidgetAnimation && WidgetAnimation->MovieScene)
				{
					FName AnimName = WidgetAnimation->MovieScene->GetFName();
					AnimationsMap.Add(AnimName, WidgetAnimation);
				}
			}
		}
		Prop = Prop->PropertyLinkNext;
	}
}

UWidgetAnimation * UMellowsUIWidget::GetAnimationByName(FName AnimationName) const
{
	UWidgetAnimation* const* WidgetAnimation = AnimationsMap.Find(AnimationName);

	if (WidgetAnimation)
	{
		return *WidgetAnimation;
	}
	else
	{
		return nullptr;
	}
}

void UMellowsUIWidget::showPickupScreenOverlay()
{
	if (GotOrbsAnimation)
	{
		PlayAnimation(GotOrbsAnimation, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);

	}
}

void UMellowsUIWidget::showImpulseReadyAnimation()
{
	if (ImpulseReadyAnimation)
	{
		PlayAnimation(ImpulseReadyAnimation, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
	}
}

void UMellowsUIWidget::showBeeingRobbedAnimation()
{
	if (BeeingRobbedAnimation)
	{
		PlayAnimation(BeeingRobbedAnimation, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
	}
}

