// Fill out your copyright notice in the Description page of Project Settings.

// has to be first
#include "MellowsPawn.h"

#include "Engine/GameEngine.h"
#include "UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "ChargePickup.h"
#include "Audio.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "MellowsGliderDisplayUIWidget.h"

#include "MellowsPlayerState.h"
#include "MelllowsGameHUD.h"
#include "MellowsGameInstance.h"

//#include "Kismet/KismetMathLibrary.h"

#define COLLISION_HOVER			ECC_GameTraceChannel1
#define COLLISION_PICKUP        ECC_GameTraceChannel2
#define COLLISION_DAMAGE        ECC_GameTraceChannel3

// Sets default values
AMellowsPawn::AMellowsPawn()
{

	SetupDefaultActorParameters();

	SetupMesh();

	SetupCamera();

	SetupCollisionSpheres();

	SetupParticleSystems();

	//GrappleCable->SetAttachEndToComponent(GrappleHook);

	GlidingWindAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("GlidingWindAudio"));

	MovementComponent = CreateDefaultSubobject<UMellowsMovementComponent>(TEXT("MovementComponent0"));
	MovementComponent->SetIsReplicated(true);
}

void AMellowsPawn::SetupGliderUI()
{

}

void AMellowsPawn::SetupMaterials()
{
	if (SpeedLinesMaterial != NULL)
	{
		SpeedLinesMaterial_Dyn = UMaterialInstanceDynamic::Create(SpeedLinesMaterial, this);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Set SpeedlinesMaterial in MellowsPawn!"));
	}

	if (TargetedWarningMaterial != NULL)
	{
		TargetedWarningMaterial_Dyn = UMaterialInstanceDynamic::Create(TargetedWarningMaterial, this);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Set TargetedWarningMaterial in MellowsPawn!"));
	}

	CustomMaterial_Dyn = UMaterialInstanceDynamic::Create(PlaneMesh->GetMaterial(0), this);
	PlaneMesh->SetMaterial(0, CustomMaterial_Dyn);
	ChangeGliderPartsColor();

	if (Role == ROLE_SimulatedProxy)
	{
		UE_LOG(LogTemp, Log, TEXT("Setup Simulated Proxy"));

		PlaneMesh->SetSimulatePhysics(false);
		SetActorTickInterval(0.f);
		LocationSnapDistance *= 10.f;
	}

	Camera->AddOrUpdateBlendable(SpeedLinesMaterial_Dyn);
	Camera->AddOrUpdateBlendable(TargetedWarningMaterial_Dyn);
}

void AMellowsPawn::StopTargeting()
{
	AMellowsPawn* OldTargetPawn = GetOwningPawn(GrappleOrbTarget);
	if (OldTargetPawn)
	{
		OldTargetPawn->bIsTargeted = false;
		OldTargetPawn->TargetingPawnsMap.Remove(this);
		Client_StopTargetingPawn(OldTargetPawn);
	}
	Client_SwapTargetForParent(GrappleOrbTarget, nullptr);
	GrappleOrbTarget = nullptr;
	bCanStartResetTimer = false;



	UE_LOG(LogTemp, Warning, TEXT("ResetTarget"));
	//LingeringGrappleActorTarget = nullptr;
	//CheckForGrappleTarget();
}

void AMellowsPawn::SetupDefaultActorParameters()
{
	bAlwaysRelevant = true;
	bReplicates = true;
	bReplicateMovement = false;
	PrimaryActorTick.bCanEverTick = true;
	if (OrbsPerSecondsCollecting > KINDA_SMALL_NUMBER)
	{
		InvOrbsPerSecondsCollecting = 1 / OrbsPerSecondsCollecting;
	}
	else
	{
		InvOrbsPerSecondsCollecting = 0;
		OrbsPerSecondsCollecting = 0;
	}


	NetUpdateFrequency = 100.f;
	MinNetUpdateFrequency = 20.f;


	SetActorTickInterval(0.016666f);
}

void AMellowsPawn::SetupMesh()
{
	/// Structs to hold one time Mesh initializations 
	struct FConstructorStaticsPlane
	{
		ConstructorHelpers::FObjectFinderOptional<USkeletalMesh> Mesh;
		FConstructorStaticsPlane() : Mesh(TEXT("SkeletalMesh'/Game/Vehicles/Speeder/SK_armature.SK_armature'")) {}
	}PlaneConstructorStatics;
	struct FConstructorStaticsSphere
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> Mesh;
		FConstructorStaticsSphere() : Mesh(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'")) {}
	}SphereConstructorStatics;

	struct FConstructorStaticsGrapple
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> Mesh;
		FConstructorStaticsGrapple() : Mesh(TEXT("StaticMesh'/Game/Vehicles/GrappleHook/SM_GrappleHook.SM_GrappleHook'")) {}
	}GrappleConstructorStatics;
	struct FConstructorStaticsCone
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> Mesh;
		FConstructorStaticsCone() : Mesh(TEXT("StaticMesh'/Game/Vehicles/GrappleHook/SM_Cone.SM_Cone'")) {}
	}ConeConstructorStatics;


	/// Initialize objects and set default parameters

	PlaneMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PlaneMesh0"));
	PlaneMesh->SetSkeletalMesh(PlaneConstructorStatics.Mesh.Get());
	if (!SetRootComponent(PlaneMesh)) { UE_LOG(LogTemp, Error, TEXT("Failed to set RootComponent")); }
	PlaneMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	PlaneMesh->SetCollisionResponseToChannel(COLLISION_HOVER, ECollisionResponse::ECR_Ignore);
	PlaneMesh->SetCollisionResponseToChannel(COLLISION_DAMAGE, ECollisionResponse::ECR_Ignore);
	PlaneMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	PlaneMesh->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	PlaneMesh->SetAngularDamping(400.f);
	PlaneMesh->SetLinearDamping(0.f);
	PlaneMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	PlaneMesh->SetSimulatePhysics(true);
	PlaneMesh->SetIsReplicated(false);
	PlaneMesh->SetUseCCD(true);
	PlaneMesh->SetEnableGravity(false);

	TargetingCone = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetingCone0"));
	TargetingCone->SetStaticMesh(ConeConstructorStatics.Mesh.Get());
	TargetingCone->SetCollisionObjectType(COLLISION_PICKUP);
	TargetingCone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TargetingCone->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	TargetingCone->SetCollisionResponseToChannel(COLLISION_PICKUP, ECollisionResponse::ECR_Overlap);
	TargetingCone->SetIsReplicated(false);
	TargetingCone->SetupAttachment(RootComponent);
	TargetingCone->SetWorldScale3D(FVector(75, 75, 150));
	TargetingCone->SetRelativeLocationAndRotation(FVector(7440.0f, 0.f, 0.f), FRotator(90.f, 0.f, 00.f));
	TargetingCone->SetVisibility(false);
	TargetingCone->bUseAttachParentBound = true;


	LaserTargetPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserTargetPoint0"));
	LaserTargetPoint->SetStaticMesh(SphereConstructorStatics.Mesh.Get());
	LaserTargetPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LaserTargetPoint->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	LaserTargetPoint->SetIsReplicated(false);
	LaserTargetPoint->SetupAttachment(RootComponent);
	LaserTargetPoint->SetWorldScale3D(FVector(5.f));
	LaserTargetPoint->SetVisibility(false);

	GrappleHook = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrappleHook0"));
	GrappleHook->SetStaticMesh(GrappleConstructorStatics.Mesh.Get());
	GrappleHook->SetupAttachment(RootComponent);
	GrappleHook->SetIsReplicated(true);
	GrappleHook->bAbsoluteLocation = true;
	GrappleHook->bAbsoluteRotation = true;
	GrappleHook->SetWorldScale3D(FVector(0.1f));
	GrappleHook->bUseAttachParentBound = true;
}

void AMellowsPawn::SetupCamera()
{
	// FIRST CAMERA SETUP
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 1000.f;// 1400.0f;//1723.0f;
	SpringArm->SocketOffset = FVector(0.f, 0.f, 200.f);// 150.f);
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritRoll = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraRotationLagSpeed = 3.f;// 7.f;
	SpringArm->CameraLagSpeed = 10.f;// 15.f;
	SpringArm->CameraLagMaxDistance = 750.f;//1000.f;
	SpringArm->bUseAttachParentBound = true;
	//SpringArm->RelativeLocation = FVector(0, 0, 0);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // Don't rotate camera with controller
	Camera->RelativeRotation.Pitch = -2.f;//-5.f;

	// SECOND CAMERA
	SpringArmLookBack = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmLookBack0"));
	SpringArmLookBack->SetupAttachment(RootComponent);
	SpringArmLookBack->TargetArmLength = 1000.f;// 1400.0f;//1723.0f;
	SpringArmLookBack->SocketOffset = FVector(0.f, 0.f, 200.f);// 150.f);
	SpringArmLookBack->bInheritPitch = true;
	SpringArmLookBack->bInheritRoll = true;
	SpringArmLookBack->bInheritYaw = true;
	SpringArmLookBack->bEnableCameraRotationLag = true;
	SpringArmLookBack->bEnableCameraLag = false;
	SpringArmLookBack->CameraRotationLagSpeed = 3.f;// 7.f;
	SpringArmLookBack->CameraLagSpeed = 10.f;// 15.f;
	SpringArmLookBack->CameraLagMaxDistance = 750.f;//1000.f;
	SpringArmLookBack->RelativeRotation.Yaw = -180.f;
	SpringArmLookBack->bUseAttachParentBound = true;

	//SpringArm->RelativeLocation = FVector(0, 0, 0);

	CameraLookBack = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraLookBack0"));
	CameraLookBack->SetupAttachment(SpringArmLookBack, USpringArmComponent::SocketName);
	CameraLookBack->bUsePawnControlRotation = false; // Don't rotate camera with controller
	CameraLookBack->RelativeRotation.Pitch = -2.f;//-5.f

}

