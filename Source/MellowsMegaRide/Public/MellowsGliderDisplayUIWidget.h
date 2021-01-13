// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MellowsGliderDisplayUIWidget.generated.h"

/**
 * 
 */
UCLASS()
class MELLOWSMEGARIDE_API UMellowsGliderDisplayUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMellowsGliderDisplayUIWidget(const FObjectInitializer& ObjectInitialize);

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry & MyGeometry, float DeltaSeconds) override;





	void StoreWidgetAnimations();

	UWidgetAnimation* GetAnimationByName(FName AnimationName) const;


	TMap<FName, UWidgetAnimation*> AnimationsMap;

	UWidgetAnimation* AddOrbsAnimation;
	UWidgetAnimation* LoseOrbsAnimation;


	void showAddOrbsAnimation();
	void showLoseOrbsAnimation();




	// IMPULSE ICON IMAGE
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UImage* ImpulseIconImg = nullptr;

	// IMPULSE ICON IMAGE
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UImage* ImpulseIconFullImg = nullptr;

	// BOOST BAR IMAGE
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UImage* BoostBarImg = nullptr;


	// ORBCOUNTER BAR IMAGE
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UImage* OrbCounterBarImg = nullptr;
	
	// ORCOUNTER TEXT
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UTextBlock* OrbCounterText = nullptr;

	// ORCOUNTER TEXT
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		class UImage* OrbIconPlusImg = nullptr;

};
