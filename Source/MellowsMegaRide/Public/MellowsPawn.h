// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Runtime/Engine/Classes/PhysicsEngine/BodyInstance.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "FollowingActor.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/Border.h"
#include "MellowsMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "MellowsPlayerState.h"
//#include "CableComponent.h"


#include "MellowsPawn.generated.h"


class UAudioComponent;

USTRUCT()
struct FColorGliderParts
{
	GENERATED_BODY()

		UPROPERTY()
		uint32 BodyColor = 0;

	UPROPERTY()
		uint32 WingColor = 4;

	UPROPERTY()
		uint32 JetColor = 6;

	UPROPERTY()
		uint32 InnerPartColor = 7;

	UPROPERTY()
		uint32 InteriorColor = 8;

	UPROPERTY()
		uint32 HingeColor = 9;

	UPROPERTY()
		uint32 PlateColor = 10;
};

UCLASS()
class MELLOWSMEGARIDE_API AMellowsPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMellowsPawn();



private:

	

	/// Setup functions
	void SetupDefaultActorParameters();
	void SetupMesh();
	void SetupCamera();
	void SetupCollisionSpheres();
	void SetupParticleSystems();

	void SetupGliderUI();

	void SetupMaterials();

	int InvertedPitchMultiplier = 1;
	int InvertedCameraPitchMultiplier = 1;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void UpdateTargetingCountdown();

	void CheckOrbsTargetedByOtherPlayers();

	void CheckForGrappleTarget();

	UFUNCTION(BlueprintCallable, Category = "Controls")
	void SetInvertedPitchMultiplier(bool Inverted);
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void SetInvertedCameraPitchMultiplier(bool Inverted);

	UFUNCTION(BlueprintCallable, Category = "Controls")
	bool IsPitchInverted();
	UFUNCTION(BlueprintCallable, Category = "Controls")
	bool IsCameraPitchInverted();

	UFUNCTION()
		void StopTargeting();
	UFUNCTION()
		void StartTargeting();

	AFollowingActor* GetNearestOrbInCone();

	void UpdateClientProperties(float DeltaTime);

	void PlayTargetingSounds();

	void RotatePawnToInput(float DeltaTime);
	void ExecuteAttachment(float DeltaTime);
	void AddGrapplePullForce();
	void ShootGrappleHook();
	void UpdateServerStateVariables();

	UCameraComponent* GetActiveCamera();


	/// RAMMING
	UFUNCTION()
		void OnDamageSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser);

	/// PICKUP
	UFUNCTION()
		void OnPickupSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	AMellowsPawn* GetOwningPawn(AFollowingActor* overlappedFollowingActor);

	UFUNCTION()
		void UpdateFollowingActorsCount();

	// Add replicated properties here
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	void SetStopped(bool newStopped);

	UFUNCTION(NetMulticast, reliable)
		void Multi_SetSimulatePhysics(bool Val);
	void Multi_SetSimulatePhysics_Implementation(bool Val);

	UFUNCTION(NetMulticast, reliable)
		void Multi_UpdateFollowerReference(AFollowingActor* firstFollower);
	void Multi_UpdateFollowerReference_Implementation(AFollowingActor* firstFollower);

	UFUNCTION(NetMulticast, reliable)
		void Multi_StartImpulseParticles();
	void Multi_StartImpulseParticles_Implementation();


	UFUNCTION()
		virtual void OnRep_VelocityChanged();
	UFUNCTION()
		virtual void OnRep_LocationChanged();
	UFUNCTION()
		virtual void OnRep_RotationChanged();




	/// INPUT FUNCTIONS

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Client, reliable)
		void Client_SwapTargetForParent(AFollowingActor* OldActorTarget, AFollowingActor* NewActorTarget);
	void Client_SwapTargetForParent_Implementation(AFollowingActor* OldActorTarget, AFollowingActor* NewActorTarget);

	UFUNCTION(Client, reliable)
		void Client_UpdateTargetForPawn(AMellowsPawn* ParentPawn, int index);
	void Client_UpdateTargetForPawn_Implementation(AMellowsPawn* ParentPawn, int index);

	UFUNCTION(Client, reliable)
		void Client_StopTargetingPawn(AMellowsPawn* ParentPawn);
	void Client_StopTargetingPawn_Implementation(AMellowsPawn* ParentPawn);

	UFUNCTION(Client, reliable)
		void Client_PlaySound(USoundBase * GrappleSound);
	void Client_PlaySound_Implementation(USoundBase * GrappleSound);


	// CALL PLAYER STOLE BALLS MESSAGE FROM SERVER
	UFUNCTION(Client, reliable)
		void Client_LostOrbsMessage(const FString& thiefName, const uint32 stolenOrbsAmount);
	void Client_LostOrbsMessage_Implementation(const FString& thiefName, const uint32 stolenOrbsAmount);

	// CALL PLAYER STOLE BALLS MESSAGE FROM SERVER
	UFUNCTION(Client, reliable)
		void Client_StoleOrbsMessage(const FString& bestolenName, const uint32 stolenOrbsAmount);
	void Client_StoleOrbsMessage_Implementation(const FString& bestolenName, const uint32 stolenOrbsAmount);

	// CALL PLAYER DELIVERED ORBS MESSAGE FROM SERVER
	UFUNCTION(Client, reliable)
		void Client_DeliveredOrbsMessage(int deliveredPoints);
	void Client_DeliveredOrbsMessage_Implementation(int deliveredPoints);

	UFUNCTION(Client, reliable)
		void Client_PickedUpImpulseMessage();
	void Client_PickedUpImpulseMessage_Implementation();

	UFUNCTION(Client, reliable)
		void Client_CollectedFreeOrbMessage();
	void Client_CollectedFreeOrbMessage_Implementation();





	// TIMERHANDLE FOR HUD MESSAGES
	FTimerHandle PickupScreenTimerHandle;
	FTimerHandle MessageTimerHandle;
	FTimerHandle TargetTimerHandle;
	FTimerHandle LingerTimerHandle;
	FTimerHandle InvinibilityTimerHandle;
	// FUNCTION TO HIDE MESSAGE
	UFUNCTION()
		void hideHUDMessage();


	/// CHARGING

	void Charge_Input(float Val);
	UFUNCTION(Server, unreliable, WithValidation)
		void Server_Charge(float Val);
	void Server_Charge_Implementation(float Val);
	bool Server_Charge_Validate(float Val);
	void ChargeUp(float Val);


	void Boost_Input(float Val);
	UFUNCTION(Server, unreliable, WithValidation)
		void Server_Boost(float Val);
	void Server_Boost_Implementation(float Val);
	bool Server_Boost_Validate(float Val);
	void Boost(float Val);

	/// ATTACHING
	void AttachPressed_Input();
	void AttachReleased_Input();
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_AttachPressed();
	void Server_AttachPressed_Implementation();
	bool Server_AttachPressed_Validate();
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_AttachReleased();
	void Server_AttachReleased_Implementation();
	bool Server_AttachReleased_Validate();
	void AttachPressed();
	void AttachReleased();


	/// IMPULSE
	void ImpulsePressed_Input();
	void ImpulseReleased_Input();
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_ImpulsePressed();
	void Server_ImpulsePressed_Implementation();
	bool Server_ImpulsePressed_Validate();
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_ImpulseReleased();
	void Server_ImpulseReleased_Implementation();
	bool Server_ImpulseReleased_Validate();
	void ImpulsePressed();
	void ImpulseReleased();

	/// LOOK BACK
	void LookBackPressed();
	void LookBackReleased();



	/// ROTATE
	void Pitch_Input(float Val);
	void Turn_Input(float Val);
	UFUNCTION(Server, unreliable, WithValidation)
		void Server_Pitch(float Val);
	void Server_Pitch_Implementation(float Val);
	bool Server_Pitch_Validate(float Val);
	UFUNCTION(Server, unreliable, WithValidation)
		void Server_Turn(float Val);
	void Server_Turn_Implementation(float Val);
	bool Server_Turn_Validate(float Val);
	void Pitch(float Val);
	void Turn(float Val);


	void Camera_Pitch_Input(float Val);
	void Camera_Turn_Input(float Val);
	UFUNCTION(Server, unreliable, WithValidation)
		void Server_Camera_Pitch(float Val);
	void Server_Camera_Pitch_Implementation(float Val);
	bool Server_Camera_Pitch_Validate(float Val);
	UFUNCTION(Server, unreliable, WithValidation)
		void Server_Camera_Turn(float Val);
	void Server_Camera_Turn_Implementation(float Val);
	bool Server_Camera_Turn_Validate(float Val);
	void Camera_Pitch(float Val);
	void Camera_Turn(float Val);

	UFUNCTION()
		void OnRep_ColorSchemeChange();

	UFUNCTION()
		void OnRep_ColorChange();

	void ChangeGliderPartsColor();

	void PlayOverheatForceFeedback();
	void StopBoostForceFeedback();

	void PlaySphereHitRumble();


	void StartInvincibility(float invincibilityTime);

	UFUNCTION()
		void InvincibilityTimeout();

	UFUNCTION(NetMulticast, reliable)
		void Multi_SetChainVisibility(bool bIsVisible);
	void Multi_SetChainVisibility_Implementation(bool bIsVisible);

	UFUNCTION(Client, reliable)
		void Client_SetChainVisibility(bool bIsVisible);
	void Client_SetChainVisibility_Implementation(bool bIsVisible);