void AMellowsPawn::SetupCollisionSpheres()
{

	DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RammingSphere0"));
	DamageSphere->SetSphereRadius(200.f);
	DamageSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageSphere->SetupAttachment(RootComponent);
	DamageSphere->OnComponentBeginOverlap.AddDynamic(this, &AMellowsPawn::OnDamageSphereBeginOverlap);

	/// Only overlap with other damagte spheres 
	DamageSphere->SetCollisionObjectType(COLLISION_DAMAGE);
	DamageSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	DamageSphere->SetCollisionResponseToChannel(COLLISION_DAMAGE, ECollisionResponse::ECR_Overlap);


	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere0"));
	PickupSphere->SetSphereRadius(200.f);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetupAttachment(RootComponent);
	PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AMellowsPawn::OnPickupSphereBeginOverlap);
	/// Only overlap with pickups
	PickupSphere->SetCollisionObjectType(COLLISION_PICKUP);
	PickupSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(COLLISION_PICKUP, ECollisionResponse::ECR_Overlap);
	PickupSphere->ComponentTags.Add("PickupSphere");


	GrapplePickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("GrapplePickupSphere0"));
	GrapplePickupSphere->SetSphereRadius(200.f);
	GrapplePickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GrapplePickupSphere->SetupAttachment(GrappleHook);
	GrapplePickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AMellowsPawn::OnPickupSphereBeginOverlap);
	/// Only overlap with pickups
	GrapplePickupSphere->SetCollisionObjectType(COLLISION_PICKUP);
	GrapplePickupSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GrapplePickupSphere->SetCollisionResponseToChannel(COLLISION_PICKUP, ECollisionResponse::ECR_Overlap);

}

void AMellowsPawn::SetupParticleSystems()
{
	static ConstructorHelpers::FObjectFinder<UParticleSystem> BoosterPS(TEXT("ParticleSystem'/Game/Effects/Boosting/PS_JetExhaust.PS_JetExhaust'"));
	BoostParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("RightBoostParticles0"));
	BoostParticles->SetTemplate(BoosterPS.Object);
	BoostParticles->SetupAttachment(RootComponent);
	BoostParticles->bAutoActivate = false;
	BoostParticles->RelativeLocation = FVector(0, 0, 0);
	BoostParticles->SetVectorParameter("LeftLocation", FVector(-40, 105, 20));
	BoostParticles->SetVectorParameter("RightLocation", FVector(-40, -105, 20));
	BoostParticles->bUseAttachParentBound = true;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> HoverPS(TEXT("ParticleSystem'/Game/Effects/Hovering/PS_Hovering.PS_Hovering'"));
	HoverParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("HoverParticles0"));
	HoverParticles->SetTemplate(HoverPS.Object);
	HoverParticles->bAutoActivate = false;
	HoverParticles->SetupAttachment(RootComponent);
	HoverParticles->bUseAttachParentBound = true;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> GlidePS(TEXT("ParticleSystem'/Game/Effects/Glide/PS_Glide.PS_Glide'"));

	GlideParticlesLeft = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("GlideParticlesLeft0"));
	GlideParticlesLeft->SetTemplate(GlidePS.Object);
	GlideParticlesLeft->bAutoActivate = false;
	GlideParticlesLeft->SetupAttachment(PlaneMesh, FName(TEXT("left_wing_outside_socket")));
	GlideParticlesLeft->bUseAttachParentBound = true;

	GlideParticlesRight = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("GlideParticlesRight0"));
	GlideParticlesRight->SetTemplate(GlidePS.Object);
	GlideParticlesRight->bAutoActivate = false;
	GlideParticlesRight->SetupAttachment(PlaneMesh, FName(TEXT("right_wing_outside_socket")));
	GlideParticlesRight->bUseAttachParentBound = true;

	GlideParticlesLeft->SetVectorParameter("PlayerColor", FVector(1.f));
	GlideParticlesRight->SetVectorParameter("PlayerColor", FVector(1.f));



	static ConstructorHelpers::FObjectFinder<UParticleSystem> GroundChargePS(TEXT("ParticleSystem'/Game/Effects/Charge/PS_Charge_Ground.PS_Charge_Ground'"));
	GoundChargeParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("GroundChargeParticles0"));
	GoundChargeParticles->SetTemplate(GroundChargePS.Object);
	GoundChargeParticles->bAutoActivate = false;
	GoundChargeParticles->bAbsoluteRotation = true;
	GoundChargeParticles->SetupAttachment(RootComponent);
	GoundChargeParticles->bUseAttachParentBound = true;
	static ConstructorHelpers::FObjectFinder<UParticleSystem> AirChargePS(TEXT("ParticleSystem'/Game/Effects/Charge/PS_Charge_Air.PS_Charge_Air'"));
	AirChargeParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("AirChargeParticles0"));
	AirChargeParticles->SetTemplate(AirChargePS.Object);
	AirChargeParticles->bAutoActivate = false;
	AirChargeParticles->bAbsoluteRotation = true;
	AirChargeParticles->SetupAttachment(RootComponent);
	AirChargeParticles->bUseAttachParentBound = true;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> GrapplePS(TEXT("ParticleSystem'/Game/Effects/Attaching/PS_LaserPointer.PS_LaserPointer'"));
	GrappleParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("GrappleParticles0"));
	GrappleParticles->SetTemplate(GrapplePS.Object);
	GrappleParticles->bAutoActivate = false;
	GrappleParticles->SetupAttachment(RootComponent);
	GrappleParticles->bUseAttachParentBound = true;
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ImpulsePS(TEXT("ParticleSystem'/Game/Effects/Impulse/PS_JetExhaust1.PS_JetExhaust1'"));
	ImpulseParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ImpulseParticles0"));
	ImpulseParticles->SetTemplate(ImpulsePS.Object);
	ImpulseParticles->bAutoActivate = false;
	ImpulseParticles->SetupAttachment(RootComponent);
	ImpulseParticles->bUseAttachParentBound = true;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> LaserPS(TEXT("ParticleSystem'/Game/Effects/Attaching/PS_LaserPointer.PS_LaserPointer'"));
	LaserParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("LaserParticles0"));
	LaserParticles->SetTemplate(LaserPS.Object);
	LaserParticles->bAutoActivate = true;
	LaserParticles->SetupAttachment(RootComponent);
	LaserParticles->bUseAttachParentBound = true;
}

// Called when the game starts or when spawned
void AMellowsPawn::BeginPlay()
{
	Super::BeginPlay();
	// Only simulate physics on the server
	if (GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		DamageSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else
	{
		DamageSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	AlertSoundComp = UGameplayStatics::CreateSound2D(GetWorld(), AlertSound, 1, 1, 0, nullptr, false, false);
	TargetingSoundComp = UGameplayStatics::CreateSound2D(GetWorld(), TargetingSound, 1, 1, 0, nullptr, false, false);

	// initialize dynamic materials
	SetupMaterials();
}

// Called every frame
void AMellowsPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	// Replication
	if (GetNetMode() == ENetMode::NM_DedicatedServer) { UpdateServerStateVariables(); }

	/// Use MovementComponent 
	MovementComponent->UpdateParentParameters(GetActorTransform(), PlaneMesh->GetPhysicsLinearVelocity(), DeltaTime);

	// only check ground and attachment on server and replicate the result 
	if (GetNetMode() == ENetMode::NM_DedicatedServer) {

		CheckForGrappleTarget();
		MovementComponent->CheckHoverableGround();
		ShootGrappleHook();
		CheckOrbsTargetedByOtherPlayers();
		UpdateTargetingCountdown();
	}

	MovementComponent->UpdateDefaultOrientation();
	if (!MovementComponent->bIsAttaching)
	{
		if (!PlaneMesh->IsSimulatingPhysics() && GetNetMode() == ENetMode::NM_DedicatedServer)
		{
			UE_LOG(LogTemp, Log, TEXT("Stop Attachment"));
			Multi_SetSimulatePhysics(true);
		}
		MovementComponent->ExecuteAppropriateMovementFunction();

	}
	else
	{
		ExecuteAttachment(DeltaTime);
		GrappleHook->SetWorldLocation(AttachmentTargetTransform.GetLocation());
		// TRY GETTING CHARGE WHEN ATTACHING TO SURFACE
		float gainedCharge = MovementComponent->BaseMaxCharge * MovementComponent->PassiveChargeGain * GetWorld()->GetDeltaSeconds() * (MovementComponent->ChargeSpeed);
		gainedCharge += MovementComponent->BaseMaxCharge * MovementComponent->ActiveChargeGain * GetWorld()->GetDeltaSeconds() * (MovementComponent->ChargeSpeed * 0.25f);
		MovementComponent->AddCharge(gainedCharge);
	}

	if (Role != ROLE_SimulatedProxy)
	{
		/// Apply calculated velocity
		if (!bIsStopped)
		{
			RotatePawnToInput(DeltaTime);
			PlaneMesh->SetPhysicsLinearVelocity(MovementComponent->ConsumeGatheredForce() * DeltaTime * 60.f, true);
			PlaneMesh->SetEnableGravity(MovementComponent->bIsGliding);
			PlaneMesh->SetPhysicsAngularVelocityInDegrees(FVector(0.f));
		}
		else
		{
			MovementComponent->ConsumeGatheredForce();
			PlaneMesh->SetPhysicsLinearVelocity(FVector(0.f));
			PlaneMesh->SetPhysicsAngularVelocityInDegrees(FVector(0.f));
		}
	}
	else
	{
		//PlaneMesh->SetPhysicsLinearVelocity(FMath::VInterpTo(PlaneMesh->GetPhysicsLinearVelocity(), ServerVelocityTarget, GetWorld()->GetDeltaSeconds(), NetworkVelocityInterpSpeed));
		SetActorLocation(FMath::VInterpTo(GetActorLocation(), ServerLocationTarget, GetWorld()->GetDeltaSeconds(), NetworkLocationInterpSpeed), false, nullptr, ETeleportType::TeleportPhysics);

		const FRotator UpdatedRot = FMath::RInterpTo(GetActorRotation(), ServerRotationTarget, GetWorld()->GetDeltaSeconds(), NetworkRotationInterpSpeed);
		SetActorRotation(UpdatedRot, ETeleportType::TeleportPhysics);
	}

	if (FirstFollowingActor) {
		FirstFollowingActor->UpdateTarget(GetActorLocation());
	}
	UpdateClientProperties(DeltaTime);
}

void AMellowsPawn::UpdateTargetingCountdown()
{



	if (GrapplePreLockOnOrb)
	{
		// check orbs targeted from other players
		AMellowsPawn* TargetPawn = GetOwningPawn(GrappleOrbTarget);
		if (TargetPawn)
		{
			float* TargetStartTime = TargetPawn->TargetingPawnsMap.Find(this);
			uint32 OrbIndexToSteal = 0;
			CollectingTargetStartTime = GetWorld()->TimeSeconds;
			if (TargetStartTime) {
				const float TimeDifference = GetWorld()->TimeSeconds - *TargetStartTime;
				const float FollowerCount = TargetPawn->FirstFollowingActor ? TargetPawn->FirstFollowingActor->FollowerCount() : 0;
				TargetingOrbs = static_cast<int>(FMath::Clamp(TimeDifference * OrbsPerSeconds, 0.f, FollowerCount - 1)) + 1;
				if (TargetingOrbs >= FollowerCount)
				{
					bAllOrbsTargeted = true;
					TargetingCountdown = 1.f;
				}
				else
				{
					bAllOrbsTargeted = false;
					TargetingCountdown = (TimeDifference - (TargetingOrbs - 1) / OrbsPerSeconds) * OrbsPerSeconds;
				}
				OrbIndexToSteal = static_cast<uint32>(FMath::Clamp(FollowerCount - TargetingOrbs, 0.f, FollowerCount));
				OrbAboutToSteal = TargetPawn->FirstFollowingActor->GetAt(OrbIndexToSteal);
				LingeringOrbs = -1;
			}
			else // this is not int the target map
			{
				UE_LOG(LogTemp, Error, TEXT("TargetStartTime null, target was collected"));
				TargetPawn->TargetingPawnsMap.Add(this, GetWorld()->TimeSeconds);
				TargetingCountdown = 0;
				TargetingOrbs = -1;
			}

		}
		else if (GetWorld()->GetTimerManager().IsTimerActive(TargetTimerHandle))
		{
			const float TimeDifference = GetWorld()->GetTimerManager().GetTimerElapsed(TargetTimerHandle);
			TargetingCountdown = FMath::Clamp(TimeDifference * OrbsPerSecondsCollecting, 0.f, 1.f);
			bAllOrbsTargeted = false;
			TargetingOrbs = 0;
			OrbAboutToSteal = nullptr;
		}
		else // no timer active
		{
			TargetingCountdown = 1.f;
			bAllOrbsTargeted = true;
			TargetingOrbs = 1;
			OrbAboutToSteal = nullptr;
		}
		if (GrapplePreLockOnOrb == GrappleOrbTarget)
		{
			LingeringOrbs = -1;
			LingeringCountdown = 1.f;
		}
		else
		{
			LingeringCountdown = 1 - TargetingCountdown;
			LingeringOrbs = 1.f;
			AMellowsPawn* PreLockPawn = GetOwningPawn(GrapplePreLockOnOrb);
			AMellowsPawn* OtherPawn = GetOwningPawn(GrappleOrbTarget);
			if (PreLockPawn && (PreLockPawn != OtherPawn))
			{
				OrbAboutToSteal = PreLockPawn->FirstFollowingActor->GetLast(2000.f);
			}
		}

	}
	else // no target in sight 
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(LingerTimerHandle))
		{
			const float TimeDifference = GetWorld()->GetTimerManager().GetTimerRemaining(LingerTimerHandle);
			LingeringCountdown = FMath::Clamp(TimeDifference * LingeringTime, 0.f, 1.f);
			TargetingCountdown = 1 - LingeringCountdown;
			bAllOrbsTargeted = false;
			LingeringOrbs = 1.f;
		}
		else
		{
			TargetingCountdown = 0;
			TargetingOrbs = -1;
			LingeringOrbs = -1.f;
		}
		OrbAboutToSteal = nullptr;
		CollectingTargetStartTime = GetWorld()->TimeSeconds;
	}
}

