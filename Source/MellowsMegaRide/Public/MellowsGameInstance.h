// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MellowsGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class MELLOWSMEGARIDE_API UMellowsGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// Controls
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	bool bPitchInverted = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	bool bCameraInverted = false;
};
