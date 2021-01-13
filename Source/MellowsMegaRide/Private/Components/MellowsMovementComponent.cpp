// Fill out your copyright notice in the Description page of Project Settings.

#include "MellowsMovementComponent.h"
#include "Engine/GameEngine.h"
#include "Audio.h"
#include "TimerManager.h"
#include "MellowsPawn.h"
#include "DrawDebugHelpers.h"
#include "MelllowsGameHUD.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

#define COLLISION_HOVER        ECC_GameTraceChannel1
#define INVERSE_SPEED_MULTIPLIER 27.f
#define SPEED_MULTIPLIER 0.036f

// Sets default values for this component's properties
UMellowsMovementComponent::UMellowsMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	// ...
}


// Called when the game starts
void UMellowsMovementComponent::BeginPlay()
{
	Super::BeginPlay();


	BoostSoundComp = UGameplayStatics::CreateSound2D(GetWorld(), BoostSound);
	BoostInitSoundComp = UGameplayStatics::CreateSound2D(GetWorld(), BoostInitSound);

	MinimumGlideSpeedInv = 1.f / MinimumGlideSpeed;
	CurrentGroundCheckMultiplier = GroundCheckMultiplier;
}

void UMellowsMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMellowsMovementComponent, bIsGliding);
	DOREPLIFETIME(UMellowsMovementComponent, bIsAttaching);
	DOREPLIFETIME(UMellowsMovementComponent, bIsBoosting);
	DOREPLIFETIME(UMellowsMovementComponent, bIsCharging);
	DOREPLIFETIME(UMellowsMovementComponent, BoostingStrength);
	DOREPLIFETIME(UMellowsMovementComponent, ChargingStrength);
	DOREPLIFETIME(UMellowsMovementComponent, Charge);
	DOREPLIFETIME(UMellowsMovementComponent, Impulse);
	DOREPLIFETIME(UMellowsMovementComponent, UsedCharge);
	DOREPLIFETIME(UMellowsMovementComponent, bIsLaunching);
	DOREPLIFETIME(UMellowsMovementComponent, ForwardSpeed);
	DOREPLIFETIME(UMellowsMovementComponent, DefaultOrientation);
	DOREPLIFETIME(UMellowsMovementComponent, LastGroundHitResult);
	DOREPLIFETIME(UMellowsMovementComponent, bIsFullyCharged);
	DOREPLIFETIME(UMellowsMovementComponent, bDisableBoosting);
}

void UMellowsMovementComponent::UpdateParentParameters(FTransform MeshTransform, FVector Velocity, float DeltaTime)
{
	ParentMeshTransform = MeshTransform;
	ParentVelocity = Velocity;
	ParentDeltaTime = DeltaTime;
}


void UMellowsMovementComponent::CheckHoverableGround()
{
	const bool prevBIsGliding = bIsGliding;
	if (bAllowHovering)
	{
		const FVector CurrentLocation = ParentMeshTransform.GetLocation();
		const FVector GroundCheckEnd = CurrentLocation - (BaseHoverDistance * CurrentGroundCheckMultiplier * DefaultOrientation.UpVector);

		if (GetNetMode() == ENetMode::NM_DedicatedServer)
		{
			//Multi_DebugLine(CurrentLocation, GroundCheckEnd);
		}
		else
		{
			//DrawDebugLine(GetWorld(), CurrentLocation, GroundCheckEnd, FColor::Green, false, 0.1f, 0.f, 20.f);
		}

		//const FVector DebugForward = CurrentLocation - (BaseHoverDistance * GroundCheckMultiplier * DefaultOrientation.ForwardVector);

		LastGroundHitResult.Init();
		GetWorld()->LineTraceSingleByChannel(LastGroundHitResult, CurrentLocation, GroundCheckEnd, COLLISION_HOVER, Hover_TraceParams);
		SetGliding(!LastGroundHitResult.bBlockingHit);
		if (LastGroundHitResult.bBlockingHit)
		{
			bIsAttaching = false;
		}
	}
	else
	{
		LastGroundHitResult.Init();
		SetGliding(true);
	}
	if (!prevBIsGliding && bIsGliding)
	{
		AddGatheredForce(ParentMeshTransform.GetRotation().GetForwardVector() * TakeofAccelleration);
	}
}