void AMellowsPawn::CheckOrbsTargetedByOtherPlayers()
{
	float TimeDifference = 0;
	for (auto It = TargetingPawnsMap.CreateConstIterator(); It; ++It)
	{
		TimeDifference = FMath::Max(GetWorld()->TimeSeconds - It.Value(), TimeDifference);
	}

	const float FollowerCount = FirstFollowingActor ? FirstFollowingActor->FollowerCount() - 1.f : 0;
	const uint32 TargetedOrbs = static_cast<uint32>(FMath::Clamp(TimeDifference * OrbsPerSeconds, 0.f, FollowerCount));

}

// is executed on the server
void AMellowsPawn::CheckForGrappleTarget()
{
	AFollowingActor* NearestOrb = GetNearestOrbInCone();

	if (NearestOrb) // found target
	{
		if ((NearestOrb != GrapplePreLockOnOrb)) // found new target
		{

			 const AMellowsPawn* PreLockPawn = GetOwningPawn(GrapplePreLockOnOrb);
			 AMellowsPawn* NearestPawn = GetOwningPawn(NearestOrb);
			const bool bShouldStartTimer = (!PreLockPawn && !NearestPawn) || PreLockPawn != NearestPawn;
			GrapplePreLockOnOrb = NearestOrb;
			if (bShouldStartTimer)
			{
				if (NearestPawn)
				{
					NearestPawn->bIsTargeted = true;
				}
				UE_LOG(LogTemp, Warning, TEXT("Start TargetTimer"));
				GetWorld()->GetTimerManager().SetTimer(TargetTimerHandle, this, &AMellowsPawn::StartTargeting, InvOrbsPerSecondsCollecting, false);
				GetWorld()->GetTimerManager().ClearTimer(LingerTimerHandle);
			}

		}
	}
	else // no target in sight
	{
		AMellowsPawn* NearestPawn = GetOwningPawn(GrapplePreLockOnOrb);
		if (NearestPawn)
		{
			NearestPawn->bIsTargeted = false;
		}
		GrapplePreLockOnOrb = NearestOrb;
		
		//When losing target keep last target for a time
		if (!GetWorld()->GetTimerManager().IsTimerActive(LingerTimerHandle) && GrappleOrbTarget)
		{
			UE_LOG(LogTemp, Warning, TEXT("Start LingerTimerHandle"));

			GetWorld()->GetTimerManager().SetTimer(LingerTimerHandle, this, &AMellowsPawn::StopTargeting, LingeringTime, false);
		}
	}
}

void AMellowsPawn::SetInvertedPitchMultiplier(bool Inverted)
{
	if (Inverted)
	{
		InvertedPitchMultiplier = -1;
	}
	else
	{
		InvertedPitchMultiplier = 1;
	}
}

void AMellowsPawn::SetInvertedCameraPitchMultiplier(bool Inverted)
{
	if (Inverted)
	{
		InvertedCameraPitchMultiplier = -1;
	}
	else
	{
		InvertedCameraPitchMultiplier = 1;
	}
}

