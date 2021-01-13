// Fill out your copyright notice in the Description page of Project Settings.Server_PlayerIsReady_Implementation


#include "MellowsPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "MellowsPlayerState.h"
#include "Engine/World.h"
#include "MellowsGameState.h"
#include "MellowsPawn.h"
#include "MellowsPlayerState.h"
#include "Components/InputComponent.h"
#include "MelllowsGameHUD.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"

#define GETENUMSTRING(etype, evalue) ( (FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true) != nullptr) ? FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true)->GetEnumName((int32)evalue) : FString("Invalid - are you sure enum uses UENUM() macro?") )

void AMellowsPlayerController::Server_PlayerIsReady_Implementation(AMellowsPlayerController* PlayerController)
{
	UE_LOG(LogTemp, Log, TEXT("Player %d said it's ready!"), this->PlayerState->PlayerId);
	AMellowsGameState* GameState = Cast<AMellowsGameState>(GetWorld()->GetGameState());
	check(GameState != nullptr && "GameState is null in PlayerController");

	GameState->PlayerIsReady(PlayerController);
}


void AMellowsPlayerController::SendPlayerReady()
{
	Server_PlayerIsReady(this);

	// HUD 
	AMelllowsGameHUD* PlayerHud = Cast<AMelllowsGameHUD>(this->GetHUD());
	if (PlayerHud)
	{
		// CHANGE JOIN TEXT
		UMellowsUIWidget* MellowsUIWidget = PlayerHud->MellowsUIWidget;
		if(MellowsUIWidget)
			PlayerHud->MellowsUIWidget->JoinGameText->SetText(FText::FromString(FString("Waiting for other players")));
	}
}

void AMellowsPlayerController::SetGliderColor(FColorGliderParts gliderparts)
{
	AMellowsPawn* MellowsPawn = Cast<AMellowsPawn>(GetPawn());
	if (MellowsPawn)
	{
		//UE_LOG(LogTemp, Log, TEXT("Set body to %d and wings to %d"), BodyColor, WingColor);
		this->ColorGliderParts = gliderparts;
		MellowsPawn->ColorGliderParts = ColorGliderParts;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Pawn is not there yet."));
		this->ColorGliderParts = gliderparts;
		bGliderColorsPending = true;
	}
}

void AMellowsPlayerController::Server_Respawn_Implementation()
{
	APawn* pawn = this->GetPawn();
	if (pawn)
	{
		UE_LOG(LogTemp, Log, TEXT("Client controls Pawn, so no respawn for you"));
	}
	else
	{
		pawn = GetWorld()->GetAuthGameMode()->SpawnDefaultPawnAtTransform(this, FTransform(RespawnPosition));
		this->Possess(pawn);
	}
}

bool AMellowsPlayerController::Server_Respawn_Validate()
{
	return true;
}

void AMellowsPlayerController::Server_ChangeMap_Implementation()
{
	GetWorld()->ServerTravel(FString("First_Draft_Map"));
}

bool AMellowsPlayerController::Server_ChangeMap_Validate()
{
	return true;
}


void AMellowsPlayerController::Client_ShowImpulseReadyAnimation_Implementation()
{
	AMelllowsGameHUD* PlayerHud = Cast<AMelllowsGameHUD>(GetHUD());
	if (PlayerHud)
		PlayerHud->MellowsUIWidget->showImpulseReadyAnimation();
}

void AMellowsPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	check(InputComponent && "Inputcomponent is somehow null");

	InputComponent->BindAction("Respawn", IE_Pressed, this, &AMellowsPlayerController::Respawn);
	InputComponent->BindAction("ChangeMap", IE_Pressed, this, &AMellowsPlayerController::ChangeMap);
	InputComponent->BindAction("Confirm", IE_Pressed, this, &AMellowsPlayerController::SendPlayerReady);
}

void AMellowsPlayerController::OnPossess(APawn * InPawn)
{
	Super::OnPossess(InPawn);

	if (bGliderColorsPending)
	{
		AMellowsPawn* MellowsPawn = Cast<AMellowsPawn>(GetPawn());
		if (MellowsPawn)
		{
			UE_LOG(LogTemp, Log, TEXT("Set body to %d and wings to %d"), ColorGliderParts.BodyColor, ColorGliderParts.WingColor);
			MellowsPawn->ColorGliderParts = this->ColorGliderParts;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Pawn still not there??? Even after it was possessed."));
		}
	}
}

void AMellowsPlayerController::Respawn()
{
	Server_Respawn();
}

void AMellowsPlayerController::ChangeMap()
{
	Server_ChangeMap();
}

void AMellowsPlayerController::SetGliderColor_exec(uint32 ColorIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("In exec"));
	Server_SetGliderColor(ColorIndex);
}

void AMellowsPlayerController::Server_SetGliderColor_Implementation(uint32 ColorIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("In Server change color"));
	AMellowsPawn* MellowsPawn = Cast<AMellowsPawn>(GetPawn());
	if (MellowsPawn)
	{
		MellowsPawn->ColorGliderParts.BodyColor = ColorIndex;
		UE_LOG(LogTemp, Warning, TEXT("Changed color."));
	}
}

bool AMellowsPlayerController::Server_SetGliderColor_Validate(uint32 ColorIndex)
{
	return true;
}