void UMellowsMovementComponent::UpdateDefaultOrientation()
{
	FVector newUp;
	FVector newForward;
	newUp = LastGroundHitResult.bBlockingHit ?
		FMath::VInterpTo(DefaultOrientation.UpVector, LastGroundHitResult.ImpactNormal, GetWorld()->GetDeltaSeconds(), 7.f) :
		FMath::VInterpTo(DefaultOrientation.UpVector, FVector::UpVector, GetWorld()->GetDeltaSeconds(), 5.f);
	newUp /= newUp.Size();
	newForward = (FVector::VectorPlaneProject(ParentMeshTransform.GetRotation().GetForwardVector(), newUp));
	newForward /= newForward.Size();

	FVector newRight(FVector::CrossProduct(newUp, newForward));
	DefaultOrientation.ForwardVector = newForward;
	DefaultOrientation.RightVector = newRight;
	DefaultOrientation.UpVector = newUp;
}

void UMellowsMovementComponent::UpdateDefaultOrientation(FTransform transform)
{
	DefaultOrientation.ForwardVector = transform.GetRotation().GetForwardVector();
	DefaultOrientation.RightVector = transform.GetRotation().GetRightVector();
	DefaultOrientation.UpVector = transform.GetRotation().GetUpVector();
}

void UMellowsMovementComponent::ExecuteAppropriateMovementFunction()
{
	if (bIsGliding)
	{
		if (bIsCharging)
		{
			if (bIsBoosting)
			{
				HandleGlideChargeBoost();
			}
			else
			{
				HandleGlideCharge();
			}
		}
		else
		{
			if (bIsBoosting)
			{
				HandleGlideBoost();
			}
			else
			{
				HandleGlide();
			}
		}

	}
	else
	{
		if (bIsCharging)
		{
			if (bIsBoosting)
			{
				HandleHoverChargeBoost();
			}
			//else if (bIsLaunching)
			//{
			//	HandleHoverChargeImpulse();
			//}
			else
			{
				HandleHoverCharge();
			}
		}
		else
		{
			if (bIsBoosting)
			{
				HandleHoverBoost();
			}
			else
			{
				HandleHover();
			}
		}
	}

	if (bIsLaunching)
	{
		HandleImpulse();
	}
	else
	{
		CurrentGroundCheckMultiplier = FMath::FInterpConstantTo(CurrentGroundCheckMultiplier, GroundCheckMultiplier, GetWorld()->GetDeltaSeconds(), 1.f);
	}

	const FVector forwardVelocity = (ParentVelocity - FVector::VectorPlaneProject(ParentVelocity, ParentMeshTransform.GetRotation().GetForwardVector()));
	// set the forwardSpeed for visual stuff
	ForwardSpeed = forwardVelocity.Size();

	const FVector normalizedForwardVelocity = forwardVelocity.GetSafeNormal();

	if (FVector::DotProduct(ParentMeshTransform.GetRotation().GetForwardVector(), normalizedForwardVelocity) < 0)
	{
		GatheredForce -= forwardVelocity * 0.2;
	}
}

void UMellowsMovementComponent::HandleGlideChargeBoost()
{
	HandleGlideCharge();
}

void UMellowsMovementComponent::HandleGlideBoost()
{
	ChargeBasedForwardDrag = FMath::FInterpConstantTo(ChargeBasedForwardDrag, BoostingForwardGlideDrag, ParentDeltaTime, 2.0f);
	ChargeBasedVerticalDrag = FMath::FInterpConstantTo(ChargeBasedVerticalDrag, BoostingVerticalGlideDrag, ParentDeltaTime, 2.0f);
	ChargeBasedSideDrag = FMath::FInterpConstantTo(ChargeBasedSideDrag, BoostingSideGlideDrag, ParentDeltaTime, 2.0f);
	
	if (Impulse < BaseMaxCharge)
	{
		AddImpulse(-BaseMaxCharge * PassiveChargeGain * ParentDeltaTime * (ChargeSpeed));
	}

	ApplyBoost();
	AddGlideDrags();
}

