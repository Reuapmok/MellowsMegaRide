// Fill out your copyright notice in the Description page of Project Settings.


#include "MellowsGameOverScoreUIWidget.h"

UMellowsGameOverScoreUIWidget::UMellowsGameOverScoreUIWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void UMellowsGameOverScoreUIWidget::NativeConstruct()
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

void UMellowsGameOverScoreUIWidget::NativeTick(const FGeometry & MyGeometry, float DeltaSeconds)
{
	Super::NativeTick(MyGeometry, DeltaSeconds);
}