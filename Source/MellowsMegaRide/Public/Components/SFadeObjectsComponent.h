// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

// My includes
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "TimerManager.h"
#include "CollisionQueryParams.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/PrimitiveComponent.h"

#include "SFadeObjectsComponent.generated.h"

USTRUCT()
struct FFadeObjStruct
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY()
		UPrimitiveComponent* primitiveComp;

	UPROPERTY()
		TArray<UMaterialInterface*> baseMatInterface;

	UPROPERTY()
		TArray<UMaterialInstanceDynamic*> fadeMID;

	UPROPERTY()
		float fadeCurrent;

	UPROPERTY()
		bool bToHide;

	void NewElement(UPrimitiveComponent* newComponent, TArray<UMaterialInterface*> newBaseMat,
		TArray<UMaterialInstanceDynamic*> newMID, float currentFade, bool bHide)
	{
		primitiveComp = newComponent;
		baseMatInterface = newBaseMat;
		fadeMID = newMID;
		fadeCurrent = currentFade;
		bToHide = bHide;
	}

	void SetHideOnly(bool hide)
	{
		bToHide = hide;
	}

	void SetFadeAndHide(float newFade, bool newHide)
	{
		fadeCurrent = newFade;
		bToHide = newHide;
	}

	//For Destroy
	void Destroy()
	{
		primitiveComp = nullptr;
	}

	//Constructor
	FFadeObjStruct()
	{
		primitiveComp = nullptr;
		fadeCurrent = 0;
		bToHide = true;
	}
};

/* Fade objects between camera manager and character
*
*/

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MELLOWSMEGARIDE_API USFadeObjectsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USFadeObjectsComponent();

	// Called every frame
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	void BeginPlay() override;

	// Check objects between camera manager and character and add to array for fade
	void AddObjectsToHide();

	// Fade worker.
	void FadeObjWorker();

	// Enable or disable fade object worker
	void SetActivate();

	void SetDeactivate();

private:

	TArray<FFadeObjStruct> fadeObjects;

	// Some worker timer
	FTimerHandle timerHandle_ObjectComputeTimer;

	FTimerHandle timerHandle_AddObjectsTimer;

	// Temp variable
	float currentFade;

	// Now ID
	int32 fadeNowID;

	// Primitive components temp variable
	TArray<UPrimitiveComponent*> fadeObjectsTemp;

	// Primitive components temp variable
	TArray<UPrimitiveComponent*> fadeObjectsHit;

	// Translucent material for fade object
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
		UMaterial* fadeMaterial;

	// Enable or disable fade object worker
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
		bool bEnable;

	// Timer interval
	float addObjectInterval;
	float calcFadeInterval;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
		float workDistance;
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
		float nearCameraRadius;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
		UClass* playerClass;

	// Check trace block by this
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
		TArray<TEnumAsByte<ECollisionChannel>> objectTypes;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
		TArray<AActor*> actorsIgnore;

	// Rate fade increment
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
		float fadeRate;

	// Trace object size
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
		float capsuleHalfHeight;
	// Trace object size
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
		float capsuleRadius;

	// All characters array (maybe you control ( > 1 ) characters)
	TArray<AActor*> characterArray;

	// Fade near and close parameters
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
		float nearObjectFade;
	// Fade near and close parameters
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
		float farObjectFade;

	// Instance fade
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
		float immediatelyFade;
};
