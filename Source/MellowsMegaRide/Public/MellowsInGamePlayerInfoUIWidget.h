// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MellowsInGamePlayerInfoUIWidget.generated.h"

/**
 * 
 */
UCLASS()
class MELLOWSMEGARIDE_API UMellowsInGamePlayerInfoUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// default widget constructor
	UMellowsInGamePlayerInfoUIWidget(const FObjectInitializer& ObjectInitialize);

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry & MyGeometry, float DeltaSeconds) override;



	// TEXT BLOCKS
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* Playername = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* Playerorbs = nullptr;

	// IMAGES
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UImage* Playercolor = nullptr;
	
};