void UMellowsMovementComponent::HandleGlideCharge()
{
	const FVector XYVelocity = FVector(ParentVelocity.X, ParentVelocity.Y, 0.f);

	//prevent upwards movement
	if (ParentVelocity.Z > 0.f)
	{
		GatheredForce -= FVector::UpVector * ParentVelocity.Z * ChargeGlideDrag;
	}

	ChargeBasedForwardDrag = FMath::FInterpConstantTo(ChargeBasedForwardDrag, BadForwardGlideDrag, ParentDeltaTime, 2.0f);
	ChargeBasedVerticalDrag = FMath::FInterpConstantTo(ChargeBasedVerticalDrag, BadVerticalGlideDrag, ParentDeltaTime, 2.5f);
	ChargeBasedSideDrag = FMath::FInterpConstantTo(ChargeBasedSideDrag, BadSideGlideDrag, ParentDeltaTime, 2.5f);

	ChargeBasedConversionFactor = FMath::FInterpConstantTo(ChargeBasedConversionFactor, GoodConversionFactor, ParentDeltaTime, 2.5f);

	ChargeUp();

	// slow down and fall
	GatheredForce += FVector(0, 0, -ChargeFallSpeed);
	GatheredForce -= XYVelocity * ChargeGlideDrag;
}

void UMellowsMovementComponent::HandleGlide()
{
	ChargeBasedForwardDrag = FMath::FInterpConstantTo(ChargeBasedForwardDrag, GoodForwardGlideDrag, ParentDeltaTime, 0.2f);
	ChargeBasedVerticalDrag = FMath::FInterpConstantTo(ChargeBasedVerticalDrag, GoodVerticalGlideDrag, ParentDeltaTime, 0.2f);
	ChargeBasedSideDrag = FMath::FInterpConstantTo(ChargeBasedSideDrag, GoodSideGlideDrag, ParentDeltaTime, 0.2f);
	ChargeBasedConversionFactor = FMath::FInterpConstantTo(ChargeBasedConversionFactor, BadConversionFactor, ParentDeltaTime, 0.2f);


	AddGlideDrags();

	if (Impulse < BaseMaxCharge)
	{
		AddImpulse(-BaseMaxCharge * PassiveChargeGain * ParentDeltaTime * (ChargeSpeed));
	}

	float gainedCharge = BaseMaxCharge * PassiveChargeGain * ParentDeltaTime * (ChargeSpeed);
	AddCharge(gainedCharge);
}


void UMellowsMovementComponent::HandleHoverChargeBoost()
{
	AddHoveringForce();
	AddHoveringDrags();

	ChargeUp();

	// slow down
	GatheredForce -= ParentVelocity * ChargingHoverDrag * ChargingStrength;
}

void UMellowsMovementComponent::HandleHoverChargeImpulse()
{
	//UE_LOG(LogTemp, Warning, TEXT("HandleHoverChargeBoost"));
	AddHoveringForce();

	ChargeUp();

	AddHoveringDrags();

}

void UMellowsMovementComponent::HandleImpulse()
{
	bActivatedImpulse = false;

	if (Impulse >= BaseMaxCharge * 0.99f)
	{
		CurrentGroundCheckMultiplier = 0.0f;
		GatheredForce += ParentMeshTransform.GetRotation().GetForwardVector() * ImpulseForceMultiplier;
		Impulse = 0;
		bDisableCharging = true;
		bAllowHovering = false;
		SetCharging(false);
		SetBoosting(false);



		bActivatedImpulse = true;

		AMellowsPawn* parent = Cast<AMellowsPawn>(GetOwner());

		if (GetNetMode() == NM_DedicatedServer && parent)
		{
			parent->Multi_StartImpulseParticles();
		}

	}
}


void UMellowsMovementComponent::HandleHoverBoost()
{
	AddHoveringForce();
	AddHoveringDrags();

	CurrentGroundCheckMultiplier = 1.f;
	ApplyBoost();
	ForwardAccelleration();
}

