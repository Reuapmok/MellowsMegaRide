// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Materials/MaterialInstanceDynamic.h"
#include "MellowsPlayerState.h"
#include "MellowsPawn.h"

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MellowsPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class AMellowsPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	UFUNCTION(Server, reliable)
	void Server_PlayerIsReady(AMellowsPlayerController* PlayerController);
	void Server_PlayerIsReady_Implementation(AMellowsPlayerController* PlayerController);

	void SendPlayerReady();
	void SetGliderColor(FColorGliderParts gliderparts);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Respawn();
	void Server_Respawn_Implementation();
	bool Server_Respawn_Validate();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ChangeMap();
	void Server_ChangeMap_Implementation();
	bool Server_ChangeMap_Validate();

	UFUNCTION(Client, Reliable)
	void Client_ShowImpulseReadyAnimation();
	void Client_ShowImpulseReadyAnimation_Implementation();


	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void Respawn();
	UFUNCTION()
	void ChangeMap();

	UFUNCTION(EXEC)
	void SetGliderColor_exec(uint32 ColorIndex);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetGliderColor(uint32 ColorIndex);
	void Server_SetGliderColor_Implementation(uint32 ColorIndex);
	bool Server_SetGliderColor_Validate(uint32 ColorIndex);

	UPROPERTY(Category = Respawn, VisibleDefaultsOnly, BlueprintReadWrite)
	FVector RespawnPosition;

	// right now hard coded, but should be in an array from the size of max players I guess? But as we have a fixed size of 4 players it make editing easier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	UMaterial* ColorSchemeRed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	UMaterial* ColorSchemePurble;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	UMaterial* ColorSchemeBrown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	UMaterial* ColorSchemeGreen;

	FColorGliderParts ColorGliderParts;

	bool bColorSchemeIsPending = false;
	bool bGliderColorsPending = false;
};
