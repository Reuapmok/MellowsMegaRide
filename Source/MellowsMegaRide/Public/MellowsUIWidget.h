// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Containers/Array.h" 

#include "MellowsUIWidget.generated.h"


/**
 * 
 */
UCLASS()
class MELLOWSMEGARIDE_API UMellowsUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// default widget constructor
	UMellowsUIWidget(const FObjectInitializer& ObjectInitialize);

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "My New User Widget")
	//	FString MyNewWidgetName;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry & MyGeometry, float DeltaSeconds) override;



	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UCanvasPanel* CanvasPanel_358 = nullptr;



	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UVerticalBox* Playerboard = nullptr;

	// Remaing Time Text
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* RemainingTimeText = nullptr;


	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UBorder* PickupScreen = nullptr;

	// Points Text
	//UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	//	class UTextBlock* PointsText = nullptr;
	UPROPERTY(BlueprintReadWrite)
		TArray<class UMellowsScoreboardEntryUIWidget*> playerBoardEntries;

	UPROPERTY(BlueprintReadWrite)
		TArray<class UMellowsInGamePlayerInfoUIWidget*> playerNamesInGame;


	//UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	//	class UMellowsGliderDisplayUIWidget* Playerboard = nullptr;


	// ANIMATIONS
	void StoreWidgetAnimations();

	UWidgetAnimation* GetAnimationByName(FName AnimationName) const;


	TMap<FName, UWidgetAnimation*> AnimationsMap;

	UWidgetAnimation* GotOrbsAnimation;
	UWidgetAnimation* ImpulseReadyAnimation;
	UWidgetAnimation* BeeingRobbedAnimation;


	void showPickupScreenOverlay();

	void showImpulseReadyAnimation();

	void showBeeingRobbedAnimation();


	//UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	//	TArray<class UTextBlock*> playerList;

	//UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	//	TArray<class UImage*> playerListBackground;

	//UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	//	TArray<class UImage*> ownPlayerBackground;


	//UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	//	class UImage* ImpulseIcon = nullptr;




	TArray<class AMellowsPlayerState*> sortedPlayerList;

	// Stole Message Text
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class URichTextBlock* MessageText = nullptr;

	// Game end Text
	//UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	//	class UTextBlock* GameEndText = nullptr;


	// Join Game Text
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class URichTextBlock* JoinGameText = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UMellowsGliderDisplayUIWidget* GliderInfoWidget = nullptr;

	// Scoreboard list
	//UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	//	class UTextBlock* ScoreBoard = nullptr;

	//UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	//	class UTextBlock* RightBumper = nullptr;




	// ForwardSpeed Text
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* SpeedText = nullptr;

	// Charge ProgressBar
	//UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	//	class UProgressBar* ChargeBar = nullptr;

	// todo: Testing circular charge meter based on image


	//UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	//	class UImage* ChargeMeter_1 = nullptr;

//	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	//	class UImage* ImpulseMeter = nullptr;




	
};


