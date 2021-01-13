// Fill out your copyright notice in the Description page of Project Settings.

#include "SFadeObjectsComponent.h"

#include "MellowsPawn.h"

USFadeObjectsComponent::USFadeObjectsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	bEnable = true;

	addObjectInterval = 0.1f;
	calcFadeInterval = 0.05f;

	fadeRate = 10.0f;

	capsuleHalfHeight = 88.0f;
	capsuleRadius = 34.0f;

	workDistance = 5000.0f;
	nearCameraRadius = 300.0f;

	nearObjectFade = 0.3;
	farObjectFade = 0.1;

	immediatelyFade = 0.5f;

	// Add first collision type
	objectTypes.Add(ECC_WorldStatic);

	// ...
}

// Called when the game starts
void USFadeObjectsComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bEnable && (GetOwnerRole() == ROLE_AutonomousProxy))
	{
		GetWorld()->GetTimerManager().SetTimer(timerHandle_AddObjectsTimer, this, &USFadeObjectsComponent::AddObjectsToHide,
			addObjectInterval, true);
		GetWorld()->GetTimerManager().SetTimer(timerHandle_ObjectComputeTimer, this, &USFadeObjectsComponent::FadeObjWorker,
			calcFadeInterval, true);
	}
}

// Called every frame
void USFadeObjectsComponent::TickComponent(const float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USFadeObjectsComponent::AddObjectsToHide()
{
	AMellowsPawn* OwnerMellowsPawn = Cast<AMellowsPawn >(GetOwner());
	if (OwnerMellowsPawn) {
		const FVector traceStart = OwnerMellowsPawn->GetActiveCamera()->GetComponentLocation();
		const FVector traceEnd = OwnerMellowsPawn->GetActorLocation();
		const FRotator traceRot = OwnerMellowsPawn->GetActorRotation();
		FVector traceLentgh = traceStart - traceEnd;
		const FQuat acQuat = OwnerMellowsPawn->GetActorQuat();

		if (traceLentgh.Size() < workDistance)
		{
			FCollisionQueryParams traceParams(TEXT("FadeObjectsTrace"), true, GetOwner());

			traceParams.AddIgnoredActors(actorsIgnore);
			//traceParams.bTraceAsyncScene = true;
			traceParams.bReturnPhysicalMaterial = false;
			// Not tracing complex uses the rough collision instead making tiny objects easier to select.
			traceParams.bTraceComplex = false;

			TArray<FHitResult> hitArray;
			TArray<TEnumAsByte<EObjectTypeQuery>> traceObjectTypes;

			// Convert ECollisionChannel to ObjectType
			for (int i = 0; i < objectTypes.Num(); ++i)
			{
				traceObjectTypes.Add(UEngineTypes::ConvertToObjectType(objectTypes[i].GetValue()));
			}

			// Check distance between camera and player for new object to fade, and add this in array
			GetWorld()->SweepMultiByObjectType(hitArray, traceStart, traceEnd, acQuat, traceObjectTypes,
				FCollisionShape::MakeCapsule(capsuleRadius, capsuleHalfHeight), traceParams);

			for (int hA = 0; hA < hitArray.Num(); ++hA)
			{
				if (hitArray[hA].bBlockingHit && IsValid(hitArray[hA].GetComponent()) && !fadeObjectsHit.Contains(hitArray[hA].GetComponent()))
				{
					fadeObjectsHit.AddUnique(hitArray[hA].GetComponent());
				}
			}

		}
	}

	// Make fade array after complete GetAllActorsOfClass loop
	for (int fO = 0; fO < fadeObjectsHit.Num(); ++fO)
	{
		// If not contains this component in fadeObjectsTemp
		if (!fadeObjectsTemp.Contains(fadeObjectsHit[fO]))
		{
			TArray<UMaterialInterface*> lBaseMaterials;
			TArray<UMaterialInstanceDynamic*> lMidMaterials;

			lBaseMaterials.Empty();
			lMidMaterials.Empty();

			fadeObjectsTemp.AddUnique(fadeObjectsHit[fO]);

			// For loop all materials ID in object
			for (int nM = 0; nM < fadeObjectsHit[fO]->GetNumMaterials(); ++nM)
			{
				lMidMaterials.Add(UMaterialInstanceDynamic::Create(fadeMaterial, fadeObjectsHit[fO]));
				lBaseMaterials.Add(fadeObjectsHit[fO]->GetMaterial(nM));

				// Set new material on object
				fadeObjectsHit[fO]->SetMaterial(nM, lMidMaterials.Last());
			}
			// Create new fade object in array of objects to fade
			FFadeObjStruct newObject;
			newObject.NewElement(fadeObjectsHit[fO], lBaseMaterials, lMidMaterials, immediatelyFade, true);
			// Add object to array
			fadeObjects.Add(newObject);

			// Set collision on Primitive Component
			fadeObjectsHit[fO]->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		}
	}

	// Set hide to visible true if contains
	for (int fOT = 0; fOT < fadeObjectsTemp.Num(); ++fOT)
	{
		if (!fadeObjectsHit.Contains(fadeObjectsTemp[fOT]))
		{
			fadeObjects[fOT].SetHideOnly(false);
		}
	}

	// Clear array
	fadeObjectsHit.Empty();
}

void USFadeObjectsComponent::FadeObjWorker()
{
	if (fadeObjects.Num() > 0)
	{
		// For loop all fade objects
		for (int i = 0; i < fadeObjects.Num(); ++i)
		{
			// Index of iteration
			int fnID = i;

			float adaptiveFade;

			if (fnID == fadeObjects.Num())
			{
				adaptiveFade = nearObjectFade;
			}
			else
			{
				adaptiveFade = farObjectFade;
			}

			// For loop fadeMID array
			for (int t = 0; t < fadeObjects[i].fadeMID.Num(); ++t)
			{
				float targetF;

				const float currentF = fadeObjects[i].fadeCurrent;

				if (fadeObjects[i].bToHide)
				{
					targetF = adaptiveFade;
				}
				else
				{
					targetF = 1.0f;
				}

				const float newFade = FMath::FInterpConstantTo(currentF, targetF, GetWorld()->GetDeltaSeconds(), fadeRate);

				fadeObjects[i].fadeMID[t]->SetScalarParameterValue("Fade", newFade);

				currentFade = newFade;

				fadeObjects[i].SetFadeAndHide(newFade, fadeObjects[i].bToHide);
			}
			// remove index in array
			if (currentFade == 1.0f)
			{
				for (int bmi = 0; bmi < fadeObjects[fnID].baseMatInterface.Num(); ++bmi)
				{
					UPrimitiveComponent * fadeObjectPrimitveComp = fadeObjects[fnID].primitiveComp;
					if (fadeObjectPrimitveComp->IsValidLowLevel())
					{
 						fadeObjects[fnID].primitiveComp->SetMaterial(bmi, nullptr);
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("Set SpeedlinesMaterial in MellowsPawn!"));
					}
				}

				fadeObjects[fnID].primitiveComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Overlap);
				fadeObjects.RemoveAt(fnID);
				fadeObjectsTemp.RemoveAt(fnID);
			}
		}
	}
}

void USFadeObjectsComponent::SetActivate()
{
	bEnable = true;
}

void USFadeObjectsComponent::SetDeactivate()
{
	bEnable = false;
}