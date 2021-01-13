// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MellowsBlueprintLibrary.generated.h"

/**
 * 
 */
UCLASS()
class MELLOWSMEGARIDE_API UMellowsBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

		UFUNCTION(BlueprintCallable, Category = "FileIO")
		static FString ReadServerList();
};
