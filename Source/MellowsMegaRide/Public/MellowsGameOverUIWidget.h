// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MellowsGameOverUIWidget.generated.h"

/**
 * 
 */
UCLASS()
class MELLOWSMEGARIDE_API UMellowsGameOverUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// default widget constructor
	UMellowsGameOverUIWidget(const FObjectInitializer& ObjectInitialize);

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry & MyGeometry, float DeltaSeconds) override;

	void setScoreBoard(bool verdict);

	// VERTICAL BOX
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UVerticalBox* PlayerVerdictBoard = nullptr;

	// LIST OF WIDGETS
	UPROPERTY(BlueprintReadWrite)
		TArray<class UMellowsGameOverScoreUIWidget*> playerVerdictBoardEntries;




	// Game end Text
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* GameEndText = nullptr;

	TArray<class AMellowsPlayerState*> sortedPlayerList;

	//UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	//	TArray<class UTextBlock*> playerList;

	
};