bool AMellowsPawn::IsPitchInverted()
{
	if (InvertedPitchMultiplier == 1)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool AMellowsPawn::IsCameraPitchInverted()
{
	if (InvertedCameraPitchMultiplier == 1)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void AMellowsPawn::StartTargeting()
{
	// stop reset when a target is found 
	UE_LOG(LogTemp, Warning, TEXT("Start Targeting"));
	GetWorld()->GetTimerManager().ClearTimer(LingerTimerHandle);
	if (GrappleOrbTarget != GrapplePreLockOnOrb)
	{
		// check if both are from the same pawn
		AMellowsPawn* OldTargetPawn = GetOwningPawn(GrappleOrbTarget);
		AMellowsPawn* NewTargetPawn = GetOwningPawn(GrapplePreLockOnOrb);

		//When changing target check if still the same pawn
		if (OldTargetPawn != NewTargetPawn)
		{
			if (OldTargetPawn)
			{
				OldTargetPawn->bIsTargeted = false;
				OldTargetPawn->TargetingPawnsMap.Remove(this);
				Client_StopTargetingPawn(OldTargetPawn);

			}
			if (NewTargetPawn)
			{
				NewTargetPawn->bIsTargeted = true;
				NewTargetPawn->TargetingPawnsMap.Add(this, GetWorld()->TimeSeconds);
			}
			else
			{

				//update the targeting material and laser on the client
				Client_SwapTargetForParent(GrappleOrbTarget, GrapplePreLockOnOrb);
			}
		}
		else // same pawn(can be nullptr) different orb
		{
			AMellowsPawn* TargetPawn = GetOwningPawn(GrappleOrbTarget);
			if (TargetPawn)
			{
				float* TargetStartTime = TargetPawn->TargetingPawnsMap.Find(this);
				const float TimeDifference = TargetStartTime ? (GetWorld()->TimeSeconds - *TargetStartTime) : 0;
				//UE_LOG(LogTemp, Warning, TEXT("TimeDifference %f"), TimeDifference);
				const float FollowerCount = TargetPawn->FirstFollowingActor->FollowerCount();
				const uint32 OrbIndexToSteal = static_cast<uint32>(FMath::Clamp(FollowerCount - TimeDifference * OrbsPerSeconds, 0.f, FollowerCount));

				Client_UpdateTargetForPawn(TargetPawn, OrbIndexToSteal);
			}
			else
			{
				//update the targeting material and laser on the client
				Client_SwapTargetForParent(GrappleOrbTarget, GrapplePreLockOnOrb);
			}
		}
	}
	else // GrappleActorTarget == PreLockOnOrb
	{
		AMellowsPawn* TargetPawn = GetOwningPawn(GrappleOrbTarget);
		if (TargetPawn)
		{
			float* TargetStartTime = TargetPawn->TargetingPawnsMap.Find(this);
			const float TimeDifference = TargetStartTime ? (GetWorld()->TimeSeconds - *TargetStartTime) : 0;
			//UE_LOG(LogTemp, Warning, TEXT("TimeDifference %f"), TimeDifference);
			const float FollowerCount = TargetPawn->FirstFollowingActor->FollowerCount();
			const uint32 OrbIndexToSteal = static_cast<uint32>(FMath::Clamp(FollowerCount - TimeDifference * OrbsPerSeconds, 0.f, FollowerCount));

			Client_UpdateTargetForPawn(TargetPawn, OrbIndexToSteal);
		}
	}

	GrappleOrbTarget = GrapplePreLockOnOrb;
}

AFollowingActor* AMellowsPawn::GetNearestOrbInCone()
{
	TArray<AActor*> OverlappingFolloingActors;
	AFollowingActor* Nearest = nullptr;

	TargetingCone->GetOverlappingActors(OverlappingFolloingActors, AFollowingActor::StaticClass());

	for (size_t i = 0; i < OverlappingFolloingActors.Num(); i++)
	{
		AFollowingActor* CurrentOverlappedFollower = Cast<AFollowingActor>(OverlappingFolloingActors[i]);
		const bool bIsValidTarget = CurrentOverlappedFollower && (!FirstFollowingActor || !FirstFollowingActor->ContainsFollowingActor(CurrentOverlappedFollower));
		if (bIsValidTarget)
		{
			AMellowsPawn * Mpawn = GetOwningPawn(CurrentOverlappedFollower);
			const bool bBallIsInvincible = Mpawn ? Mpawn->bIsInvincible : false;
			if (!bBallIsInvincible)
			{
				FHitResult HitResult;
				if (!GetWorld()->LineTraceSingleByChannel(HitResult, GetActorLocation(), CurrentOverlappedFollower->GetActorLocation(), ECollisionChannel::ECC_Visibility))
				{

					if (Nearest)
					{
						const float DetectedDistSq = FVector::DistSquared(CurrentOverlappedFollower->GetActorLocation(), GetActorLocation());
						const float NearestDistanceSq = FVector::DistSquared(Nearest->GetActorLocation(), GetActorLocation());
						if (DetectedDistSq < NearestDistanceSq)
						{
							Nearest = CurrentOverlappedFollower;
						}
					}
					else
					{
						Nearest = CurrentOverlappedFollower;
					}
				}
			}

		}
	}

	return Nearest;
}


void AMellowsPawn::UpdateClientProperties(float DeltaTime)
{
	if (GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		//Camera->SetFieldOfView(FMath::FInterpConstantTo(Camera->FieldOfView, FMath::Clamp(MovementComponent->ForwardSpeed * 0.036f, 80.f, 120.f), DeltaTime, 20.f));

		HoverParticles->SetWorldRotation(FTransform(MovementComponent->DefaultOrientation.ForwardVector, MovementComponent->DefaultOrientation.RightVector, MovementComponent->DefaultOrientation.UpVector, GetActorLocation()).GetRotation());

		// update speedlines
		float newWeight = FMath::Clamp((FMath::Abs(MovementComponent->ForwardSpeed) - SpeedLinesThreshold) / SpeedLinesThreshold, 0.0f, 1.0f);
		GlideParticlesRight->SetFloatParameter("speedAlpha", newWeight);
		GlideParticlesLeft->SetFloatParameter("speedAlpha", newWeight);
		if (SpeedLinesMaterial_Dyn) {
			SpeedLinesMaterial_Dyn->SetScalarParameterValue("Weight", newWeight);
		}
		if (CustomMaterial_Dyn)
		{
			CustomMaterial_Dyn->SetScalarParameterValue("PipeGlowStrength", (1.0f - (MovementComponent->Charge / MovementComponent->BaseMaxCharge)) * 10);
		}

		if (bIsTargeted && TargetedWarningMaterial_Dyn)
		{
			TargetedWarningMaterial_Dyn->SetScalarParameterValue("Visible", 1.f);
		}
		else
		{
			TargetedWarningMaterial_Dyn->SetScalarParameterValue("Visible", 0.f);
		}


		// gliding wind audio pitch update
		//float newPitchMultiplier = FMath::Clamp((FMath::Abs(MovementComponent->ForwardSpeed) - 2000.0f) / 2000.0f, 0.0f, 1.0f) * 3;
		//GlidingWindAudio->SetPitchMultiplier(newPitchMultiplier);

		//float newWindVolume = FMath::Clamp((FMath::Abs(MovementComponent->ForwardSpeed) - 1700.0f) / 1700.0f, 0.0f, 1.0f);
		//GlidingWindAudio->AdjustVolume(0.2f, newWindVolume);

		float LaserAlpha = GrappleOrbTarget ? 1.0f : 0.5f;
		if (OrbAboutToSteal)
		{
			LaserTargetPoint->SetWorldLocation(OrbAboutToSteal->GetActorLocation());
			LaserTargetPoint->SetVisibility(false);
		}
		else if (GrappleOrbTarget)
		{
			LaserTargetPoint->SetWorldLocation(GrappleOrbTarget->GetActorLocation());
			LaserTargetPoint->SetVisibility(false);
		}
		else if (!MovementComponent->bIsAttaching)
		{

			FHitResult laserHit;

			if (GetWorld()->LineTraceSingleByChannel(laserHit, GetActorLocation(), GrappleLocationTarget, ECC_Visibility, MovementComponent->Hover_TraceParams))
			{
				LaserTargetPoint->SetWorldLocation(FMath::VInterpTo(LaserTargetPoint->GetComponentLocation(), laserHit.ImpactPoint, GetWorld()->GetDeltaSeconds(), 20.f));
				LaserTargetPoint->SetWorldScale3D(FVector(FMath::Clamp(laserHit.Distance * 0.0001f * laserHit.Distance * 0.001f + 0.1f, 0.1f, 3.f)));
				LaserTargetPoint->SetVisibility(true);
				LaserAlpha = 1.f;
			}
			else
			{
				LaserTargetPoint->SetWorldLocation(FMath::VInterpTo(LaserTargetPoint->GetComponentLocation(), GrappleLocationTarget, GetWorld()->GetDeltaSeconds(), 20.f));
				LaserTargetPoint->SetVisibility(false);
			}
			GrappleLocationTarget = GetActorLocation() + PlaneMesh->GetForwardVector() * HookshotMaxRange;
		}
		else
		{
			LaserTargetPoint->SetWorldLocation(FMath::VInterpTo(LaserTargetPoint->GetComponentLocation(), AttachmentTargetTransform.GetLocation(), GetWorld()->GetDeltaSeconds(), 20.f));
			LaserTargetPoint->SetVisibility(false);
		}


		LaserParticles->SetBeamTargetPoint(0, LaserTargetPoint->GetComponentLocation(), 0);
		LaserParticles->SetBeamSourcePoint(0, GetActorLocation(), 0);
		LaserParticles->SetFloatParameter("Alpha", LaserAlpha);

		BoostParticles->SetVectorParameter("Size", FVector(MovementComponent->Charge));
		BoostParticles->SetFloatParameter("Strength", MovementComponent->Charge);

		// TODO: move somewhere more suitable
		if (MovementComponent->bIsGliding)
		{
			SpringArmLookBack->CameraRotationLagSpeed = GlideRotationLagSpeed;// 7.f;
			SpringArm->bInheritRoll = false;

			if (CurrentCameraTurnAngle > 0.1f || CurrentCameraTurnAngle < -0.1)
				Camera->RelativeRotation.Yaw = FMath::FInterpTo(Camera->RelativeRotation.Yaw, CurrentCameraTurnAngle, FApp::GetDeltaTime(), 0.75f);
			else
				Camera->RelativeRotation.Yaw = FMath::FInterpTo(Camera->RelativeRotation.Yaw, 0.0f, FApp::GetDeltaTime(), 1.75f);

		}
		else
		{
			SpringArmLookBack->CameraRotationLagSpeed = HoverRotationLagSpeed;// 9.f;// 7.f;
			SpringArm->bInheritRoll = true;
			Camera->RelativeRotation.Yaw = FMath::FInterpTo(Camera->RelativeRotation.Yaw, 0.0f, FApp::GetDeltaTime(), 1.5f);


		}


		// PITCHING
		if (FGenericPlatformMath::Abs(CameraPitchValue) > 0.1f)
		{
			float newPitch = FMath::Clamp(SpringArm->RelativeRotation.Pitch + (CameraPitchValue * 1.65f), -89.0f, 89.0f);
			SpringArm->RelativeRotation.Pitch = newPitch;
		}
		else
		{
			// TODO: CAMERA
			if (MovementComponent->bIsGliding)
			{
				if (MovementComponent->bIsCharging)
				{
					const FVector VerticalVelocity = PlaneMesh->GetPhysicsLinearVelocity() - FVector::VectorPlaneProject(PlaneMesh->GetPhysicsLinearVelocity(), GetActorTransform().GetRotation().GetUpVector());
					SpringArm->RelativeRotation.Pitch = FMath::FInterpConstantTo(SpringArm->RelativeRotation.Pitch, -25.f, DeltaTime, FMath::Clamp(VerticalVelocity.Size() * 0.017f, 0.f, 5.f));
					SpringArm->CameraRotationLagSpeed = GlideChargingRotationLagSpeed;// 6.f;
					//SpringArm->CameraRotationLagSpeed = FMath::FInterpTo(SpringArm->CameraRotationLagSpeed,6.f, DeltaTime, 10.f);// 2.f);
				}
				else {
					SpringArm->RelativeRotation.Pitch = FMath::FInterpTo(SpringArm->RelativeRotation.Pitch, 0.f, DeltaTime, 5.f);// 2.f);
					SpringArm->CameraRotationLagSpeed = FMath::FInterpTo(SpringArm->CameraRotationLagSpeed, GlideRotationLagSpeed, DeltaTime, 7.5f);// 2.f);
					//SpringArm->CameraRotationLagSpeed = GlideRotationLagSpeed;// 6.f;

				}
				SpringArm->SocketOffset.Z = FMath::FInterpTo(SpringArm->SocketOffset.Z, 300.f, DeltaTime, 2.f);
				//SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, 1500.0f, DeltaTime, 2.f); //1723.0f
				SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, 1350.0f, DeltaTime, 2.f); //1723.0f

				//Camera->RelativeRotation.Pitch = FMath::FInterpTo(Camera->RelativeRotation.Pitch, -10.0f, DeltaTime, 2.f);
				Camera->RelativeRotation.Pitch = FMath::FInterpTo(Camera->RelativeRotation.Pitch, -7.5f, DeltaTime, 2.f);

			}
			else
			{

				SpringArm->RelativeRotation.Pitch = FMath::FInterpTo(SpringArm->RelativeRotation.Pitch, 0.f, DeltaTime, 5.f);// 2.f);
				SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, 735.0f, DeltaTime, 2.f);
				SpringArm->SocketOffset.Z = FMath::FInterpTo(SpringArm->SocketOffset.Z, 230.f, DeltaTime, 2.f);
				Camera->RelativeRotation.Pitch = FMath::FInterpTo(Camera->RelativeRotation.Pitch, -3.5f, DeltaTime, 2.f);


			}
		}

		// YAWING/TURNING
		if (FGenericPlatformMath::Abs(CameraYawValue) > 0.1f && bCameraLookBack == false)
		{
			float newYaw = FMath::Clamp(SpringArm->RelativeRotation.Yaw + (CameraYawValue * 1.65f), -89.0f, 89.0f);
			SpringArm->RelativeRotation.Yaw = newYaw;
		}
		else
		{
			SpringArm->RelativeRotation.Yaw = FMath::FInterpTo(SpringArm->RelativeRotation.Yaw, 0.0f, FApp::GetDeltaTime(), 5.f);
		}

		Camera->SetActive(!bCameraLookBack);
		CameraLookBack->SetActive(bCameraLookBack);



		// Field of View based on Forward Speed
		Camera->SetFieldOfView(FMath::FInterpConstantTo(
			Camera->FieldOfView,
			MovementComponent->bIsBoosting ? FMath::Clamp(MovementComponent->ForwardSpeed * 0.036f, 80.f, 110.f) : 80.f,
			DeltaTime,
			20.f)
		);

		//Camera->SetFieldOfView(FMath::FInterpConstantTo(Camera->FieldOfView, FMath::Clamp(MovementComponent->ForwardSpeed * 0.036f, 80.f, 110.f), DeltaTime, 20.f));//120

		GrappleHook->SetVisibility(!(bHookEnabled && !bShootHook) || MovementComponent->bIsAttaching, true);

		// change rumble if charge is low
		const bool bChargeTooLow = ((MovementComponent->Charge / MovementComponent->BaseMaxCharge) < 0.9);
		if (!MovementComponent->bIsCharging && MovementComponent->bIsBoosting && bChargeTooLow && (Role == ROLE_AutonomousProxy))
		{
			float capacity = (MovementComponent->Charge / MovementComponent->BaseMaxCharge);

			if (!bSwitchedToOverheatWarningRumble)
			{
				bSwitchedToOverheatWarningRumble = true;
				BoostRumbleHandle = Cast<APlayerController>(GetController())->PlayDynamicForceFeedback
				(
					// exponential function 16^(-x - 0.1)
					pow(16.f, -1.0f * capacity - 0.1f),
					-1.0f,
					false,
					true,
					false,
					true,
					EDynamicForceFeedbackAction::Start,
					BoostRumbleHandle
				);
			}
			else
			{
				BoostRumbleHandle = Cast<APlayerController>(GetController())->PlayDynamicForceFeedback
				(
					pow(16.f, -1.0f * capacity - 0.1f),
					-1.0f,
					capacity < 0.3,
					true,
					capacity < 0.3,
					true,
					EDynamicForceFeedbackAction::Update,
					BoostRumbleHandle
				);

				//UE_LOG(LogTemp, Log, TEXT("Rumble Strength now exponential: %f"), pow(16.f, -1 * (MovementComponent->Charge / MovementComponent->BaseMaxCharge)));
			}
		}
	}
	if (Role == ROLE_AutonomousProxy)
	{
		//PlayTargetingSounds();
		if (bIsTargeted)
		{
			if (bPlayAlertSound)
			{
				if (!AlertSoundComp->IsValidLowLevel())
				{
					AlertSoundComp = UGameplayStatics::CreateSound2D(GetWorld(), AlertSound);
				}
				if (AlertSoundComp->IsValidLowLevel())
				{
					AlertSoundComp->Play();
				}
				bPlayAlertSound = false;
			}

		}
		else
		{
			if (AlertSoundComp->IsValidLowLevel())
			{
				AlertSoundComp->Stop();
			}
			bPlayAlertSound = true;
		}
	}
}

