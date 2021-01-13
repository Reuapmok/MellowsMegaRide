// Fill out your copyright notice in the Description page of Project Settings.

#include "MellowsGliderDisplayUIWidget.h"
#include "MellowsPlayerState.h"
#include "MellowsGameState.h"
#include "MellowsPawn.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"


UMellowsGliderDisplayUIWidget::UMellowsGliderDisplayUIWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void UMellowsGliderDisplayUIWidget::NativeConstruct()
{
	Super::NativeConstruct();

	StoreWidgetAnimations();

	AddOrbsAnimation = GetAnimationByName(TEXT("AddOrbsAnim"));
	LoseOrbsAnimation = GetAnimationByName(TEXT("LoseOrbsAnim"));

}

void UMellowsGliderDisplayUIWidget::NativeTick(const FGeometry & MyGeometry, float DeltaSeconds)
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

			// UPDATE CURRENT PLAYER ORBS
			// TODO:
			// Do everytime the player:
			// delivers orbs
			// steals orbs
			// get orbs stolen
			//if (MellowState->FollowingActorsCount < 10)
			//{
			//	OrbCounterText->SetText(FText::FromString("0" + FString::FromInt(MellowState->FollowingActorsCount)));
			//}
			//else
			//{
				OrbCounterText->SetText(FText::AsNumber(MellowState->FollowingActorsCount));
			//}

			//OrbCounterBarImg->GetDynamicMaterial()->SetScalarParameterValue("Progress", (MellowState->FollowingActorsCount)*0.1f);
			if (MellowState->FollowingActorsCount > 10)
				OrbIconPlusImg->SetVisibility(ESlateVisibility::Visible);
			else
				OrbIconPlusImg->SetVisibility(ESlateVisibility::Hidden);
		}

		//BoostBarImg->GetDynamicMaterial()->SetScalarParameterValue("Progress", (Mellow->MovementComponent->Charge)*0.01f);


		//ImpulseIconFullImg->SetRenderOpacity(Mellow->MovementComponent->Impulse*0.01f);

		if (Mellow->MovementComponent->Impulse >= 100.f)
		{

			ImpulseIconImg->SetVisibility(ESlateVisibility::Hidden);
		}
		else
		{

			ImpulseIconImg->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UMellowsGliderDisplayUIWidget::StoreWidgetAnimations()
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

UWidgetAnimation * UMellowsGliderDisplayUIWidget::GetAnimationByName(FName AnimationName) const
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

void UMellowsGliderDisplayUIWidget::showAddOrbsAnimation()
{
	if (AddOrbsAnimation)
	{
		PlayAnimation(AddOrbsAnimation, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
	}
}

void UMellowsGliderDisplayUIWidget::showLoseOrbsAnimation()
{
	if (LoseOrbsAnimation)
	{
		PlayAnimation(LoseOrbsAnimation, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
	}
}
