// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MellowsScoreboardEntryUIWidget.generated.h"

/**
 * 
 */
UCLASS()
class MELLOWSMEGARIDE_API UMellowsScoreboardEntryUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// default widget constructor
	UMellowsScoreboardEntryUIWidget(const FObjectInitializer& ObjectInitialize);

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry & MyGeometry, float DeltaSeconds) override;


	// TEXT BLOCKS
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* Playername = nullptr;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* Playerscore = nullptr;

	// IMAGES
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UImage* Playercolor = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UImage* Playericon = nullptr;
};