void UMellowsMovementComponent::ApplyBoost()
{
	const float CurrentChargeDrain = ActiveChargeDrain * BoostingStrength + PassiveChargeDrain * (1 - BoostingStrength);
	//consume charge and gain more accelleration
	const float ChargeCost = BaseMaxCharge * CurrentChargeDrain * ParentDeltaTime * (ChargeSpeed);
	UsedCharge += ChargeCost;
	AddCharge(-ChargeCost);
	// Todo add stats
	if (Charge >= 0.01f)
	{
		float TargetBoostVelocity = 300.f * INVERSE_SPEED_MULTIPLIER;
		float DifferenceToTargetMultiplier = FMath::Clamp(TargetBoostVelocity - ForwardSpeed, 0.f, TargetBoostVelocity) / TargetBoostVelocity;
		GatheredForce += ParentMeshTransform.GetRotation().GetForwardVector() * BoostingForceMultiplier * BoostingStrength * DifferenceToTargetMultiplier;
	}
	else
	{
		if (GetNetMode() == ENetMode::NM_DedicatedServer)
		{
			SetBoosting(false);
		}
		UsedCharge = 0;
	}
}

void UMellowsMovementComponent::HandleHoverCharge()
{
	AddHoveringForce();
	AddHoveringDrags();

	ChargeUp();

	//slow down to a halt
	GatheredForce -= ParentVelocity * ChargingHoverDrag * ChargingStrength;
}

void UMellowsMovementComponent::HandleHover()
{
	AddHoveringForce();
	AddHoveringDrags();
	ChargeUp();
	ForwardAccelleration();
}

void UMellowsMovementComponent::ChargeUp()
{
	// Charge slow when gliding and fast when on hovering
	const float CurrentChargeGain = (bIsGliding || bIsCharging) ? PassiveChargeGain : ActiveChargeGain;
	const float CurrentImpulseChargeGain = bIsGliding ? PassiveImpulseGain : ActiveImpulseGain;

	//The actual charge gained
	float gainedCharge = BaseMaxCharge * CurrentChargeGain * ParentDeltaTime * (ChargeSpeed);
	float gainedImpulse = BaseMaxCharge * CurrentImpulseChargeGain * ParentDeltaTime * (ChargeSpeed);

	//check for full impulse for the particles
	if (Impulse >= BaseMaxCharge)
	{
		if (!bIsFullyCharged)
		{
			if (GetNetMode() == ENetMode::NM_DedicatedServer)
			{
				AMellowsPawn* MellowsParent = Cast<AMellowsPawn>(GetOwner());
				if (MellowsParent->IsValidLowLevel())
				{
					// SHOW IMPULSE READY SCREEN ANIMATION
					AMellowsPlayerController* MPlayerController = Cast<AMellowsPlayerController>(MellowsParent->GetController());
					if (MPlayerController->IsValidLowLevel()) { MPlayerController->Client_ShowImpulseReadyAnimation(); }

					MellowsParent->Client_PlaySound(ImpulseReadySound);
				}
			}
			Multi_ChargeChanged(bIsCharging);
		}
		bIsFullyCharged = true;

	}
	else
	{
		bIsFullyCharged = false;
	}

	//Add the charges
	AddCharge(gainedCharge);
	AddImpulse(gainedImpulse);


}