private:

	bool bIsInvincible = false;

	void CallPlaySound(USoundBase* Sound);

public:
	/// COMPONENTS

	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class USkeletalMeshComponent* PlaneMesh;
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* GrappleHook;
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* TargetingCone;
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* LaserTargetPoint;

	// SETTINGS FOR CAMERA AND SPRING ARMS
	// GLIDE LAGS

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float GlideLagSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float GlideRotationLagSpeed = 3.75f;// 4.0f;// 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float GlideChargingRotationLagSpeed = 6.0f;

	// HOVER LAGS

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float HoverLagSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float HoverRotationLagSpeed = 9.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float HoverChargeRotationLagSpeed = 300.0f;

	// CAMERA PITCH, YAW WHILE MOVING WITHOUT CAMERA STICK
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float MaxCameraTurnAngle = 17.5f;// 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
		float CurrentCameraTurnAngle = 0.f;


	// ORB SCREEN USERWIDGET
	//UPROPERTY(Category = UserInterface, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	//	class UUserWidget* OrbWidget;


	// FIRST CAMERA
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* SpringArm;
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* Camera;

	// SECOND CAMERA
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* SpringArmLookBack;
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* CameraLookBack;

	// todo not sure if needed
	UPROPERTY(Category = Ramming, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class USphereComponent* DamageSphere;
	UPROPERTY(Category = Collecting, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class USphereComponent* PickupSphere;
	UPROPERTY(Category = Collecting, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class USphereComponent* GrapplePickupSphere;

	UPROPERTY(Category = Particles, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UParticleSystemComponent* BoostParticles;
	UPROPERTY(Category = Particles, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UParticleSystemComponent* GoundChargeParticles;
	UPROPERTY(Category = Particles, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UParticleSystemComponent* AirChargeParticles;
	UPROPERTY(Category = Particles, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UParticleSystemComponent* GlideParticlesLeft;
	UPROPERTY(Category = Particles, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UParticleSystemComponent* GlideParticlesRight;
	UPROPERTY(Category = Particles, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UParticleSystemComponent* HoverParticles;
	UPROPERTY(Category = Particles, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UParticleSystemComponent* GrappleParticles;
	UPROPERTY(Category = Particles, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UParticleSystemComponent* ImpulseParticles;
	UPROPERTY(Category = Particles, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UParticleSystemComponent* LaserParticles;

	UPROPERTY(Category = Audio, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* GlidingWindAudio;

	UPROPERTY(Category = Movement, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Replicated)
		class UMellowsMovementComponent* MovementComponent;


	UPROPERTY(Category = NetworkInterpolation, EditAnywhere)
		float NetworkLocationInterpSpeed = 8.f;
	UPROPERTY(Category = NetworkInterpolation, EditAnywhere)
		float NetworkVelocityInterpSpeed = 8.f;
	UPROPERTY(Category = NetworkInterpolation, EditAnywhere)
		float NetworkRotationInterpSpeed = 15.f;
	UPROPERTY(Category = NetworkInterpolation, EditAnywhere)
		float LocationSnapDistance = 500.f;
	UPROPERTY(Category = NetworkInterpolation, EditAnywhere)
		float VelocitySnapDistance = 500.f;


	UPROPERTY(Category = Attachment, EditAnywhere)
		float AttachLocationInterpSpeed = 5.f;
	UPROPERTY(Category = Attachment, EditAnywhere)
		float AttachRotationInterpSpeed = 5.f;
	// When the hookshot switches from adding force to attaching
	UPROPERTY(Category = Attachment, EditAnywhere)
		float AttachmentTransitionRange = 300.f;
	UPROPERTY(Category = Attachment, EditAnywhere)
		float HookshotMaxRange = 15000.f;
	UPROPERTY(Category = Attachment, EditAnywhere)
		float HookshotSpeed = 30000.0f;
	UPROPERTY(Category = Attachment, EditAnywhere)
		float PullForceMultiplier = 100.f;

	TMap<AMellowsPawn*, float> TargetingPawnsMap;
	float CollectingTargetStartTime = 0.f;
	bool bCanStartResetTimer = true;
	UPROPERTY(Category = Targeting, EditAnywhere)
		float LingeringTime = 1.f;
	UPROPERTY(Category = Targeting, EditAnywhere)
		float OrbsPerSeconds = 1.f;
	UPROPERTY(Category = Targeting, EditAnywhere)
		float OrbsPerSecondsCollecting = 2.f;
	float InvOrbsPerSecondsCollecting = 0.5f;
	UPROPERTY(Category = Targeting, EditAnywhere)
		float InvincibilityTimeStolen = 2.f;
	UPROPERTY(Category = Targeting, EditAnywhere)
		float InvincibilityTimeSteal = 0.2f;

	UPROPERTY(BlueprintReadOnly, Replicated)
		bool bIsTargeted = false;
	UPROPERTY(BlueprintReadOnly, Replicated)
		int TargetingOrbs = 0;
	UPROPERTY(BlueprintReadOnly, Replicated)
		int LingeringOrbs = 0;
	UPROPERTY(BlueprintReadOnly, Replicated)
		bool bAllOrbsTargeted = false;
	UPROPERTY(BlueprintReadOnly, Replicated)
		float TargetingCountdown = 0;
	UPROPERTY(BlueprintReadOnly, Replicated)
		float LingeringCountdown = 0;
	UPROPERTY(BlueprintReadOnly, Replicated)
		bool bShootHook = false;
	UPROPERTY(BlueprintReadOnly, Replicated)
		bool bHookEnabled = true;
	UPROPERTY(BlueprintReadOnly, Replicated)
		FTransform AttachmentTargetTransform;
	UPROPERTY(BlueprintReadOnly, Replicated)
		FVector GrappleLocationTarget;
	UPROPERTY(BlueprintReadOnly, Replicated)
		AFollowingActor* GrappleOrbTarget;
	UPROPERTY(BlueprintReadOnly, Replicated)
		AFollowingActor* GrappleOwnedOrbTarget;
	UPROPERTY(BlueprintReadOnly, Replicated)
		AFollowingActor* ShotGrappleOrbTarget;

	bool bPlayTargetingSound = true;
	bool bPlayAlertSound = true;

	UPROPERTY(BlueprintReadOnly, Replicated)
		AFollowingActor* GrapplePreLockOnOrb;
	UPROPERTY(BlueprintReadOnly, Replicated)
		AFollowingActor* OrbAboutToSteal;

	UPROPERTY(BlueprintReadOnly, Replicated)
		bool bIsStopped = true;

	FVector GrappleLocalDelta;

	UPROPERTY()
		FColor TrailColor = FColor::White;



	// CAMERA VARIABLES
	UPROPERTY(BlueprintReadOnly)
		float CameraYawValue = 0.f;
	UPROPERTY(BlueprintReadOnly)
		float CameraPitchValue = 0.f;

	UPROPERTY(BlueprintReadOnly)
		bool bCameraLookBack = false;

	// Clientside Interpolation
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_LocationChanged)
		FVector ServerLocationTarget;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_VelocityChanged)
		FVector ServerVelocityTarget;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_RotationChanged)
		FRotator ServerRotationTarget;

	/// Postprocessing 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
		UMaterial* SpeedLinesMaterial;
	UMaterialInstanceDynamic* SpeedLinesMaterial_Dyn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
		UMaterial* TargetedWarningMaterial;
	UMaterialInstanceDynamic* TargetedWarningMaterial_Dyn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
		float SpeedLinesThreshold = 1700.0f;

	UPROPERTY(ReplicatedUsing = OnRep_ColorChange)
		FColorGliderParts ColorGliderParts;

	UPROPERTY(BlueprintReadWrite)
	UMaterialInstanceDynamic* CustomMaterial_Dyn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
		TArray<FLinearColor> GliderPartColors;

	UPROPERTY(Replicated)
		AFollowingActor* FirstFollowingActor = nullptr;
	
	UPROPERTY(EditAnywhere, Category = Sounds)
		USoundBase* GrappleSound;
	UPROPERTY(EditAnywhere, Category = Sounds)
		USoundBase* StealSound;
	UPROPERTY(EditAnywhere, Category = Sounds)
		USoundBase* StolenSound;
	UPROPERTY(EditAnywhere, Category = Sounds)
		USoundBase* PickupSound;
	UPROPERTY(EditAnywhere, Category = Sounds)
		USoundBase* DeliverSound;
	UPROPERTY(EditAnywhere, Category = Sounds)
		USoundBase* TargetingSound;
	UAudioComponent* TargetingSoundComp;
	UPROPERTY(EditAnywhere, Category = Sounds)
		USoundBase* AlertSound;
	UAudioComponent* AlertSoundComp;

	UPROPERTY(EditAnywhere, Category = ForceFeedback)
		UForceFeedbackEffect* ImpulseRumble;

	UPROPERTY(EditAnywhere, Category = ForceFeedback)
		UForceFeedbackEffect* OverheatRumble;

	UPROPERTY(EditAnywhere, Category = ForceFeedback)
		UForceFeedbackEffect* GrappleRumble;

	UPROPERTY(EditAnywhere, Category = ForceFeedback)
	UForceFeedbackEffect* HitRumble;

	FDynamicForceFeedbackHandle BoostRumbleHandle;

	bool bSwitchedToOverheatWarningRumble = false;
};