void AMellowsPawn::PlayTargetingSounds()
{
	if (TargetingOrbs >= 0.f)
	{
		if (TargetingCountdown > 0.f && !bAllOrbsTargeted)
		{
			if (bPlayTargetingSound)
			{
				if (!TargetingSoundComp->IsValidLowLevel())
				{
					TargetingSoundComp = UGameplayStatics::CreateSound2D(GetWorld(), TargetingSound);
				}
				if (TargetingSoundComp->IsValidLowLevel())
				{
					TargetingSoundComp->Play();
				}
				bPlayTargetingSound = false;
			}
			if (!bShootHook)
			{
				if (!GrapplePreLockOnOrb && LingeringOrbs >= 0)
				{
					TargetingSoundComp->SetPitchMultiplier(LingeringCountdown * 2 - 1);
				}
				else
				{
					TargetingSoundComp->SetPitchMultiplier(TargetingCountdown * 2 - 1);
				}
			}
			else
			{
				if (TargetingSoundComp->IsValidLowLevel())
				{
					TargetingSoundComp->Stop();
				}
				bPlayTargetingSound = true;
			}
		}
		else
		{
			if (TargetingSoundComp->IsValidLowLevel())
			{
				TargetingSoundComp->Stop();
			}
			bPlayTargetingSound = true;
		}
	}
	else
	{
		if (TargetingSoundComp->IsValidLowLevel())
		{
			TargetingSoundComp->Stop();
		}
		bPlayTargetingSound = true;
	}
}

void AMellowsPawn::RotatePawnToInput(float DeltaTime)
{
	if (!PlaneMesh->IsSimulatingPhysics())
	{
		return;
	}
	const FQuat DeltaYaw(MovementComponent->DefaultOrientation.UpVector, MovementComponent->CurrentYawSpeed * DeltaTime);
	const FQuat DeltaPitch(MovementComponent->DefaultOrientation.RightVector, (MovementComponent->CurrentPitchSpeed) * DeltaTime);
	const FQuat DeltaRoll(PlaneMesh->GetForwardVector(), (-MovementComponent->CurrentRollSpeed) * DeltaTime);
	AddActorWorldRotation(DeltaYaw, false, nullptr, ETeleportType::TeleportPhysics);
	AddActorWorldRotation(DeltaPitch, false, nullptr, ETeleportType::TeleportPhysics);
	AddActorWorldRotation(DeltaRoll, false, nullptr, ETeleportType::TeleportPhysics);
}

void AMellowsPawn::ExecuteAttachment(float DeltaTime)
{
	const bool bTooFarFromTarget = FVector::Distance(AttachmentTargetTransform.GetLocation(), GetActorLocation()) > AttachmentTransitionRange;
	if (bTooFarFromTarget)
	{
		MovementComponent->SetCurrentGroundCheckMultiplier(0.8f);
		FVector pullDirection = (AttachmentTargetTransform.GetLocation() - GetActorLocation()).GetSafeNormal();
		// Keep on path to the grapple hit location
		MovementComponent->AddGatheredForce(FVector::VectorPlaneProject(-PlaneMesh->GetPhysicsLinearVelocity() * 0.25f, pullDirection));
		// pull towards the grapple hit location
		MovementComponent->AddGatheredForce(pullDirection * PullForceMultiplier);
	}
	else
	{
		MovementComponent->SetCurrentGroundCheckMultiplier(1.7f);

		// end attachment
		MovementComponent->bIsAttaching = false;
		MovementComponent->DefaultOrientation.ForwardVector = AttachmentTargetTransform.GetRotation().GetForwardVector();
		MovementComponent->DefaultOrientation.UpVector = AttachmentTargetTransform.GetRotation().GetUpVector();
		MovementComponent->DefaultOrientation.RightVector = AttachmentTargetTransform.GetRotation().GetRightVector();
		MovementComponent->CheckHoverableGround();
		MovementComponent->UpdateDefaultOrientation();
		MovementComponent->SetGliding(MovementComponent->bIsGliding);
		PlaneMesh->SetWorldRotation(
			FTransform(
				MovementComponent->DefaultOrientation.ForwardVector,
				MovementComponent->DefaultOrientation.RightVector,
				MovementComponent->DefaultOrientation.UpVector,
				FVector()
			).GetRotation(),
			false,
			nullptr,
			ETeleportType::TeleportPhysics);

	}
}

void AMellowsPawn::ShootGrappleHook()
{
	if (bShootHook)
	{
		if (ShotGrappleOrbTarget)
		{
			GrappleLocationTarget = ShotGrappleOrbTarget->GetActorLocation();
		}



		// shoot grapple 
		GrappleHook->SetWorldLocation(FMath::VInterpConstantTo(
			GrappleHook->GetComponentLocation(),
			GrappleLocationTarget,
			GetWorld()->GetDeltaSeconds(),
			HookshotSpeed));
		// check for hit
		FHitResult AttachmentHit;
		FVector start = GetActorLocation();
		FVector end = GrappleHook->GetComponentLocation();
		GetWorld()->LineTraceSingleByChannel(AttachmentHit, start, end, ECollisionChannel::ECC_Visibility, MovementComponent->Hover_TraceParams);
		if (AttachmentHit.bBlockingHit)
		{
			UE_LOG(LogTemp, Log, TEXT("Hit"));


			// rumble
			APlayerController* PlayerController = Cast<APlayerController>(GetController());
			if (PlayerController && GrappleRumble)
			{
				FForceFeedbackParameters Params;
				Params.bLooping = false;
				Params.Tag = "GrappleRumble";
				PlayerController->ClientPlayForceFeedback(GrappleRumble, Params);
			}

			/// Create attachment target
			const FVector newUp(AttachmentHit.Normal);
			FVector newForward(FVector::VectorPlaneProject(PlaneMesh->GetForwardVector(), newUp));
			newForward /= newForward.Size();
			const FVector newRight(FVector::CrossProduct(newUp, newForward));
			const FVector targetLocation = AttachmentHit.Location + (AttachmentHit.Normal * MovementComponent->BaseHoverDistance);
			const FTransform TargetTransform(newForward, newRight, newUp, targetLocation);
			if (TargetTransform.IsValid())
			{
				MovementComponent->bIsAttaching = true;
				AttachmentTargetTransform = TargetTransform;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("AttachmentTargetTransform Not Valid!!!!"));
				AttachmentTargetTransform = GetActorTransform();
			}

			// End hook shooting
			bShootHook = false;
		}
		else // no hit
		{
			// continue until at max range
			bShootHook = !GrappleHook->GetComponentLocation().Equals(GrappleLocationTarget);
			if (!bShootHook)
			{
				bHookEnabled = false;
			}
		}

		GrappleLocalDelta = GetActorLocation() - GrappleHook->GetComponentLocation();
	}
	else
	{
		// set grapple target while not attaching
		ShotGrappleOrbTarget = GrappleOrbTarget;
		if (GrappleOrbTarget)
		{
			GrappleLocationTarget = GrappleOrbTarget->GetActorLocation();
		}
		else
		{
			GrappleLocationTarget = GetActorLocation() + PlaneMesh->GetForwardVector() * HookshotMaxRange;

		}
		if (bHookEnabled)
		{
			GrappleHook->SetWorldLocation(GetActorLocation());
		}
		else
		{
			GrappleLocalDelta = FMath::VInterpConstantTo(GrappleLocalDelta, FVector(0.f), GetWorld()->GetDeltaSeconds(), HookshotSpeed);
			if (GrappleLocalDelta.SizeSquared() > 1.f)
			{
				GrappleHook->SetWorldLocation(GetActorLocation() - GrappleLocalDelta);
			}
			else
			{
				bHookEnabled = true;
			}
		}

	}
}

void AMellowsPawn::UpdateServerStateVariables()
{
	ServerLocationTarget = GetActorLocation();
	ServerRotationTarget = GetActorRotation();
	ServerVelocityTarget = PlaneMesh->GetPhysicsLinearVelocity();
}

