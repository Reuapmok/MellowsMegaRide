// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnrealNetwork.h"

#include "MellowsMovementComponent.generated.h"

class AMellowsPawn;
class UAudioComponent;

USTRUCT(BlueprintType)
struct FDefaultOrientation
{
	GENERATED_BODY()

		UPROPERTY(BlueprintReadOnly)
		FVector ForwardVector = FVector(1, 0, 0);
	UPROPERTY(BlueprintReadOnly)
		FVector RightVector = FVector(0, 1, 0);
	UPROPERTY(BlueprintReadOnly)
		FVector UpVector = FVector(0, 0, 1);
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MELLOWSMEGARIDE_API UMellowsMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMellowsMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const;

public:
	// Called at the start of the parent tick 
	void UpdateParentParameters(FTransform MeshTransform, FVector Velocity, float DeltaTime);
	// returns GatheredForce and resets it to the 0 vector
	FVector ConsumeGatheredForce();

	// linetraces below the pawn against the custom hover collision channel 
	void CheckHoverableGround();
	// Updates the default orientation from the hover hit result 
	void UpdateDefaultOrientation();
	// Updates the default orientation along the x, y and z axis of the provided transform
	void UpdateDefaultOrientation(FTransform transform);

	/**
	*	Main function for handling the movement.
	*	Switches between different movement states depending on the input states.
	*/
	void ExecuteAppropriateMovementFunction();

	/**
	* Movement functions
	* Names indicate the input states that are pressed when the function is called.
	*/

	void HandleGlideChargeBoost();
	void HandleGlideBoost();
	void HandleGlideCharge();
	void HandleGlide();
	void HandleHoverChargeBoost();
	void HandleHoverChargeImpulse();
	void HandleImpulse();
	void HandleHoverBoost();
	void HandleHoverCharge();
	void HandleHover();

	/**
	* Utility functions
	*/
	// consume charge to add force along the meshes forward vector
	void ApplyBoost();
	// add strong drag and continualy add normal and impulse charge
	void ChargeUp();
	// add different drags for x, y and z axis of the parent mesh
	void AddGlideDrags();
	// continualy add force along the meshes forward vector until the velocity along this vector reaches a certain size
	void ForwardAccelleration();
	// add drags to forward and right velocities
	void AddHoveringDrags();
	// adds force to keep the mesh a certain distance above the ground
	void AddHoveringForce();


	void AddCharge(float addedCharge);
	void AddImpulse(float addedImpulse);

	void AddGatheredForce(FVector addedForce);

	/**
	* Evens
	*/
	// called when the attachment finishes
	UFUNCTION(NetMulticast, reliable)
		void Multi_OnAttachFinished(FHitResult LastHit);
	void Multi_OnAttachFinished_Implementation(FHitResult LastHit);

	/**
	* Input functions
	*/
	// Pitches the mesh relative to the default orientatin or resets it to the default orientation
	void HandlePitch(float Val);
	void HandleTurn(float Val);

	void HandleCharge(float Val);
	void HandleBoost(float Val);

	void StartImpulse();
	void StopImpulse();


	/// deprecated
	void StartBoost();
	void StopBoost();
	void StartCharge();
	void StopCharge();
	/// end deprecated


	UFUNCTION(NetMulticast, Reliable)
		void Multi_DebugLine(FVector start, FVector end);
	void Multi_DebugLine_Implementation(FVector start, FVector end);

	void SetCurrentGroundCheckMultiplier(float Value);

private:
	FVector GatheredForce;
	FTransform ParentMeshTransform;
	FVector ParentVelocity;
	float ParentDeltaTime;


	FTimerHandle BoostInitTimerHandle;



	/// Hovering 
	bool bAllowHovering = true;
	float PreviousHoverDelta = 0;
	float CurrentGroundCheckMultiplier;

	/// Controls
	bool bIsInverted = false;