void UMellowsMovementComponent::AddGlideDrags()
{
	const FVector VerticalVelocity = ParentVelocity - FVector::VectorPlaneProject(ParentVelocity, ParentMeshTransform.GetRotation().GetUpVector());
	const FVector ForwardVelocity = ParentVelocity - FVector::VectorPlaneProject(ParentVelocity, ParentMeshTransform.GetRotation().GetForwardVector());
	const FVector SideVelocity = ParentVelocity - FVector::VectorPlaneProject(ParentVelocity, ParentMeshTransform.GetRotation().GetRightVector());

	/// negate the extracted velocities with different multipliers
	const FVector ForwardDrag = -ForwardVelocity * ChargeBasedForwardDrag;

	const float speedValue = ForwardSpeed * SPEED_MULTIPLIER;
	const float SpeedModifier = FMath::Clamp(speedValue * speedValue * MinimumGlideSpeedInv * MinimumGlideSpeedInv, 0.f, 1.f);
	//UE_LOG(LogTemp, Warning, TEXT("SpeedModifier %f, MinimumGlideSpeedInv %f"), SpeedModifier, MinimumGlideSpeedInv);
	const FVector VerticalDrag = -VerticalVelocity * ChargeBasedVerticalDrag * SpeedModifier;
	const FVector SideDrag = -SideVelocity * ChargeBasedSideDrag;
	const FVector VerticalToForwardConversion = VerticalVelocity.Size() * ParentMeshTransform.GetRotation().GetForwardVector() * ChargeBasedConversionFactor * FMath::Clamp(1.f - SpeedModifier, 0.f, 1.f);

	GatheredForce += ForwardDrag + VerticalDrag + SideDrag + VerticalToForwardConversion;
}

void UMellowsMovementComponent::ForwardAccelleration()
{
	/// Accellerate to a certain forward speed
	const float ForwardVelocitySize = (ParentVelocity - FVector::VectorPlaneProject(ParentVelocity, DefaultOrientation.ForwardVector)).Size();
	if (ForwardVelocitySize < BaseTopSpeed)
	{
		GatheredForce += DefaultOrientation.ForwardVector * BaseAccelleration;
	}
}

void UMellowsMovementComponent::AddHoveringDrags()
{
	const FVector ForwardVelocity = (ParentVelocity - FVector::VectorPlaneProject(ParentVelocity, DefaultOrientation.ForwardVector));
	const FVector SideVelocity = ParentVelocity - FVector::VectorPlaneProject(ParentVelocity, DefaultOrientation.RightVector);
	const FVector ForwardDrag = -ForwardVelocity * ForwardHoverDrag;
	const FVector SideDrag = -SideVelocity * SideHoverDrag;
	GatheredForce += ForwardDrag + SideDrag;
}

void UMellowsMovementComponent::AddHoveringForce()
{
	const float HoverDelta = BaseHoverDistance - LastGroundHitResult.Distance;
	const FVector HoverForce = (((HoverDelta - PreviousHoverDelta) / ParentDeltaTime * DampingCoefficient) + (HoverDelta * SpringCoefficient)) * DefaultOrientation.UpVector;
	PreviousHoverDelta = HoverDelta;
	GatheredForce += HoverForce;
}



void UMellowsMovementComponent::AddCharge(float addedCharge)
{
	Charge = FMath::Clamp(Charge + addedCharge, 0.f, BaseMaxCharge);
}

void UMellowsMovementComponent::AddImpulse(float addedImpulse)
{
	Impulse = FMath::Clamp(Impulse + (addedImpulse * 1.5f), 0.f, BaseMaxCharge);
}

void UMellowsMovementComponent::AddGatheredForce(FVector addedForce)
{
	GatheredForce += addedForce;
}

void UMellowsMovementComponent::Multi_OnAttachFinished_Implementation(FHitResult LastHit)
{
	LastGroundHitResult = LastHit;
}

FVector UMellowsMovementComponent::ConsumeGatheredForce()
{
	const FVector TemporaryReturnForce = GatheredForce;
	GatheredForce = FVector(0.f);
	return TemporaryReturnForce;
}