void AMellowsPawn::OnDamageSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (MovementComponent->IsValidLowLevel())
	{
		AMellowsPawn* rammedActor = Cast<AMellowsPawn>(OtherActor);
		if (rammedActor)
		{
			if (OtherComp->GetName().Equals(DamageSphere->GetName()) && MovementComponent->bIsBoosting)
			{
				if (!rammedActor->MovementComponent->bIsBoosting)
				{
					/// Steal charge
					const float stolenCharge = rammedActor->MovementComponent->Charge * 0.5f;
					MovementComponent->AddCharge(stolenCharge);
					rammedActor->MovementComponent->AddCharge(-stolenCharge);

					/// Drop other players followers
					//if (rammedActor->FirstFollowingActor)
					//{
					//	const uint32 otherFollowerCount = rammedActor->FirstFollowingActor->FollowerCount();
					//	if (otherFollowerCount > 0)
					//	{
					//		AFollowingActor* firstDroppedFollower = rammedActor->FirstFollowingActor->GetAt((uint32)(otherFollowerCount * 0.5));
					//		if (firstDroppedFollower)
					//		{
					//			firstDroppedFollower->DropFromHere();
					//		}
					//	}
					//}
					// todo finish impulse on ramming
					//rammedActor->PlaneMesh->AddImpulse(60000.f * (GetActorLocation() - OtherActor->GetActorLocation()));
				}
			}
		}
	}
}

float AMellowsPawn::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Todo implement take damage
	UE_LOG(LogTemp, Warning, TEXT("TakeDamage not implemented!"));
	return 0.0f;
}



void AMellowsPawn::OnPickupSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		AFollowingActor* OverlappedFollowingActor = Cast<AFollowingActor>(OtherActor);
		if (OverlappedFollowingActor != nullptr)
		{
			if (FirstFollowingActor == nullptr) // No spheres are following 
			{
				AMellowsPawn* OtherPlayerPawn = GetOwningPawn(OverlappedFollowingActor);
				if (OtherPlayerPawn) // steal from other player 
				{
					if (!OtherPlayerPawn->bIsInvincible)
					{
						// calculate the orbs that should be stolen 

						float* TargetStartTime = OtherPlayerPawn->TargetingPawnsMap.Find(this);
						const float TimeDifference = TargetStartTime ? (GetWorld()->TimeSeconds - *TargetStartTime) : 0;
						//UE_LOG(LogTemp, Warning, TEXT("TimeDifference %f"), TimeDifference);
						const float FollowerCount = OtherPlayerPawn->FirstFollowingActor->FollowerCount();
						const uint32 OrbIndexToSteal = static_cast<uint32>(FMath::Clamp(FollowerCount - TimeDifference * OrbsPerSeconds, 0.f, FollowerCount));

						// Steal the orbs
						AFollowingActor* StolenOrb = OtherPlayerPawn->FirstFollowingActor->GetAt(OrbIndexToSteal);
						if (StolenOrb)
						{
							uint32 orbsStolen = StolenOrb->FollowerCount();
							StolenOrb->FollowTarget(this);
							FirstFollowingActor = StolenOrb;
							FirstFollowingActor->UpdateTarget(GetActorLocation());
							UpdateFollowingActorsCount();
							OtherPlayerPawn->UpdateFollowingActorsCount();
							GrappleOrbTarget = nullptr;
							OtherPlayerPawn->StartInvincibility(InvincibilityTimeStolen);
							StartInvincibility(InvincibilityTimeSteal);
							OtherPlayerPawn->bIsTargeted = false;
							bIsTargeted = false;
							OtherPlayerPawn->TargetingPawnsMap.Remove(this);

							Client_PlaySound(StealSound);
							OtherPlayerPawn->Client_PlaySound(StolenSound);

							AMellowsPlayerState* ownMellowsPlayerState = Cast<AMellowsPlayerState>(this->GetPlayerState());
							AMellowsPlayerState* otherMellowsPlayerState = Cast<AMellowsPlayerState>(OtherPlayerPawn->GetPlayerState());

							if (ownMellowsPlayerState && otherMellowsPlayerState)
							{
								// RPC to other clients pawn that he has been robbed
								OtherPlayerPawn->Client_LostOrbsMessage(ownMellowsPlayerState->GetPlayerName(), orbsStolen);

								// Inform own player client that he/she stole orbs
								this->Client_StoleOrbsMessage(otherMellowsPlayerState->GetPlayerName(), orbsStolen);
							}

							PlaySphereHitRumble();
						}
					}
				}
				else // collect lonely sphere 
				{
					OverlappedFollowingActor->FollowTarget(this);
					FirstFollowingActor = OverlappedFollowingActor;
					FirstFollowingActor->UpdateTarget(GetActorLocation());
					GrappleOrbTarget = nullptr;
					bIsTargeted = false;
					UpdateFollowingActorsCount();
					Client_SetChainVisibility(false);

					//this->Client_StoleOrbsMessage("", 1);
					this->Client_CollectedFreeOrbMessage();



					CallPlaySound(PickupSound);
					PlaySphereHitRumble();
				}
			}
			else if (!FirstFollowingActor->ContainsFollowingActor(OverlappedFollowingActor)) // Append to existing spheres
			{
				AMellowsPawn* OtherPlayerPawn = GetOwningPawn(OverlappedFollowingActor);
				if (OtherPlayerPawn) // steal from other player
				{
					if (!OtherPlayerPawn->bIsInvincible)
					{
						// calculate the orbs that should be stolen 
						float* TargetStartTime = OtherPlayerPawn->TargetingPawnsMap.Find(this);
						const float TimeDifference = TargetStartTime ? (GetWorld()->TimeSeconds - *TargetStartTime) : 0;
						//UE_LOG(LogTemp, Warning, TEXT("TimeDifference %f"), TimeDifference);
						const float FollowerCount = OtherPlayerPawn->FirstFollowingActor->FollowerCount();
						const uint32 OrbIndexToSteal = static_cast<uint32>(FMath::Clamp(FollowerCount - TimeDifference * OrbsPerSeconds, 0.f, FollowerCount));

						// Steal the orbs
						AFollowingActor* StolenOrb = OtherPlayerPawn->FirstFollowingActor->GetAt(OrbIndexToSteal);
						if (StolenOrb)
						{
							StolenOrb->FollowTarget(FirstFollowingActor->GetLast(2000));
							StolenOrb->UpdateTarget(GetActorLocation());
							GrappleOrbTarget = nullptr;
							//uint32 orbsStolen = FirstFollowingActor->FollowerCount();

							UpdateFollowingActorsCount();
							OtherPlayerPawn->UpdateFollowingActorsCount();
							OtherPlayerPawn->StartInvincibility(InvincibilityTimeStolen);
							StartInvincibility(InvincibilityTimeSteal);
							OtherPlayerPawn->bIsTargeted = false;
							bIsTargeted = false;
							OtherPlayerPawn->TargetingPawnsMap.Remove(this);

							Client_PlaySound(StealSound);
							OtherPlayerPawn->Client_PlaySound(StolenSound);

							AMellowsPlayerState* MellowsPlayerState = Cast<AMellowsPlayerState>(this->GetPlayerState());
							AMellowsPlayerState* otherMellowsPlayerState = Cast<AMellowsPlayerState>(OtherPlayerPawn->GetPlayerState());

							if (MellowsPlayerState && otherMellowsPlayerState)
							{
								// RPC to other clients pawn that he has been robbed
								OtherPlayerPawn->Client_LostOrbsMessage(MellowsPlayerState->GetPlayerName(), StolenOrb->FollowerCount());

								// Inform own player client that he/she stole orbs
								this->Client_StoleOrbsMessage(otherMellowsPlayerState->GetPlayerName(), StolenOrb->FollowerCount());
							}

							PlaySphereHitRumble();
						}
					}


				}
				else // collect lonely sphere
				{
					OverlappedFollowingActor->FollowTarget(FirstFollowingActor->GetLast(2000.f));
					OverlappedFollowingActor->UpdateTarget(GetActorLocation());
					GrappleOrbTarget = nullptr;
					bIsTargeted = false;
					UpdateFollowingActorsCount();
					Client_SetChainVisibility(false);

					//this->Client_StoleOrbsMessage("", 1);
					this->Client_CollectedFreeOrbMessage();


					CallPlaySound(PickupSound);
					PlaySphereHitRumble();
				}
			}
			if (bIsInvincible)
			{
				Multi_SetChainVisibility(false);
			}
		}

	}
}

AMellowsPawn* AMellowsPawn::GetOwningPawn(AFollowingActor* overlappedFollowingActor)
{
	if (overlappedFollowingActor && overlappedFollowingActor->previous)
	{
		AActor* otherPlayersFollowingActor = overlappedFollowingActor->previous;
		while (!Cast<AMellowsPawn>(otherPlayersFollowingActor))
		{
			AFollowingActor* OtherFollower = Cast<AFollowingActor>(otherPlayersFollowingActor);
			if (OtherFollower)
			{
				otherPlayersFollowingActor = OtherFollower->previous;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Previous null, Player probably left"));
				break;
			}
		}
		return Cast<AMellowsPawn>(otherPlayersFollowingActor);
	}
	return nullptr;

}

void AMellowsPawn::UpdateFollowingActorsCount()
{
	// get player state
	AMellowsPlayerState* MellowState = Cast<AMellowsPlayerState>(GetPlayerState());
	if (MellowState)
	{
		// update the PlayerState FollowingActorCount
		MellowState->SetFollowingActorsCount(FirstFollowingActor ? FirstFollowingActor->FollowerCount() : 0);
	}
}


void AMellowsPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMellowsPawn, MovementComponent);
	DOREPLIFETIME(AMellowsPawn, ServerLocationTarget);
	DOREPLIFETIME(AMellowsPawn, ServerVelocityTarget);
	DOREPLIFETIME(AMellowsPawn, ServerRotationTarget);
	DOREPLIFETIME(AMellowsPawn, FirstFollowingActor);
	DOREPLIFETIME(AMellowsPawn, bShootHook);
	DOREPLIFETIME(AMellowsPawn, GrappleLocationTarget);
	DOREPLIFETIME(AMellowsPawn, AttachmentTargetTransform);

	DOREPLIFETIME(AMellowsPawn, bIsStopped);
	DOREPLIFETIME(AMellowsPawn, bHookEnabled);
	DOREPLIFETIME(AMellowsPawn, bIsTargeted);
	DOREPLIFETIME(AMellowsPawn, ColorGliderParts);
	DOREPLIFETIME(AMellowsPawn, TargetingOrbs);
	DOREPLIFETIME(AMellowsPawn, TargetingCountdown);
	DOREPLIFETIME(AMellowsPawn, LingeringCountdown);
	DOREPLIFETIME(AMellowsPawn, bAllOrbsTargeted);
	DOREPLIFETIME(AMellowsPawn, LingeringOrbs);
	DOREPLIFETIME(AMellowsPawn, GrappleOrbTarget);
	DOREPLIFETIME(AMellowsPawn, GrapplePreLockOnOrb);
	DOREPLIFETIME(AMellowsPawn, GrappleOwnedOrbTarget);
	DOREPLIFETIME(AMellowsPawn, OrbAboutToSteal);
}

