// Fill out your copyright notice in the Description page of Project Settings.


#include "MellowsBlueprintLibrary.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

FString UMellowsBlueprintLibrary::ReadServerList()
{
	FString Result;
	FString GameLocation = FPaths::ConvertRelativePathToFull(FPaths::GameDir());
	FString Path = GameLocation.Append("ServerList.txt");
	UE_LOG(LogTemp, Log, TEXT("Look for ServerList in: %s"), *Path);
	bool worked = FFileHelper::LoadFileToString(Result, *Path);

	return Result;
}