void UMellowsMovementComponent::HandlePitch(float Val)
{
	const float axisInputValue = bIsInverted ? -Val : Val;
	const float DotForwardVec = FVector::DotProduct(DefaultOrientation.UpVector, ParentMeshTransform.GetRotation().GetForwardVector());

	const bool IsLookingUp = DotForwardVec < 0.f ? true : false;

	float ForwardAngle = FQuat::FindBetweenNormals(DefaultOrientation.ForwardVector, ParentMeshTransform.GetRotation().GetForwardVector()).GetAngle() * (DotForwardVec < 0.f ? -1.f : 1.f);
	const bool bIsPitching = FMath::Abs(Val) > 0.1f;
	const bool bIsTakeoffAngle = (ForwardAngle > MaxHoverPitch* PI* TakeofPitch);
	const bool bIsTakeoffSpeed = (ForwardSpeed > TakeofSpeed* INVERSE_SPEED_MULTIPLIER);
	bAllowHovering = bIsCharging || !bIsTakeoffSpeed || !(bIsPitching && bIsTakeoffSpeed) || !(bIsTakeoffAngle && bIsTakeoffSpeed);

	//if (!bAllowHovering)
	//	GroundCheckMultiplier = 1.f;


	float CurrentMaxPitch = 0;
	if (bIsGliding)
	{
		CurrentMaxPitch = MaxGlidePitch;
	}
	else if (bIsCharging)
	{
		CurrentMaxPitch = MaxChargePitch;
	}
	else
	{
		if (IsLookingUp)
			CurrentMaxPitch = 0.07f;
		else
			CurrentMaxPitch = MaxHoverPitch;

		//UE_LOG(LogTemp, Warning, TEXT("Forward Angle %f"), ForwardAngle);


	}


	// todo:	find out why you can still overpitch up to a angle that resets a bit down

	float  TargetPitchSpeed;

	// Keep the pitch when charging
	if (bIsCharging && !bIsGliding && ForwardAngle < CurrentMaxPitch && ForwardAngle > 0.f)
	{
		TargetPitchSpeed = axisInputValue * PitchSpeed * 1.2f;//0.5f;//1.5f;
	}
	else {

		TargetPitchSpeed = bIsPitching
			? (axisInputValue + (2 / (PI * (CurrentMaxPitch))) * ForwardAngle) * PitchSpeed
			: ForwardAngle * PitchNormalizeMultiplier;
	}

	CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, TargetPitchSpeed, GetWorld()->GetDeltaSeconds(), 20.f);
}

void UMellowsMovementComponent::HandleTurn(float Val)
{
	const float DotRightVec = FVector::DotProduct(DefaultOrientation.UpVector, ParentMeshTransform.GetRotation().GetRightVector());
	//const float DotForwardVec = FVector::DotProduct(DefaultOrientation.UpVector, ParentMeshTransform.GetRotation().GetForwardVector());
	const float RightAngle = FQuat::FindBetweenNormals(DefaultOrientation.RightVector, ParentMeshTransform.GetRotation().GetRightVector()).GetAngle() * (DotRightVec < 0.f ? -1.f : 1.f);
	const float ForwardAngle = FQuat::FindBetweenNormals(DefaultOrientation.ForwardVector, ParentMeshTransform.GetRotation().GetForwardVector()).GetAngle();

	const bool bIsTurning = FMath::Abs(Val) > 0.1f;
	float CurrentMaxRoll = bIsGliding ? MaxGlideRoll : MaxHoverRoll;
	float RollMultiplier = 1; // FMath::Clamp(1 - ForwardAngle * PI / 2, 0.f, CurrentMaxRoll);
	//UE_LOG(LogTemp, Warning, TEXT("Angle %f"), RightAngle);

	bIsTurningLeft = DotRightVec < 0.f ? false : true;

	const float TargetRollSpeed = bIsTurning
		? (Val + (2 / (PI * (CurrentMaxRoll))) * RightAngle) * RollSpeed * RollMultiplier
		: RightAngle * RollNormalizeMultiplier;


	CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, TargetRollSpeed, GetWorld()->GetDeltaSeconds(), 20.f);
	//CurrentRollSpeed = TargetRollSpeed;
	float TargetYawSpeed;
	if (!bIsGliding)
	{
		TargetYawSpeed = (Val) * (bIsCharging ? ChargeTurnSpeed : BaseTurnSpeed) * 1.72f;
	}
	else
	{
		TargetYawSpeed = (Val) * (bIsCharging ? ChargeTurnSpeed * 1.32f : BaseTurnSpeed);
	}

	CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, TargetYawSpeed, GetWorld()->GetDeltaSeconds(), 20.f);
}