void AMellowsPawn::Multi_SetSimulatePhysics_Implementation(bool Val)
{
	PlaneMesh->SetSimulatePhysics(Val);
}


void AMellowsPawn::Multi_UpdateFollowerReference_Implementation(AFollowingActor* firstFollower)
{
	FirstFollowingActor = firstFollower;
}

void AMellowsPawn::Multi_StartImpulseParticles_Implementation()
{
	ImpulseParticles->Activate();

	if (ImpulseRumble && Role == ROLE_AutonomousProxy)
	{
		FForceFeedbackParameters Params;
		Params.bLooping = false;
		Params.Tag = "ImpulseRumble";
		Cast<APlayerController>(GetController())->ClientPlayForceFeedback(ImpulseRumble, Params);

	}
}

void AMellowsPawn::OnRep_VelocityChanged()
{
	const bool bShouldInterpolate = FVector::Distance(PlaneMesh->GetPhysicsLinearVelocity(), ServerVelocityTarget) < VelocitySnapDistance;
	if (bShouldInterpolate)
	{
		PlaneMesh->SetPhysicsLinearVelocity(FMath::VInterpTo(PlaneMesh->GetPhysicsLinearVelocity(), ServerVelocityTarget, GetWorld()->GetDeltaSeconds(), NetworkVelocityInterpSpeed));
	}
	else // Hard Snap Velocity
	{
		PlaneMesh->SetPhysicsLinearVelocity(ServerVelocityTarget);
	}
}

void AMellowsPawn::OnRep_LocationChanged()
{
	const bool bShouldInterpolate = FVector::Distance(GetActorLocation(), ServerLocationTarget) < LocationSnapDistance;
	if (bShouldInterpolate)
	{
		SetActorLocation(FMath::VInterpTo(GetActorLocation(), ServerLocationTarget, GetWorld()->GetDeltaSeconds(), NetworkLocationInterpSpeed), false, nullptr, ETeleportType::TeleportPhysics);
	}
	else // Hard Snap Location
	{
		SetActorLocation(ServerLocationTarget, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AMellowsPawn::OnRep_RotationChanged()
{
	const FRotator UpdatedRot = FMath::RInterpTo(GetActorRotation(), ServerRotationTarget, GetWorld()->GetDeltaSeconds(), NetworkRotationInterpSpeed);
	SetActorRotation(UpdatedRot, ETeleportType::TeleportPhysics);
}

// Called to bind functionality to input
void AMellowsPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// -1 is for weirdos 1 is for cool guys
	UMellowsGameInstance* GameInstance = Cast<UMellowsGameInstance>(GetGameInstance());
	GameInstance->bPitchInverted ? InvertedPitchMultiplier = -1 : InvertedPitchMultiplier = 1;
	GameInstance->bCameraInverted ? InvertedCameraPitchMultiplier = -1 : InvertedCameraPitchMultiplier = 1;

	Super::SetupPlayerInputComponent(PlayerInputComponent);
	InputComponent->BindAxis("Pitch", this, &AMellowsPawn::Pitch_Input);
	InputComponent->BindAxis("Turn", this, &AMellowsPawn::Turn_Input);
	InputComponent->BindAxis("ChargeAxis", this, &AMellowsPawn::Charge_Input);
	InputComponent->BindAxis("BoostAxis", this, &AMellowsPawn::Boost_Input);

	InputComponent->BindAxis("CamRight", this, &AMellowsPawn::Camera_Turn_Input);
	InputComponent->BindAxis("CamUp", this, &AMellowsPawn::Camera_Pitch_Input);

	InputComponent->BindAction("Attach", IE_Pressed, this, &AMellowsPawn::AttachPressed_Input);
	InputComponent->BindAction("Attach", IE_Released, this, &AMellowsPawn::AttachReleased_Input);

	InputComponent->BindAction("Impulse", IE_Pressed, this, &AMellowsPawn::ImpulsePressed_Input);
	InputComponent->BindAction("Impulse", IE_Released, this, &AMellowsPawn::ImpulseReleased_Input);

	InputComponent->BindAction("LookBack", IE_Pressed, this, &AMellowsPawn::LookBackPressed);
	InputComponent->BindAction("LookBack", IE_Released, this, &AMellowsPawn::LookBackReleased);


}

void AMellowsPawn::Client_SwapTargetForParent_Implementation(AFollowingActor* OldActorTarget, AFollowingActor* NewActorTarget)
{

	if (OldActorTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("OldActorTarget->EndTargetForParent();"));
		OldActorTarget->EndTargetForParent();
	}
	if (NewActorTarget)
	{
		NewActorTarget->StartTargetForParent();
		LaserTargetPoint->SetWorldLocation(NewActorTarget->GetActorLocation());
		LaserTargetPoint->SetVisibility(false);
		// todo update laser pointer end position
	}
}

void AMellowsPawn::Client_UpdateTargetForPawn_Implementation(AMellowsPawn* ParentPawn, int index)
{
	if (ParentPawn)
	{
		AFollowingActor* TargetedPawn = ParentPawn->FirstFollowingActor->GetAt(index);
		if (TargetedPawn)
		{
			TargetedPawn->StartTargetForFollowing();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Wrong Index on updating target"));

		}
	}
}

void AMellowsPawn::Client_StopTargetingPawn_Implementation(AMellowsPawn* ParentPawn)
{
	if (ParentPawn && ParentPawn->FirstFollowingActor)
	{
		ParentPawn->FirstFollowingActor->EndTargetForFollowing();
	}
}

void AMellowsPawn::Client_PlaySound_Implementation(USoundBase* GrappleSound)
{
	UGameplayStatics::PlaySound2D(GetWorld(), GrappleSound);
}

void AMellowsPawn::Client_LostOrbsMessage_Implementation(const FString& thiefName, const uint32 stolenOrbsAmount)
{
	// NOTIFY CLIENT THAT HIS BALLS HAVE BEEN STOLEN
	APlayerController* Player = GetWorld()->GetFirstPlayerController();

	if (Player)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("HE STOLE YOUR BALLS")));
		AMelllowsGameHUD* PlayerHud = Cast<AMelllowsGameHUD>(Player->GetHUD());
		//if (stolenOrbsAmount > 1)
		if (PlayerHud)
		{
			PlayerHud->MellowsUIWidget->MessageText->SetText(FText::FromString(FString("<LostOrbs>-") + FString::FromInt(stolenOrbsAmount) + FString("</> to <PlayerName>") + thiefName + FString("</>")));
			//else
			//	PlayerHud->MellowsUIWidget->MessageText->SetText(FText::FromString(FString("<LostOrbs>-") + FString::FromInt(stolenOrbsAmount) + FString("</> to <PlayerName>") + thiefName + FString("</>")));

			PlayerHud->MellowsUIWidget->showBeeingRobbedAnimation();

			// PLAY COUNTER ANIMATION
			PlayerHud->MellowsUIWidget->GliderInfoWidget->showLoseOrbsAnimation();
			// SET TIMER
			GetWorld()->GetTimerManager().SetTimer(MessageTimerHandle, this, &AMellowsPawn::hideHUDMessage, 5, false);
		}
	}
}

void AMellowsPawn::Client_StoleOrbsMessage_Implementation(const FString& bestolenName, const uint32 stolenOrbsAmount)
{
	// NOTIFY CLIENT THAT HE HAS STOLEN BALLS
	APlayerController* Player = GetWorld()->GetFirstPlayerController();

	AMelllowsGameHUD* PlayerHud = Cast<AMelllowsGameHUD>(Player->GetHUD());
	if (stolenOrbsAmount > 1)
		PlayerHud->MellowsUIWidget->MessageText->SetText(FText::FromString(FString("<GotOrbs>+") + FString::FromInt(stolenOrbsAmount) + FString("</> from  <PlayerName>") + bestolenName + FString("</>") ));
	else
		PlayerHud->MellowsUIWidget->MessageText->SetText(FText::FromString(FString("<GotOrbs>+") + FString::FromInt(stolenOrbsAmount) + FString("</> from  <PlayerName>") + bestolenName + FString("</>")));

	// SET TIMER
	GetWorld()->GetTimerManager().SetTimer(MessageTimerHandle, this, &AMellowsPawn::hideHUDMessage, 5, false);

	// FLASH PICKUP SCREEN
	PlayerHud->MellowsUIWidget->showPickupScreenOverlay();
	// PLAY COUNTER ANIMATION
	PlayerHud->MellowsUIWidget->GliderInfoWidget->showAddOrbsAnimation();
}

void AMellowsPawn::Client_DeliveredOrbsMessage_Implementation(int deliveredPoints)
{
	// NOTIFY CLIENT HOW MANY POINTS HE RECEIVED
	APlayerController* Player = GetWorld()->GetFirstPlayerController();

	if (Player)
	{

		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("HE STOLE YOUR BALLS")));
		AMelllowsGameHUD* PlayerHud = Cast<AMelllowsGameHUD>(Player->GetHUD());

		if (PlayerHud)
		{
			PlayerHud->MellowsUIWidget->MessageText->SetText(FText::FromString("<GotPoints>+" + FString::FromInt(deliveredPoints) + "</> POINTS"));

			// SET TIMER
			GetWorld()->GetTimerManager().SetTimer(MessageTimerHandle, this, &AMellowsPawn::hideHUDMessage, 5, false);
		}
	}
}

void AMellowsPawn::Client_PickedUpImpulseMessage_Implementation()
{
	APlayerController* Player = GetWorld()->GetFirstPlayerController();


	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("HE STOLE YOUR BALLS")));
	AMelllowsGameHUD* PlayerHud = Cast<AMelllowsGameHUD>(Player->GetHUD());
	PlayerHud->MellowsUIWidget->showImpulseReadyAnimation();
}

void AMellowsPawn::Client_CollectedFreeOrbMessage_Implementation()
{
	APlayerController* Player = GetWorld()->GetFirstPlayerController();

	AMelllowsGameHUD* PlayerHud = Cast<AMelllowsGameHUD>(Player->GetHUD());
	PlayerHud->MellowsUIWidget->showPickupScreenOverlay();
	// PLAY COUNTER ANIMATION
	PlayerHud->MellowsUIWidget->GliderInfoWidget->showAddOrbsAnimation();
}