	float ChargeBasedForwardDrag = GoodForwardGlideDrag;
	float ChargeBasedVerticalDrag = GoodVerticalGlideDrag;
	float ChargeBasedSideDrag = GoodSideGlideDrag;
	float ChargeBasedConversionFactor = GoodSideGlideDrag;

public:
	const FCollisionQueryParams Hover_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, GetOwner());


	/// States 
	UPROPERTY(BlueprintReadOnly, Replicated)
		bool bIsBoosting = false;

	UPROPERTY(BlueprintReadOnly, Replicated)
		bool bDisableBoosting = false;
	UPROPERTY(BlueprintReadOnly, Replicated)
		bool bIsCharging = false;
	bool bDisableCharging = false;
	UPROPERTY(BlueprintReadOnly, Replicated)
		bool bIsGliding = false;

	UPROPERTY(BlueprintReadOnly, Replicated)
		bool bIsFullyCharged = false;

	UPROPERTY(BlueprintReadWrite)
		bool bActivatedImpulse = false;
	UPROPERTY(BlueprintReadWrite)
		bool bIsTurningLeft = false;


	UPROPERTY(BlueprintReadOnly, Replicated)
		bool bIsLaunching = false;
	UPROPERTY(BlueprintReadOnly, Replicated)
		bool bIsAttaching = false;

	UPROPERTY(BlueprintReadOnly, Replicated)
		float BoostingStrength;
	UPROPERTY(BlueprintReadOnly, Replicated)
		float ChargingStrength;

	UPROPERTY(EditAnywhere, Category = Sounds)
		USoundBase* ImpulseReadySound;

	UPROPERTY(EditAnywhere, Category = Sounds)
		USoundBase* BoostSound;
	UPROPERTY(EditAnywhere, Category = Sounds)
		USoundBase* BoostInitSound;
	UPROPERTY(EditAnywhere, Category = Sounds)
		USoundBase* BoostOverheatSound;
	UAudioComponent* BoostSoundComp;
	UAudioComponent* BoostInitSoundComp;
	/// call multicast functions only when the value changed

	void SetBoosting(bool isBoosting);
	void SetCharging(bool isCharging);
	void SetGliding(bool isGliding);


	///	set value from server and updates particle system states

	UFUNCTION(NetMulticast, reliable)
		void Multi_BoostChanged(bool isBoosting, bool boostDisabled);
	void Multi_BoostChanged_Implementation(bool isBoosting,bool boostDisabled);
	UFUNCTION(NetMulticast, reliable)
		void Multi_ChargeChanged(bool isCharging);
	void Multi_ChargeChanged_Implementation(bool isCharging);
	void BoostFeedback(AMellowsPawn * Owner);
	UFUNCTION(NetMulticast, reliable)
		void Multi_GlideChanged(bool isGliding, FHitResult lastHit);
	void Multi_GlideChanged_Implementation(bool isGliding, FHitResult lastHit);



	UPROPERTY(Replicated)
		FDefaultOrientation DefaultOrientation;
	UPROPERTY(Replicated)
		FHitResult LastGroundHitResult;

	UPROPERTY(BlueprintReadOnly)
		float CurrentYawSpeed = 0.f;
	UPROPERTY(BlueprintReadOnly)
		float CurrentPitchSpeed = 0.f;
	UPROPERTY(BlueprintReadOnly)
		float CurrentRollSpeed = 0.f;

	UPROPERTY(Category = Boost, EditAnywhere)
		float BoostingForceMultiplier = 100.f;
	UPROPERTY(Category = Boost, EditAnywhere)
		float ImpulseForceMultiplier = 5000.F;

	UPROPERTY(Category = Speed, EditAnywhere, BlueprintReadOnly, replicated)
		float ForwardSpeed = 0.f;
	UPROPERTY(Category = Speed, EditAnywhere)
		float BaseTopSpeed = 2500.0f;
	UPROPERTY(Category = Speed, EditAnywhere)
		float BaseAccelleration = 15.f;// 20.f;



	UPROPERTY(Category = Hovering, EditAnywhere)
		float SpringCoefficient = 0.5f;
	UPROPERTY(Category = Hovering, EditAnywhere)
		float DampingCoefficient = 0.1f;
	UPROPERTY(Category = Hovering, EditAnywhere)
		float BaseHoverDistance = 300.0;//150.0f War im prototypen niedriger aber gleiter ist größer als stern
	UPROPERTY(Category = Hovering, EditAnywhere)
		float GroundCheckMultiplier = 1.4f;
	UPROPERTY(Category = Hovering, EditAnywhere)
		float SideHoverDrag = 0.2f;
	UPROPERTY(Category = Hovering, EditAnywhere)
		float ChargingHoverDrag = 0.1f;
	UPROPERTY(Category = Hovering, EditAnywhere)
		float ForwardHoverDrag = 0.0005f;


	UPROPERTY(Category = RotationClamp, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float MaxGlidePitch = 0.8f;//  0.5f;
	UPROPERTY(Category = RotationClamp, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float MaxHoverPitch = 0.2f;
	UPROPERTY(Category = RotationClamp, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float MaxChargePitch = 0.8;// 0.6f;
	UPROPERTY(Category = RotationClamp, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float MaxGlideRoll = 0.6f;// 0.5f;
	UPROPERTY(Category = RotationClamp, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float MaxHoverRoll = 0.3f;// 0.2f;

	UPROPERTY(Category = Takeof, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float TakeofPitch = 0.15f;// 0.35f;
	UPROPERTY(Category = Takeof, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float TakeofSpeed = 50.f;
	UPROPERTY(Category = Takeof, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float TakeofAccelleration = 750.f;

	UPROPERTY(Category = RotationSpeeds, EditAnywhere)
		float PitchSpeed = 1.5f;
	UPROPERTY(Category = RotationSpeeds, EditAnywhere)
		float RollSpeed = 3.f;// 1.f;
	UPROPERTY(Category = RotationSpeeds, EditAnywhere)
		float RollNormalizeMultiplier = 3.f;
	UPROPERTY(Category = RotationSpeeds, EditAnywhere)
		float PitchNormalizeMultiplier = 1.5f;
	UPROPERTY(Category = RotationSpeeds, EditAnywhere)
		float BaseTurnSpeed = 1.25f;// 1.5f;
	UPROPERTY(Category = RotationSpeeds, EditAnywhere)
		float ChargeTurnSpeed = 2.f;// 2.2f;




	UPROPERTY(Category = Charge, EditAnywhere)
		float PassiveChargeGain = 0.3f;
	//deprecated
	UPROPERTY(Category = Charge, EditAnywhere)
		float PassiveChargeDrain = 0.05f;
	UPROPERTY(Category = Charge, EditAnywhere)
		float ActiveChargeGain = 0.7f;
	UPROPERTY(Category = Charge, EditAnywhere)
		float ActiveChargeDrain = 0.3f;// 0.2f;
	UPROPERTY(Category = Charge, EditAnywhere)
		float ActiveImpulseGain = 0.75f;// 1.0f;
	UPROPERTY(Category = Charge, EditAnywhere)
		float PassiveImpulseGain = 0.25f;// 0.1f;
	UPROPERTY(Category = Charge, EditAnywhere, BlueprintReadOnly, replicated)
		float Charge = 0.f;
	UPROPERTY(Category = Charge, EditAnywhere, BlueprintReadOnly, replicated)
		float Impulse = 0.f;
	UPROPERTY(Category = Charge, EditAnywhere, BlueprintReadOnly, replicated)
		float UsedCharge = 0.f;
	UPROPERTY(Category = Charge, BlueprintReadOnly, EditAnywhere)
		float BaseMaxCharge = 100.f;
	UPROPERTY(Category = Charge, EditAnywhere)
		float BonusMaxCharge = 0.f;
	UPROPERTY(Category = Charge, EditAnywhere)
		float ChargeSpeed = 1.f;
	UPROPERTY(Category = Charge, EditAnywhere)
		float ChargeGlideDrag = 0.01f;
	UPROPERTY(Category = Charge, EditAnywhere)
		float ChargeFallSpeed = 20.f;

	// How much vertical velocity gets converted into forward velocity
	//UPROPERTY(Category = Gliding, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "0.9", UIMin = "0.0", UIMax = "0.9"))
	//	float GlideConvertionFactor = 0.9f;
	UPROPERTY(Category = Gliding, EditAnywhere)
		float BoostingForwardGlideDrag = 0.001f;// 0.0001f;
	UPROPERTY(Category = Gliding, EditAnywhere)
		float BoostingSideGlideDrag = 1.0f;
	UPROPERTY(Category = Gliding, EditAnywhere)
		float BoostingVerticalGlideDrag = 1.0f;
	UPROPERTY(Category = Gliding, EditAnywhere)
		float GoodForwardGlideDrag = 0.001f;
	UPROPERTY(Category = Gliding, EditAnywhere)
		float GoodSideGlideDrag = 0.1f;
	UPROPERTY(Category = Gliding, EditAnywhere)
		float GoodVerticalGlideDrag = 0.1f;
	UPROPERTY(Category = Gliding, EditAnywhere)
		float GoodConversionFactor = 0.1f;
	UPROPERTY(Category = Gliding, EditAnywhere)
		float BadForwardGlideDrag = 0.001f;
	UPROPERTY(Category = Gliding, EditAnywhere)
		float BadVerticalGlideDrag = 0.003f;
	UPROPERTY(Category = Gliding, EditAnywhere)
		float BadSideGlideDrag = 0.003f;
	UPROPERTY(Category = Gliding, EditAnywhere)
		float BadConversionFactor = 0.02f;

	UPROPERTY(Category = Gliding, EditAnywhere)
		float MinimumGlideSpeed = 100.0f;
	float MinimumGlideSpeedInv = 1.f / MinimumGlideSpeed;
};