void UMellowsMovementComponent::HandleCharge(float Val)
{
	const bool bIsChargingTmp = Val > 0.1f;
	if (!bDisableCharging)
	{
		if (bIsCharging != bIsChargingTmp)
		{
			UsedCharge = 0;
		}
		SetCharging(bIsChargingTmp);
		ChargingStrength = Val;
	}
	else if (bIsChargingTmp == false)
	{
		bDisableCharging = false;
	}
	if (bIsChargingTmp)
	{
		bIsAttaching = false;
	}
}

void UMellowsMovementComponent::HandleBoost(float Val)
{
	//UE_LOG(LogTemp, Warning, TEXT("HandleBoost %s Value %f"), *GetOwner()->GetName(), Val);
	const bool bIsBoostingTmp = Val > 0.1f;
	if (!bDisableBoosting)
	{
		if (bIsBoosting != bIsBoostingTmp)
		{
			UsedCharge = 0;
		}
		if (bIsBoosting)
		{
			SetBoosting(bIsBoostingTmp);
		}
		else if (Charge > 0.0f)
		{
			SetBoosting(bIsBoostingTmp);
		}
		else
		{
			bDisableBoosting = true;
			SetBoosting(false);
		}

		BoostingStrength = Val;
	}
	else if (Charge >= BaseMaxCharge * 0.90f)
	{
		bDisableBoosting = false;
	}
	if (bIsBoostingTmp)
	{
		bIsAttaching = false;
	}

}

void UMellowsMovementComponent::StartBoost()
{
	SetBoosting(true);
	UsedCharge = 0;

}

void UMellowsMovementComponent::StopBoost()
{
	SetBoosting(false);
	UsedCharge = 0;
}

void UMellowsMovementComponent::StartCharge()
{
	SetCharging(true);
}

void UMellowsMovementComponent::StopCharge()
{
	SetCharging(false);
}

// todo: attach and impulse
void UMellowsMovementComponent::StartImpulse()
{
	bIsLaunching = true;
	bIsAttaching = false;
}

void UMellowsMovementComponent::StopImpulse()
{
	bIsLaunching = false;

}

void UMellowsMovementComponent::Multi_DebugLine_Implementation(FVector start, FVector end)
{
	DrawDebugLine(GetWorld(), start, end, FColor::Red, false, 0.1f, 0.f, 10.f);
}

void UMellowsMovementComponent::SetCurrentGroundCheckMultiplier(float Value)
{
	CurrentGroundCheckMultiplier = Value;
}

void UMellowsMovementComponent::SetBoosting(bool isBoosting)
{
	if ((GetNetMode() == ENetMode::NM_DedicatedServer) && ((isBoosting != bIsBoosting) || bDisableBoosting))
	{
		bIsBoosting = isBoosting;
		Multi_BoostChanged(bIsBoosting, bDisableBoosting);
	}
}

void UMellowsMovementComponent::SetCharging(bool isCharging)
{
	if ((GetNetMode() == ENetMode::NM_DedicatedServer) && (isCharging != bIsCharging))
	{
		bIsCharging = isCharging;
		Multi_ChargeChanged(bIsCharging);
	}
}

void UMellowsMovementComponent::SetGliding(bool isGliding)
{
	if ((GetNetMode() == ENetMode::NM_DedicatedServer) && (isGliding != bIsGliding))
	{
		bIsGliding = isGliding;
		Multi_GlideChanged(bIsGliding, LastGroundHitResult);
	}
}

void UMellowsMovementComponent::Multi_BoostChanged_Implementation(bool isBoosting, bool boostDisabled)
{
	if (GetNetMode() != ENetMode::NM_DedicatedServer)
	{

		//UE_LOG(LogTemp, Warning, TEXT("BoostChanged"));
		bIsBoosting = isBoosting;
		bDisableBoosting = boostDisabled;
		AMellowsPawn* Owner = Cast<AMellowsPawn>(GetOwner());
		if (Owner)
		{
			Owner->BoostParticles->SetActive(bIsBoosting, true);
			if (GetOwnerRole() == ROLE_AutonomousProxy)
			{
				BoostFeedback(Owner);
			}
		}
	}
}