void AMellowsPawn::hideHUDMessage()
{

	// TODO:: PASS THAT FROM THE PARAMETERS 

	// NOTIFY CLIENT THAT HIS BALLS HAVE BEEN STOLEN
	APlayerController* Player = GetWorld()->GetFirstPlayerController();

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("HE STOLE YOUR BALLS")));
	AMelllowsGameHUD* PlayerHud = Cast<AMelllowsGameHUD>(Player->GetHUD());
	PlayerHud->MellowsUIWidget->MessageText->SetText(FText::FromString(FString("")));


}

void AMellowsPawn::Charge_Input(float Val)
{
	Server_Charge(Val);
	ChargeUp(Val);
}

void AMellowsPawn::Server_Charge_Implementation(float Val) { ChargeUp(Val); }
bool AMellowsPawn::Server_Charge_Validate(float Val) { return true; }
void AMellowsPawn::ChargeUp(float Val) { MovementComponent->HandleCharge(Val); }


void AMellowsPawn::Boost_Input(float Val) {
	Server_Boost(Val);
	Boost(Val);
}
void AMellowsPawn::Server_Boost_Implementation(float Val) { Boost(Val); }
bool AMellowsPawn::Server_Boost_Validate(float Val) { return true; }
void AMellowsPawn::Boost(float Val) { MovementComponent->HandleBoost(Val); }


void AMellowsPawn::AttachPressed_Input() { Server_AttachPressed(); }
void AMellowsPawn::AttachReleased_Input() { Server_AttachReleased(); }
void AMellowsPawn::Server_AttachPressed_Implementation() { AttachPressed(); }
bool AMellowsPawn::Server_AttachPressed_Validate() { return true; }
void AMellowsPawn::Server_AttachReleased_Implementation() { AttachReleased(); }
bool AMellowsPawn::Server_AttachReleased_Validate() { return true; }


void AMellowsPawn::ImpulsePressed_Input() { ImpulsePressed(); Server_ImpulsePressed(); }
void AMellowsPawn::ImpulseReleased_Input() { ImpulseReleased(); Server_ImpulseReleased(); }
void AMellowsPawn::Server_ImpulsePressed_Implementation() { ImpulsePressed(); }
bool AMellowsPawn::Server_ImpulsePressed_Validate() { return true; }
void AMellowsPawn::Server_ImpulseReleased_Implementation() { ImpulseReleased(); }
bool AMellowsPawn::Server_ImpulseReleased_Validate() { return true; }


void AMellowsPawn::ImpulsePressed()
{
	MovementComponent->StartImpulse();
}
void AMellowsPawn::ImpulseReleased()
{
	MovementComponent->StopImpulse();
}


void AMellowsPawn::AttachPressed()
{
	const bool bShootHookPrevious = bShootHook;
	if (bHookEnabled)
	{
		if (MovementComponent->bIsAttaching)
		{

			if (GrappleOrbTarget)
			{
				bShootHook = true;
			}
			else
			{
				bShootHook = false;
			}
			MovementComponent->bIsAttaching = false;
		}
		else
		{
			if (!ShotGrappleOrbTarget && (GrappleOrbTarget != ShotGrappleOrbTarget))
			{
				ShotGrappleOrbTarget = GrappleOrbTarget;
				bShootHook = true;
			}
			else if (GrappleOrbTarget)
			{
				bShootHook = true;
			}
			else
			{
				bShootHook = !bShootHook;
			}
		}
		if (!bShootHook)
		{
			bHookEnabled = false;
		}
	}
	else
	{
		bShootHook = false;
	}

	if ((bShootHook == true) && (bShootHookPrevious != bShootHook))
	{
		Client_PlaySound(GrappleSound);
	}
}

void AMellowsPawn::AttachReleased()
{
	//bShootHook = false;
	//MovementComponent->bIsAttaching = false;
}


void AMellowsPawn::LookBackPressed()
{
	bCameraLookBack = true;
}
void AMellowsPawn::LookBackReleased()
{
	bCameraLookBack = false;
}



void AMellowsPawn::Pitch_Input(float Val)
{
	Val *= InvertedPitchMultiplier;
	Server_Pitch(Val);
	Pitch(Val);
}
void AMellowsPawn::Turn_Input(float Val)
{
	Server_Turn(Val);
	Turn(Val);
}
void AMellowsPawn::Server_Pitch_Implementation(float Val) { Pitch(Val); }
bool AMellowsPawn::Server_Pitch_Validate(float Val) { return true; }
void AMellowsPawn::Server_Turn_Implementation(float Val) { Turn(Val); }
bool AMellowsPawn::Server_Turn_Validate(float Val) { return true; }
void AMellowsPawn::Pitch(float Val) { MovementComponent->HandlePitch(Val); }
void AMellowsPawn::Turn(float Val) {
	MovementComponent->HandleTurn(Val);
	CurrentCameraTurnAngle = MaxCameraTurnAngle * Val;// FMath::FInterpTo(CurrentCameraTurnAngle, MaxCameraTurnAngle * Val, GetWorld()->GetDeltaSeconds(), 10.f);
}



void AMellowsPawn::Camera_Pitch_Input(float Val) {
	Val *= InvertedCameraPitchMultiplier;

	Server_Camera_Pitch(Val);
	Camera_Pitch(Val);
}
void AMellowsPawn::Camera_Turn_Input(float Val)
{
	Server_Camera_Turn(Val);
	Camera_Turn(Val);
}
void AMellowsPawn::Server_Camera_Pitch_Implementation(float Val) { Camera_Pitch(Val); }
bool AMellowsPawn::Server_Camera_Pitch_Validate(float Val) { return true; }
void AMellowsPawn::Server_Camera_Turn_Implementation(float Val) { Camera_Turn(Val); }
bool AMellowsPawn::Server_Camera_Turn_Validate(float Val) { return true; }

void AMellowsPawn::Camera_Pitch(float Val)
{
	CameraPitchValue = Val;
}
void AMellowsPawn::Camera_Turn(float Val)
{
	CameraYawValue = Val;
}

void AMellowsPawn::OnRep_ColorSchemeChange()
{
	//PlaneMesh->SetMaterial(0, ActiveGliderMaterial);
	if (Role != ROLE_AutonomousProxy)
	{
		PlaneMesh->SetRenderCustomDepth(true);
		PlaneMesh->SetCustomDepthStencilValue(254);
	}
}



void AMellowsPawn::OnRep_ColorChange()
{
	ChangeGliderPartsColor();
}

void AMellowsPawn::ChangeGliderPartsColor()
{
	if (CustomMaterial_Dyn)
	{
		CustomMaterial_Dyn->SetVectorParameterValue("Body", GliderPartColors[ColorGliderParts.BodyColor]);
		CustomMaterial_Dyn->SetVectorParameterValue("RightWing", GliderPartColors[ColorGliderParts.WingColor]);
		CustomMaterial_Dyn->SetVectorParameterValue("LeftWing", GliderPartColors[ColorGliderParts.WingColor]);
		CustomMaterial_Dyn->SetVectorParameterValue("Jets", GliderPartColors[ColorGliderParts.JetColor]);
		CustomMaterial_Dyn->SetVectorParameterValue("Misc01", GliderPartColors[ColorGliderParts.InnerPartColor]);
		CustomMaterial_Dyn->SetVectorParameterValue("Misc02", GliderPartColors[ColorGliderParts.InteriorColor]);
		CustomMaterial_Dyn->SetVectorParameterValue("Misc03", GliderPartColors[ColorGliderParts.HingeColor]);
		CustomMaterial_Dyn->SetVectorParameterValue("Misc04", GliderPartColors[ColorGliderParts.PlateColor]);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Dynamic material does not exist yet."));
	}
}

void AMellowsPawn::PlayOverheatForceFeedback()
{
	if (Role == ROLE_AutonomousProxy)
	{
		FForceFeedbackParameters Params;
		Params.bLooping = false;
		Params.Tag = "OverheatRumble";
		Cast<AMellowsPlayerController>(GetController())->ClientPlayForceFeedback(OverheatRumble, Params);
	}
}

void AMellowsPawn::StopBoostForceFeedback()
{
	if (Role == ROLE_AutonomousProxy)
	{
		BoostRumbleHandle = Cast<APlayerController>(GetController())->PlayDynamicForceFeedback
		(
			0.f,
			0.01f,
			true,
			true,
			true,
			true,
			EDynamicForceFeedbackAction::Stop,
			BoostRumbleHandle
		);

		bSwitchedToOverheatWarningRumble = false;
	}
}

void AMellowsPawn::PlaySphereHitRumble()
{
	if (HitRumble)
	{
		FForceFeedbackParameters Params;
		Params.bLooping = false;
		Params.Tag = "SphereHitRumble";
		Cast<APlayerController>(GetController())->ClientPlayForceFeedback(HitRumble, Params);
	}
}

void AMellowsPawn::StartInvincibility(float invincibilityTime)
{
	bIsInvincible = true;
	GetWorld()->GetTimerManager().SetTimer(InvinibilityTimerHandle, this, &AMellowsPawn::InvincibilityTimeout, invincibilityTime, false);
	Multi_SetChainVisibility(false);
	// todo timer
}

void AMellowsPawn::InvincibilityTimeout()
{
	bIsInvincible = false;
	Multi_SetChainVisibility(true);
}

void AMellowsPawn::Multi_SetChainVisibility_Implementation(bool bIsVisible)
{
	if (FirstFollowingActor) {
		{
			if (Role == ROLE_AutonomousProxy)
			{
				FirstFollowingActor->SetVisibilityLocal(false);
			}
			else
			{
				FirstFollowingActor->SetVisibilityLocal(bIsVisible);
			}

		}
	}
}

void AMellowsPawn::Client_SetChainVisibility_Implementation(bool bIsVisible)
{
	if (FirstFollowingActor) {
		FirstFollowingActor->SetVisibilityLocal(bIsVisible);
	}
}

void AMellowsPawn::CallPlaySound(USoundBase* Sound)
{
	Client_PlaySound(Sound);
}

UCameraComponent* AMellowsPawn::GetActiveCamera()
{
	return bCameraLookBack ? CameraLookBack : Camera;
}

void AMellowsPawn::SetStopped(bool newStopped)
{
	PlaneMesh->SetEnableGravity(!newStopped);
	bIsStopped = newStopped;
}