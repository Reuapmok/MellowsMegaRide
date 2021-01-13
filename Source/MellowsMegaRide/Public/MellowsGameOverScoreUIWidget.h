// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MellowsGameOverScoreUIWidget.generated.h"

/**
 * 
 */
UCLASS()
class MELLOWSMEGARIDE_API UMellowsGameOverScoreUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// default widget constructor
	UMellowsGameOverScoreUIWidget(const FObjectInitializer& ObjectInitialize);

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry & MyGeometry, float DeltaSeconds) override;




	// PLAYERNAME		(TEXTBLOCK)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* Playername = nullptr;

	// PLAYERSCORE		(TEXTBLOCK)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* Playerscore = nullptr;

	// PLAYER RANK		(IMAGE)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UImage* PlayerrankIcon = nullptr;

	// PLAYERNAME		(TEXTBLOCK)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* PlayerrankText = nullptr;

	// PLAYER RANK		(IMAGE)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UImage* PlayerColor = nullptr;

	// PLAYER RANK		(IMAGE)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UImage* PlayerColorIcon = nullptr;
	
};