void UMellowsMovementComponent::Multi_ChargeChanged_Implementation(bool isCharging)
{
	if (GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		//UE_LOG(LogTemp, Warning, TEXT("ChargeChanged"));
		bIsCharging = isCharging;
		AMellowsPawn* Owner = Cast<AMellowsPawn>(GetOwner());
		if (Owner)
		{
			bIsFullyCharged = (Impulse >= BaseMaxCharge);
			Owner->GoundChargeParticles->SetActive(!bIsGliding && !bIsFullyCharged, true);
			Owner->AirChargeParticles->SetActive(bIsCharging && bIsGliding, true);
			Owner->BoostParticles->SetActive(!bIsCharging && bIsBoosting, true);
			if (GetOwnerRole() == ROLE_AutonomousProxy)
			{
				BoostFeedback(Owner);
			}
		}
	}
}

void UMellowsMovementComponent::BoostFeedback(AMellowsPawn* Owner)
{
	if (!Owner->IsValidLowLevel())
	{
		return;
	}

	if (!bIsCharging)
	{
		if (bIsBoosting)
		{
			if (!BoostSoundComp->IsValidLowLevel())
			{
				BoostSoundComp = UGameplayStatics::CreateSound2D(GetWorld(), BoostSound);
			}
			if (BoostSoundComp->IsValidLowLevel())
			{
				if (!GetWorld()->GetTimerManager().IsTimerActive(BoostInitTimerHandle))
				{
					GetWorld()->GetTimerManager().SetTimer(BoostInitTimerHandle, 0.3f, false, -1.f);

					if (!BoostInitSoundComp->IsValidLowLevel())
					{
						BoostInitSoundComp = UGameplayStatics::CreateSound2D(GetWorld(), BoostInitSound);
					}
					if (BoostInitSoundComp->IsValidLowLevel())
					{
						BoostInitSoundComp->FadeIn(0.2f, 0.6f);
					}
				}

				BoostSoundComp->FadeIn(1.f);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("BoostSoundComp Could not be made"));
			}
		}
		else
		{
			if (bDisableBoosting && !bIsCharging)
			{
				UGameplayStatics::PlaySound2D(GetWorld(), BoostOverheatSound, 2.f);
				Owner->PlayOverheatForceFeedback();
			}
			if (BoostSoundComp->IsValidLowLevel())
			{
				BoostSoundComp->FadeOut(0.5f, 0.f);
				BoostSoundComp->StopDelayed(0.5f);
			}
			if (BoostInitSoundComp->IsValidLowLevel())
			{
				BoostInitSoundComp->FadeOut(0.4f, 0.f);
				BoostInitSoundComp->StopDelayed(0.4f);
			}
			Owner->StopBoostForceFeedback();
		}
	}
	else
	{
		if (BoostSoundComp->IsValidLowLevel())
		{
			BoostSoundComp->FadeOut(0.5f, 0.f);
			BoostSoundComp->StopDelayed(0.5f);
		}
		if (BoostInitSoundComp->IsValidLowLevel())
		{
			BoostInitSoundComp->FadeOut(0.4f, 0.f);
			BoostInitSoundComp->StopDelayed(0.4f);
		}
		UE_LOG(LogTemp, Warning, TEXT("ChargeChanged"));
		Owner->StopBoostForceFeedback();
	}
}

void UMellowsMovementComponent::Multi_GlideChanged_Implementation(bool isGliding, FHitResult lastHit)
{
	if (GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		//UE_LOG(LogTemp, Warning, TEXT("GlideChanged"));
		bIsGliding = isGliding;
		LastGroundHitResult = lastHit;
		AMellowsPawn* Owner = Cast<AMellowsPawn>(GetOwner());
		if (Owner)
		{
			bIsFullyCharged = (Impulse >= BaseMaxCharge);
			Owner->GoundChargeParticles->SetActive(!bIsGliding && !bIsFullyCharged);
			Owner->AirChargeParticles->SetActive(bIsCharging && bIsGliding);
			Owner->HoverParticles->SetActive(!bIsGliding);
			Owner->GlideParticlesLeft->SetActive(bIsGliding);
			Owner->GlideParticlesRight->SetActive(bIsGliding);
		}
	}

}


