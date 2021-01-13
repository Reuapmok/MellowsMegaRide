// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Components/VerticalBox.h" 
#include "MellowsUIWidget.h"
#include "MellowsScoreboardEntryUIWidget.h"
#include "MellowsGameOverUIWidget.h"
#include "MellowsInGamePlayerInfoUIWidget.h"
#include "MellowsPlayerController.h"
#include "Components/CanvasPanelSlot.h" 
#include "Components/CanvasPanel.h" 
#include "MellowsGameOverScoreUIWidget.h"
#include "MellowsGameState.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Components/Image.h"

#include "MelllowsGameHUD.generated.h"





/**
 * 
 */

UCLASS()
class MELLOWSMEGARIDE_API AMelllowsGameHUD : public AHUD
{
	GENERATED_BODY()

public:
	AMelllowsGameHUD();

	virtual void DrawHUD() override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	void createGameEndWidget();
	void addMellowsUIWidget();
	void addPlayerEntryUIWidget(AMellowsPlayerState* newPlayerEntry);
	void addInGamePlayerInfoUIWidget(AMellowsPlayerState* newPlayerEntry);

	void updatePlayerEntryUIWidgets();


	// NEW ONES
	void updateScoreBoard();		// CREATE A TEMP COPY OF PLAYERSTATE LIST, SORT THE LIST, UPDATE HUD
	void reCreateScoreBoard();		// DELETES ALL CHILDS OF SCOREBOARD AND CREATES NEW ONES FOR EVERY PLAYERSTATE





	UPROPERTY(EditDefaultsOnly, Category = "Interactive")
	TSubclassOf<UUserWidget> MellowsUIWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Interactive")
	TSubclassOf<UUserWidget> MellowsGameOverUIWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Interactive")
		TSubclassOf<UUserWidget> MellowsGameOverPlayerEntryUIWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Interactive")
	TSubclassOf<UUserWidget> MellowsPlayerEntryUIWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Interactive")
	TSubclassOf<UUserWidget> MellowsInGamePlayerInfoUIWidgetClass;




	
//private:
	// GAME OVER WIDGET
	UPROPERTY(BlueprintReadWrite)
	UMellowsGameOverUIWidget* MellowsGameOverUIWidget;

	// MAIN GAME WIDGET
	UPROPERTY(BlueprintReadWrite)
	UMellowsUIWidget* MellowsUIWidget;
};
